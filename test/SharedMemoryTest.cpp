#include "catch2/catch_test_macros.hpp"
#include "SharedMemory.hpp"

#include "utils.hpp"


TEST_CASE("SharedMemory create/assign and share data ", "[sharedmemory]") {
    SharedMemory shm{true};

    //assign sth
    auto *p = shm.getPtr();
    REQUIRE(p != nullptr);
    auto data = DataSOR();
    data.isOpen = true;
    data.lekarzePIDs[0] = 1234;

    //commit it
    *p = data;

    SharedMemory shm2;
    auto *p2 = shm2.getPtr();

    //read test
    REQUIRE(p2 != nullptr);
    REQUIRE(p2->isOpen == true);
    REQUIRE(p2->lekarzePIDs[0] == 1234);
}
