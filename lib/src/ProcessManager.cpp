#include "ProcessManager.hpp"
#include "Process.hpp"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <sys/wait.h>
#include <thread>
#include <vector>
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
                                   memory(true) {
    reaperThread = std::thread(&ProcessManager::reaperLoop, this);
    spdlog::debug("ProcessManager: Reaper thread started");
}

ProcessManager::~ProcessManager() {
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
            // TODO! : Make sure that result of this process is HANDLED by parent process to avoid zombie
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
    }
    spdlog::debug("ProcessManager: Added process with pid={}", pid);
    return true;
}


void ProcessManager::removeProcess(pid_t pid) {
    std::lock_guard lock(processMutex);
    processList.erase(pid);
    spdlog::debug("ProcessManager: Removed process with pid={}", pid);
}


void ProcessManager::printThreadSafeLog(const char *msg, bool isError = true, const char *subProcessPath = "") {
    auto timeIs = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());

    tm tm{};
    localtime_r(&timeIs, &tm);
    fprintf(
        isError ? stderr : stdout,
        "↓--→[%02d:%02d:%02d] [PID %d] [%s] %s path=%s\n",
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        // tm.tm_
        getpid(),
        isError ? "ERROR" : "INFO",
        msg,
        subProcessPath
    );
}

void ProcessManager::reaperLoop() {
    printThreadSafeLog("Reaper thread for zombie cleanup started.", false, "");

    while (!stopReaper.load()) {
        int status = 0;
        pid_t pid = waitpid(-1, &status, WNOHANG);

        if (pid > 0) {
            {
                std::lock_guard lock(processMutex);
                auto it = processList.find(pid);
                if (it != processList.end()) {
                    printThreadSafeLog("Reaper: child process terminated", false, std::to_string(pid).c_str());
                    processList.erase(it);
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
