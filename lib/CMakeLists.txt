add_library(sorLib STATIC
        src/utils.cpp
        src/DataSOR.cpp
        include/DataSOR.hpp
        include/SharedMemory.hpp
        src/SharedMemory.cpp
        include/utils.hpp
        include/SemaphoreArray.hpp
        include/GenericIPC.hpp
        src/GenericIPC.cpp
        src/SemaphoreArray.cpp
        include/Process.hpp
        src/Process.cpp
        include/ProcessManager.hpp
        src/ProcessManager.cpp
        include/MessageQueue.hpp
        include/MessageQueue.tpp
        src/MessageQueue.cpp
)

target_include_directories(sorLib PUBLIC include)
target_link_libraries(sorLib PUBLIC spdlog::spdlog)
target_link_libraries(sorLib PUBLIC global_config)
target_link_libraries(sorLib PUBLIC test_config)