#include  "Process.hpp"

#include <unistd.h>
#include <utility>

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

Process::Process(ProcessType processType, std::vector<std::string> extraArgs)
    : status(UNMANAGED), extraArgs(std::move(extraArgs)), processType(processType) {
    path = getProcessExecPath(processType);
}

ProcessType Process::getProcessType() const {
    return processType;
}

Process::~Process() {
    if (pid > 0 && status == RUNNING) {
        spdlog::debug("Process: Terminating process with pid={}", pid);
        kill(pid, SIGTERM);

        int status;
        //handle in while in case of interruption by intrupt signal
        while (true) {
            const pid_t result = waitpid(pid, &status, 0); // blocking wait

            //process killed successfully
            if (result == pid) break;

            //just interrupted not killed!!!!!! Do next iteration of while
            if (result == -1 && errno == EINTR) continue;

            //Crashed elsewhere or reaped - no need to wait!
            if (result == -1 && errno == ECHILD) break;
            break;
        }
        spdlog::debug("Process: Process with pid={} terminated", pid);
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