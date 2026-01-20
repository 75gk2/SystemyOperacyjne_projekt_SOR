#include "ProcessManager.hpp"
#include "Process.hpp"

#include <csignal>
#include <sys/wait.h>
#include <mutex>
#include <spdlog/spdlog.h>
static ProcessManager* GLOBAL_PROCESS_MANAGER;
static std::mutex GLOBAL_PROCESS_MANAGER_MUTEX;

//global func for whole project
void ProcessManager::sigchldHandler(int sig) {
    if (GLOBAL_PROCESS_MANAGER == nullptr) return;

    pid_t child_pid;
    int status;

    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        std::lock_guard lock(GLOBAL_PROCESS_MANAGER_MUTEX);
        GLOBAL_PROCESS_MANAGER->removeProcess(child_pid);
    }
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
    std::lock_guard lock(GLOBAL_PROCESS_MANAGER_MUTEX);

    for (auto &[pid, process]: processList) {
        spdlog::info("ProcessManager: Terminating process with pid={}", pid);
    }
    processList.clear();

    signal(SIGCHLD, SIG_DFL);
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
            printThreadSafeError("Forked process: execl failed for process", process->path);

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


void ProcessManager::printThreadSafeError(const char *msg, const char *subProcessPath = "") {
    auto timeIs = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());

    tm tm{};
    localtime_r(&timeIs, &tm);
    fprintf(
        stderr,
        "↓--→[%02d:%02d:%02d] [PID %d] [error] %s path=%s\n",
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        // tm.tm_
        getpid(),
        msg,
        subProcessPath
    );
}
