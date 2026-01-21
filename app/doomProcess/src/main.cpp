#include <unistd.h>

#include "utils.hpp"

int main() {
    // spdlog::info("Doom: Initializing process");
    for (int i = 1; i <= 5; i++) {
        // spdlog::info("Doom: Task {}/5",i);
        sleep(1);
    }
    spdlog::info("Doom process ok!");
}
