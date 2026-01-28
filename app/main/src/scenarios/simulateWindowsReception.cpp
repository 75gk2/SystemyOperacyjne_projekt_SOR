#include <unistd.h>

#include "manageSimulations.hpp"
#include "ProcessManager.hpp"
#include "childProcesses/Registration.hpp"
#include "spdlog/spdlog.h"

void simulateManageRegistrationWindows() {

    ProcessManager pm;
    spdlog::info("MAIN: Assigning Registration process, result={}",
        pm.assignProcess(std::make_unique<Registration>(10, 100)));

    for (int j = 1; j <= 3; j++) {
        for (int i = 1; i <= 30; i++) {
            spdlog::info("MAIN: Assigning Patient process, result={}",
                pm.assignProcess(std::make_unique<Patient>(1, false, false)));
        }
        sleep(5);
    }
    sleep(15);
}
