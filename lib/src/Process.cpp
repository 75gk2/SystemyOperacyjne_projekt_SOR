#include  "Process.hpp"

#include <unistd.h>

#include "utils.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include <spdlog/spdlog.h>

pid_t Process::getPid() const {
    return pid;
}

pid_t Process::isManaged() const {
    return this->status > UNMANAGED;
}

Process::Process(ProcessType processType) : status(UNMANAGED) {
    path = getProcessExecPath(processType);
}

Process::~Process() {
    if (pid > 0 && status == RUNNING) {
        spdlog::info("Process: Terminating process with pid={}", pid);
        kill(pid, SIGTERM);
        
        int status;
        waitpid(pid, &status, 0); // blocking wait
        spdlog::info("Process: Process with pid={} terminated", pid);
    }
}

bool Process::assignToManager() {
    if (isManaged()) return false;
    this->status = MANAGED;
    return true;
}

void Process::setPid(pid_t procPid) {
    this->pid = procPid;
}
