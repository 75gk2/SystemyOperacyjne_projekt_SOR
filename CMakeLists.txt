cmake_minimum_required(VERSION 3.10)
project(SOR_SO VERSION 1.0.0 LANGUAGES CXX C)

# C++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type when not provided by the generator
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Common warnings and diagnostics
add_compile_options(-Wall -Wextra)

# Debug/Release flags (adjust as needed)
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Build position independent code for libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Centralize output directories
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Option to enable/disable tests
#option(BUILD_TESTS "Build tests" ON)

# Subprojects
#add_subdirectory(lib)
add_subdirectory(app)
#if(BUILD_TESTS)
#    enable_testing()
#    add_subdirectory(test)
#endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}")
