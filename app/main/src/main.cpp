#include "constants.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"
#include "utils.hpp"
#include <unistd.h>

#include "childProcesses/Patient.hpp"
#include "childProcesses/Registration.hpp"


int main() {
    spdlog::info("\n\n\n==========================\nMAIN: Initializing program\n==========================\n");
    spdlog::info("MAIN: path={}",DOOM_PATH);

    ProcessManager pm;
    spdlog::info("MAIN: Assigning Registration process, result={}",
        pm.assignProcess(std::make_unique<Registration>()));

    spdlog::info("MAIN: Assigning Patient process, result={}",
        pm.assignProcess(std::make_unique<Patient>()));
    // for (int i = 1; i <= 10000; i++) {
    //     if (pm.assignProcess(std::make_unique<Process>(ProcessType::TEST))) {
    //         spdlog::info("→Runn test");
    //     }
    // }

    sleep(5);
    // ProcessManager automatycznie posprzata procesy w destruktorze
    pm.~ProcessManager();
    // sleep(60); //czekaj na sprawdzenie zombie procesow

    return 0;
}
