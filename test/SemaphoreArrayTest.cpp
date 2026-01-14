

#include "SemaphoreArray.hpp"
#include "utils.hpp"
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Semaphore Array create/delete ", "[sharedmemory]") {
    logging::init();
    SemaphoreArray semArray{true};
    REQUIRE(semArray.pullUp(SemaphoreArray::SEM_TYPE::FIRST));
    REQUIRE(semArray.pullUp(SemaphoreArray::SEM_TYPE::FIRST));
    REQUIRE(semArray.pullDown(SemaphoreArray::SEM_TYPE::FIRST));
}
