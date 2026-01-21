
#include <sys/wait.h>

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


TEST_CASE("Process: test creation and deletion of 10000 processs, then exit immidiatelly!") {
    try {
        ProcessManager pm;
        for (int i = 1; i <= 10000; i++) {
            pm.assignProcess(std::make_unique<Process>(ProcessType::TEST));
        }
        pm.~ProcessManager();
        //Manually check for zombie processes after test
    }catch (const std::exception& e) {
        spdlog::error("ProcessTest: Exception during Process creation: {}", e.what());
        FAIL("ProcessTest: Exception during Process creation");
    }
}