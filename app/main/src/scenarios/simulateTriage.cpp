#include <unistd.h>

#include "manageSimulations.hpp"
#include "ProcessManager.hpp"
#include "childProcesses/Patient.hpp"
#include "childProcesses/Registration.hpp"
#include "childProcesses/Triage.hpp"
#include "childProcesses/Doctor.hpp"
#include "spdlog/spdlog.h"

void simulateTriage() {
    spdlog::info("MAIN: Starting Triage simulation");
    ProcessManager pm;
    spdlog::info("MAIN: Assigning Registration process, result={}",
                 pm.assignProcess(std::make_unique<Registration>(0)));
    spdlog::info("MAIN: Assigning Triage process, result={}",
                 pm.assignProcess(std::make_unique<Triage>()));
    for (int i = 0; i < static_cast<int>(Triage::COUNT); i++) {
        spdlog::info("MAIN: Assigning Doctor process for specialist={}, result={}",
                     i,
                     pm.assignProcess(std::make_unique<Doctor>(static_cast<Triage::Specialist>(i), 0)));
    }

    for (int i = 1; i <= 20000; i++) {
        spdlog::info("MAIN: Assigning Patient process, id={}, result={}",
                     i,
                     pm.assignProcess(std::make_unique<Patient>(i, false, false, true)));
    }
    // spdlog::warn("MAIN: Simulation running for 60 seconds");
    (void) sleep(20);
    spdlog::warn("MAIN: Time is up, termination");
}
