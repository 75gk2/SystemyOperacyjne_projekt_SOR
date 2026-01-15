#include <cstdio>
#include <unistd.h>

#include "constants.h"
#include "Process.h"
#include "utils.hpp"

int main() {
    LOG->info("\n\n\n==========================\nMAIN: Initializing program\n==========================\n");
    LOG->info("MAIN: path={}",DOOM_PATH);

    Process proc = Process(ProcessType::TEST);
    // sleep(120);
    return 0;
}
