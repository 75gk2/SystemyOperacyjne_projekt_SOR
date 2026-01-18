#pragma once
#include <sched.h>

#include "SemaphoreArray.hpp"
#include "SharedMemory.hpp"


class Process {
private:
    pid_t pid;
    void printThreadSafeError(const char *msg, const char *subProcessPath);

protected:
    [[nodiscard]] pid_t getPid() const;

public:
    explicit Process(ProcessType processType);

    ~Process();
};
