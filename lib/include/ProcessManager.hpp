#pragma once
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <bits/stl_vector.h>

#include "Process.hpp"
#include "SemaphoreArray.hpp"
#include "SharedMemory.hpp"


class ProcessManager {
public:
    std::vector<pid_t> getPidsOfProcesses() const;
    // static ProcessManager* GLOBAL_PROCESS_MANAGER;
    // static std::mutex GLOBAL_PROCESS_MANAGER_MUTEX;

    ProcessManager();
    ~ProcessManager();

    [[nodiscard]] bool assignProcess(std::unique_ptr<Process> process);
    void removeProcess(pid_t pid);

    // True after SIGINT or requestShutdown()
    [[nodiscard]] bool isShutdownRequested();


private:
    std::unordered_map<pid_t, std::unique_ptr<Process>> processList;
    static void printThreadSafeLog(const char *msg, bool isError, const char *subProcessPath);

    void reaperLoop();

    void installSigintHandlerGlobally();
    void requestShutdown();

    std::atomic_bool shutdownRequested{false};

    std::atomic_bool stopReaper{false};
    std::thread reaperThread;
    mutable std::mutex processMutex;


    SemaphoreArray semaphores;
    SharedMemory memory;
};
