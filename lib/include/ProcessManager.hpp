#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>

#include "Process.hpp"


class ProcessManager {
public:
    static void sigchldHandler(int sig);
    // static ProcessManager* GLOBAL_PROCESS_MANAGER;
    // static std::mutex GLOBAL_PROCESS_MANAGER_MUTEX;

    ProcessManager();
    ~ProcessManager();

    bool assignProcess(std::unique_ptr<Process> process);
    void removeProcess(pid_t pid);

private:
    std::unordered_map<pid_t, std::unique_ptr<Process>> processList;
    static void printThreadSafeError(const char *msg, const char *subProcessPath);
};
