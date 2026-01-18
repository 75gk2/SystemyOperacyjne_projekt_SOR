#include "constants.h"
#include "Process.h"
#include "utils.hpp"

int main() {
    spdlog::info("\n\n\n==========================\nMAIN: Initializing program\n==========================\n");
    spdlog::info("MAIN: path={}",DOOM_PATH);

    Process proc = Process(ProcessType::LEKARZ);
    Process proc2 = Process(ProcessType::TEST);
    return 0;
}
