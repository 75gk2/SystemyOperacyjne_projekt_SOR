#include  "Process.h"

#include <unistd.h>

#include "utils.hpp"
#include <sys/wait.h>


pid_t Process::getPid() const {
    return pid;
}

Process::Process(ProcessType processType) {
    const char *path = getProcessExecPath(processType);

    switch (pid = fork()) {
        case -1:
            LOG->error("Process: Fork failed for process path={}", path);
            throw std::runtime_error("Process: Fork failed - can't create Process object");
        case 0:
            execl(path, path, static_cast<char*>(nullptr));
            _exit(EXIT_FAILURE);
            break;
        default:
            LOG->info("Process: forked successfully for Process with pid={}", pid);;
    }
}

Process::~Process() {
    if (pid > 0) {
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
    }
}
