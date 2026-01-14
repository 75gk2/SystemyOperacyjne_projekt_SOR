cmake_minimum_required(VERSION 3.10)
project(SOR_Tests LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

file(GLOB TEST_SOURCES *.cpp)

add_executable(sor_tests ${TEST_SOURCES}
        SemaphoreArrayTest.cpp)


target_link_libraries(sor_tests PRIVATE sorLib)
target_link_libraries(sor_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(sor_tests)
