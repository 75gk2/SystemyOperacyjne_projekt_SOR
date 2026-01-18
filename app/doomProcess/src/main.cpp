#include <unistd.h>

#include "utils.hpp"

int main(int argc, char *argv[]) {
    spdlog::info("DOOM PROCESS ACTIVE111");
    sleep(5);
    spdlog::info("Doom process exited");
}
