#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>
#include <bits/stl_vector.h>

#include "Process.hpp"
#include "SemaphoreArray.hpp"
#include "SharedMemory.hpp"


class ProcessManager {
public:
    static void sigchldHandler(int sig);

    std::vector<pid_t> getPidsOfProcesses() const;
    // static ProcessManager* GLOBAL_PROCESS_MANAGER;
    // static std::mutex GLOBAL_PROCESS_MANAGER_MUTEX;

    ProcessManager();
    ~ProcessManager();

    [[nodiscard]] bool assignProcess(std::unique_ptr<Process> process);
    void removeProcess(pid_t pid);



private:
    std::unordered_map<pid_t, std::unique_ptr<Process>> processList;
    static void printThreadSafeLog(const char *msg, bool isError, const char *subProcessPath);


    SemaphoreArray semaphores;
    SharedMemory memory;
};
