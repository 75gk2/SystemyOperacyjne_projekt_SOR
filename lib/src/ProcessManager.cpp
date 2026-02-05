#include "ProcessManager.hpp"
#include "Process.hpp"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <sys/wait.h>
#include <thread>
#include <vector>
#include <unistd.h>

#include <spdlog/spdlog.h>

std::vector<pid_t> ProcessManager::getPidsOfProcesses() const {
    std::vector<pid_t> keys;
    {
        std::lock_guard lock(processMutex);
        keys.reserve(processList.size());

        for (const auto &kv: processList) {
            keys.push_back(kv.first);
        }
    }
    return keys;
}


ProcessManager::ProcessManager() : semaphores(true),
                                   memory(true),
                                   doctorIn('D', true),
                                   doctorsRoom('E', true) {
    installSigintHandlerGlobally();
    reaperThread = std::thread(&ProcessManager::reaperLoop, this);
    spdlog::debug("ProcessManager: Reaper thread started");
}

ProcessManager::~ProcessManager() {
    requestShutdown();
    stopReaper.store(true);
    if (reaperThread.joinable()) {
        reaperThread.join();
    }

    {
        std::lock_guard lock(processMutex);
        spdlog::debug("ProcessManager: Terminating processes number={}", processList.size());

        // Clear the process list, what calls Process' destructor, → each Process destructor will kill and wait self process
        processList.clear();
    }
    spdlog::debug("ProcessManager: Destroyed");
}

bool ProcessManager::assignProcess(std::unique_ptr<Process> process) {
    if (isShutdownRequested()) {
        spdlog::warn("ProcessManager: shutdown requested, not spawning new process path={}", process->path);
        return false;
    }
    if (!process->assignToManager()) return false;
    switch (process->pid = fork()) {
        case -1:
            spdlog::error("Process: Fork failed for process path={}", process->path);
            perror("Process: fork failed");
            throw std::runtime_error("Process: Fork failed - can't create Process object");

        case 0: {
            std::vector<char *> argv;
            argv.reserve(process->extraArgs.size() + 2);
            argv.push_back(const_cast<char *>(process->path));
            for (auto &arg: process->extraArgs) {
                argv.push_back(const_cast<char *>(arg.c_str()));
            }
            argv.push_back(nullptr);

            execv(process->path, argv.data());
            printThreadSafeLog("Forked process: execv failed for process", true, process->path);
            perror("Process: execv failed");
            //return without calling any copied destructors
            _exit(EXIT_FAILURE);
        }
        default:
            spdlog::debug("Process: forked successfully for Process with pid={}", process->pid);
            process->status = RUNNING;
    }
    pid_t pid = process->getPid();
    {
        std::lock_guard lock(processMutex);
        processList[pid] = std::move(process);
        updateSharedMemoryLocked();
    }
    spdlog::debug("ProcessManager: Added process with pid={}", pid);
    return true;
}


void ProcessManager::removeProcess(pid_t pid) {
    std::lock_guard lock(processMutex);
    processList.erase(pid);
    updateSharedMemoryLocked();
    spdlog::debug("ProcessManager: Removed process with pid={}", pid);
}


void ProcessManager::printThreadSafeLog(const char *msg, bool isError, const char *subProcessPath) {
    auto timeIs = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm tm{};
    localtime_r(&timeIs, &tm);
    const char *colorStart = isError ? "\033[31m" : "";
    const char *colorEnd = isError ? "\033[0m" : "";
    fprintf(
        isError ? stderr : stdout,
        "%s↓--→[%02d:%02d:%02d] [PID %d] [%s] %s path=%s%s\n",
        colorStart,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        getpid(),
        isError ? "ERROR" : "INFO",
        msg,
        subProcessPath,
        colorEnd
    );
}

namespace {
    // Flag set by SIGINT handler
    //volatile sig_atomic_t is special type preffered for signal handlers by C++DOCS
    volatile sig_atomic_t g_sigintReceived = 0;
    std::atomic_bool g_sigintInstalled{false};

    void onSigint(int) {
        g_sigintReceived = 1;
    }
}

void ProcessManager::installSigintHandlerGlobally() {
    // Make sure that it was not installed by another ProcessManager already (case of sequential simullation)
    if (g_sigintInstalled.exchange(true)) return;

    struct sigaction sa{};
    sa.sa_handler = onSigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        perror("ProcessManager: sigaction(SIGINT) failed");
    }
}

void ProcessManager::requestShutdown() {
    shutdownRequested.store(true);
}

bool ProcessManager::isShutdownRequested() {
    if (shutdownRequested.load()) return true;
    if (g_sigintReceived != 0) shutdownRequested.store(true);
    return shutdownRequested.load();
}


void ProcessManager::reaperLoop() {
    printThreadSafeLog("Reaper thread for zombie cleanup started.", false, "");

    while (!stopReaper.load()) {
        if (isShutdownRequested()) break;
        int status = 0;
        pid_t pid = waitpid(-1, &status, WNOHANG);

        if (pid > 0) {
            {
                std::lock_guard lock(processMutex);
                auto it = processList.find(pid);
                if (it != processList.end()) {
                    printThreadSafeLog("Reaper: child process terminated", false, std::to_string(pid).c_str());
                    processList.erase(it);
                    updateSharedMemoryLocked();
                }
            }
        } else if (pid == 0) {
            continue;
        } else if (errno == ECHILD || errno == EINTR) {
            continue;
        } else {
            printThreadSafeLog("Reaper: waitpid error", true, "");
            perror("ProcessManager: waitpid error");
            break;
        }
    }

    printThreadSafeLog("Reaper thread for zombie cleanup stopped.", false, "");
}

void ProcessManager::updateSharedMemoryLocked() {
    auto *data = memory.getPtr();
    if (!data) {
        spdlog::error("ProcessManager: SharedMemory pointer is null");
        return;
    }

    data->lekarzePIDs.fill(0);
    data->processPIDs.fill(0);
    data->processCount = 0;

    std::size_t doctorIndex = 0;
    for (const auto &kv: processList) {
        const pid_t pid = kv.first;
        const auto *proc = kv.second.get();
        if (!proc) {
            continue;
        }
        if (proc->getProcessType() != ProcessType::REGISTRATION &&
            proc->getProcessType() != ProcessType::TRIAGE) {
            if (data->processCount < data->processPIDs.size()) {
                data->processPIDs[data->processCount++] = pid;
            }
        }
        if (proc->getProcessType() == ProcessType::DOCTOR && doctorIndex < data->lekarzePIDs.size()) {
            data->lekarzePIDs[doctorIndex++] = pid;
        }
    }
}
