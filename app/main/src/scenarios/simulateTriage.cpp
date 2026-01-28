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
    MessageQueue doctorIn(Doctor::QID_DOCTOR_IN, true);
    MessageQueue doctorOut(Doctor::QID_DOCTOR_OUT, true);
    spdlog::info("MAIN: Assigning Registration process, result={}",
        pm.assignProcess(std::make_unique<Registration>(10)));
    spdlog::info("MAIN: Assigning Triage process, result={}",
        pm.assignProcess(std::make_unique<Triage>()));
    for (int i = 0; i < static_cast<int>(Triage::COUNT); i++) {
        spdlog::info("MAIN: Assigning Doctor process for specialist={}, result={}",
            i,
            pm.assignProcess(std::make_unique<Doctor>(static_cast<Triage::Specialist>(i), 50)));
    }

    for (int i = 1; i <= 10000; i++) {
        spdlog::info("MAIN: Assigning Patient process, id={}, result={}",
        i,
        pm.assignProcess(std::make_unique<Patient>(i, false, false)));
    }
    spdlog::warn("MAIN: Simulation running for 60 seconds");
        (void)sleep(500);
    spdlog::warn("MAIN: Time is up, termination");
}
