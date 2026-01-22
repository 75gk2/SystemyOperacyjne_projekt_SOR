#include "ProcessManager.hpp"
#include "Process.hpp"

#include <csignal>
#include <sys/wait.h>
#include <mutex>
#include <spdlog/spdlog.h>
static ProcessManager *GLOBAL_PROCESS_MANAGER;
static std::mutex GLOBAL_PROCESS_MANAGER_MUTEX;

//global func for whole project
void ProcessManager::sigchldHandler(int sig) {
    if (GLOBAL_PROCESS_MANAGER == nullptr) {
        printThreadSafeLog("[CRITICAL]: PM sigchildHandler got event, but no PM defined!", true, "");
        return;
    }
    pid_t child_pid;
    int status;

    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printThreadSafeLog("SIGCHLD handler: Child process terminated", false,std::to_string(child_pid).c_str());

        std::lock_guard lock(GLOBAL_PROCESS_MANAGER_MUTEX);
        GLOBAL_PROCESS_MANAGER->removeProcess(child_pid);
    }
}

std::vector<pid_t> ProcessManager::getPidsOfProcesses() const {
    std::vector<pid_t> keys;
    keys.reserve(processList.size());

    for (const auto &kv: processList) {
        keys.push_back(kv.first);
    }
    return keys;
}


ProcessManager::ProcessManager() {
    GLOBAL_PROCESS_MANAGER = this;

    struct sigaction sa;
    sa.sa_handler = sigchldHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;

    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        spdlog::error("ProcessManager: Failed to register SIGCHLD handler");
        throw std::runtime_error("Failed to register SIGCHLD handler");
    }

    spdlog::info("ProcessManager: Initialized with SIGCHLD handler");
}

ProcessManager::~ProcessManager() {
    // Disable SIGCHLD handler first to prevent concurrent modification during cleanup
    signal(SIGCHLD, SIG_DFL);
    
    std::lock_guard lock(GLOBAL_PROCESS_MANAGER_MUTEX);

    spdlog::info("ProcessManager: Terminating processes number={}", processList.size());

    // Clear the process list, which will call Process destructors
    // Each Process destructor will kill and wait for its process
    processList.clear();

    GLOBAL_PROCESS_MANAGER = nullptr;

    spdlog::info("ProcessManager: Destroyed");
}

bool ProcessManager::assignProcess(std::unique_ptr<Process> process) {
    if (!process->assignToManager()) return false;
    std::lock_guard lock(GLOBAL_PROCESS_MANAGER_MUTEX);

    switch (process->pid = fork()) {
        case -1:
            spdlog::error("Process: Fork failed for process path={}", process->path);
            throw std::runtime_error("Process: Fork failed - can't create Process object");

        case 0: {
            execl(process->path, process->path, static_cast<char *>(nullptr));
            printThreadSafeLog("Forked process: execl failed for process", true, process->path);
            // TODO! : Make sure that result of this process is HANDLED by parent process to avoid zombie
            //return without calling any copied destructors
            _exit(EXIT_FAILURE);
        }
        default:
            spdlog::info("Process: forked successfully for Process with pid={}", process->pid);
            process->status = RUNNING;
    }
    pid_t pid = process->getPid();
    processList[pid] = std::move(process);
    spdlog::info("ProcessManager: Added process with pid={}", pid);
    return true;
}


void ProcessManager::removeProcess(pid_t pid) {
    processList.erase(pid);
    spdlog::info("ProcessManager: Removed process with pid={}", pid);
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
