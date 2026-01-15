#pragma once
#include <sched.h>

#include "SemaphoreArray.hpp"
#include "SharedMemory.hpp"


class Process {
private:
    pid_t pid;

protected:
    [[nodiscard]] pid_t getPid() const;

public:
    explicit Process(ProcessType processType);

    ~Process();
};
