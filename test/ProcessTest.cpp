
#include <sys/wait.h>

#include "Process.h"
#include "utils.hpp"
#include "catch2/catch_test_macros.hpp"
TEST_CASE("Process: test creation and deletion of processs") {return;
    try {
        Process proc = Process(ProcessType::TEST);
    }catch (const std::exception& e) {
        LOG->error("ProcessTest: Exception during Process creation: {}", e.what());
        FAIL("ProcessTest: Exception during Process creation");
    }

    // proc.~Process();
}