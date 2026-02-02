#include "catch2/catch_test_macros.hpp"
#include "MessageQueue.hpp"
#include "ProcessManager.hpp"
#include "SemaphoreArray.hpp"
#include "childProcesses/Registration.hpp"
#include "constants.hpp"

#include <cstring>
#include <memory>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>


TEST_CASE (
"Registration scenario: open and close windows based on queue size"
,
"[registration]"
)
 {
    ProcessManager pm;
    REQUIRE(pm.assignProcess(std::make_unique<Registration>(1)));
}