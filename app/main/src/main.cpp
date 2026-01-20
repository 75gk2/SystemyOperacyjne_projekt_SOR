#include "constants.h"
#include "Process.hpp"
#include "ProcessManager.hpp"
#include "utils.hpp"
#include <unistd.h>


int main() {
    spdlog::info("\n\n\n==========================\nMAIN: Initializing program\n==========================\n");
    spdlog::info("MAIN: path={}",DOOM_PATH);

    ProcessManager pm;
    auto process = std::make_unique<Process>(ProcessType::TEST);
    if (pm.assignProcess(std::move(process))) {
        spdlog::info("MAIN: Process initialized");
    }
sleep(4);

    
    // ProcessManager automatycznie posprzata procesy w destruktorze
    return 0;
}
