

#include "SemaphoreArray.hpp"
#include "utils.hpp"
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Semaphore Array create/delete ", "[sharedmemory]") {
    SemaphoreArray semArray{true};
    REQUIRE(semArray.pullUp(SEM_TYPE::TEST));
    REQUIRE(semArray.pullUp(SEM_TYPE::TEST));
    REQUIRE(semArray.pullDown(SEM_TYPE::TEST));
}
