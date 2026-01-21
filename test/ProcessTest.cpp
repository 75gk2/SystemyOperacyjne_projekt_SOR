
#include <sys/wait.h>
#include <stdio.h>
#include "Process.hpp"
#include "ProcessManager.hpp"
#include "utils.hpp"
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Process: test creation and deletion of single processs") {
    try {
        ProcessManager pm;
        if (pm.assignProcess(std::make_unique<Process>(ProcessType::TEST))) {
            spdlog::info("MAIN: Process initialized");
            sleep(6);
        }
    }catch (const std::exception& e) {
        spdlog::error("ProcessTest: Exception during Process creation: {}", e.what());
        FAIL("ProcessTest: Exception during Process creation");
    }
}


TEST_CASE("Process: create 10k and killl imidiatelly") {
    try {
        const auto pm = new ProcessManager();
        for (int i = 1; i <= 10000; i++) {
            if (pm->assignProcess(std::make_unique<Process>(ProcessType::TEST))) {
                spdlog::info("→Runn test");
            }
        }
        //Manually check for zombie processes after test
        delete pm;
        spdlog::info("ProcessTest: Destructor compleated, check for zombies, wait 10 s...");
        std::this_thread::sleep_for(std::chrono::seconds(10));
        spdlog::info("ProcessTest: Wait compleated");
    }catch (const std::exception& e) {
        spdlog::error("ProcessTest: Exception during Process creation: {}", e.what());
        FAIL("ProcessTest: Exception during Process creation");
    }
}


TEST_CASE("Process: 10k proc, then let them finish") {

    try {
        ProcessManager pm;
        for (int i = 1; i <= 10000; i++) {
            if (pm.assignProcess(std::make_unique<Process>(ProcessType::TEST))) {
                spdlog::info("→Runn test");
            }
        }
        //Manually check for zombie processes after test
        spdlog::info("ProcessTest: generated processes: wait 10 s, then check for zombies, wait: 15s");
        std::this_thread::sleep_for(std::chrono::seconds(15));
        spdlog::info("ProcessTest: Wait compleated");
    }catch (const std::exception& e) {
        spdlog::error("ProcessTest: Exception during Process creation: {}", e.what());
        FAIL("ProcessTest: Exception during Process creation");
    }
}