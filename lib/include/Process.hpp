#pragma once
#include <sched.h>

#include "constants.hpp"


enum ProcessStatus {
    UNMANAGED,
    MANAGED,
    RUNNING,
    FINISHED,
};

class Process {
    pid_t pid = -1;
    ProcessStatus status;
    const char *path = nullptr;

public:
    [[nodiscard]] pid_t getPid() const;

    [[nodiscard]] pid_t isManaged() const;

    Process(ProcessType processType);

    virtual ~Process();


    // Disable copy and move semantics (required when destructor manages this process (0))
    Process(const Process &) = delete; //copy construction
    Process &operator=(const Process &) = delete; //
    Process(Process &&) = delete;

    Process &operator=(Process &&) = delete;

private:
    bool assignToManager();
    void setPid(pid_t procPid);

    friend class ProcessManager;
};
