#include "constants.h"
#include "Process.hpp"
#include "ProcessManager.hpp"
#include "utils.hpp"
#include <unistd.h>


int main() {
    spdlog::info("\n\n\n==========================\nMAIN: Initializing program\n==========================\n");
    spdlog::info("MAIN: path={}",DOOM_PATH);

    ProcessManager pm;
    for (int i = 1; i <= 10000; i++) {
        if (pm.assignProcess(std::make_unique<Process>(ProcessType::TEST))) {
            spdlog::info("→Runn test");
        }
    }
    
    // ProcessManager automatycznie posprzata procesy w destruktorze
    pm.~ProcessManager();
    sleep(60); //czekaj na sprawdzenie zombie procesow

    return 0;
}
