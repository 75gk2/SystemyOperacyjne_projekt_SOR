#include  "Process.h"

#include <unistd.h>

#include "utils.hpp"
#include <unistd.h>
#include <sys/wait.h>

Process::Process(ProcessType processType) {
    const char *path = getProcessExecPath(processType);

    switch (pid = fork()) {
        case -1:
            spdlog::error("Process: Fork failed for process path={}", path);
            throw std::runtime_error("Process: Fork failed - can't create Process object");
        case 0: {
            execl(path, path, static_cast<char *>(nullptr));
            // LoggerInitializer(); // reinitialize logger to avoid mutex duplication
            // spdlog::error("Process: Execl failed for process, exiting... path={}", path);
            printThreadSafeError("Forked process: execl failed for process", path);
            exit(EXIT_FAILURE);
            break;
        }
        default:
            spdlog::info("Process: forked successfully for Process with pid={}", pid);
    }
}

Process::~Process() {
    spdlog::info("Process: Destructor called, pid={}", pid);
    if (pid > 0) {
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
    }
    spdlog::info("Process: Destructor ended, pid={}", pid);
}

pid_t Process::getPid() const {
    return pid;
}

void Process::printThreadSafeError(const char *msg, const char *subProcessPath="") {
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
