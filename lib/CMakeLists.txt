add_library(sorLib STATIC
        src/utils.cpp
        src/DataSOR.cpp
        include/DataSOR.h
        include/SharedMemory.hpp
        src/SharedMemory.cpp
        include/utils.hpp
)

target_include_directories(sorLib PUBLIC include)
target_link_libraries(sorLib PUBLIC spdlog::spdlog)