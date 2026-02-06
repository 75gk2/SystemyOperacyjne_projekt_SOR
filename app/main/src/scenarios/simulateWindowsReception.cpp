#include <unistd.h>

#include "manageSimulations.hpp"
#include "ProcessManager.hpp"
#include "childProcesses/Registration.hpp"
#include "spdlog/spdlog.h"
#include "childProcesses/Triage.hpp"

void simulateManageRegistrationWindows() {
    ProcessManager pm;
    spdlog::info("MAIN: Assigning Registration process, result={}",
                 pm.assignProcess(std::make_unique<Registration>(1000, 5)));
    
    spdlog::info("MAIN: Assigning Triage process, result={}",
                 pm.assignProcess(std::make_unique<Triage>()));

    for (int j = 1; j <= 5; j++) {
        for (int i = 1; i <= 1000; i++) {
            spdlog::info("MAIN: Assigning Patient process, result={}",
                         pm.assignProcess(std::make_unique<Patient>(i, false, false, false)));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));
}