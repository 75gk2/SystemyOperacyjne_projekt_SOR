#include  "Process.hpp"

#include <unistd.h>

#include "utils.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>

pid_t Process::getPid() const {
    return pid;
}

pid_t Process::isManaged() const {
    return this->status > UNMANAGED;
}

Process::Process(ProcessType processType): status(UNMANAGED) {
    path=getProcessExecPath(processType);
}

Process::~Process() {
    spdlog::info("Process: Destructor called, pid={}", pid);

    if (pid > 0) {
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
    }
    spdlog::info("Process: Destructor ended, pid={}", pid);
}

bool Process::assignToManager() {
    if (isManaged() ) return false;
    this->status = MANAGED;
    return true;
}

void Process::setPid(pid_t procPid) {
    this->pid = procPid;
}
