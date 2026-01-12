add_library(sorLib STATIC
        src/utils.cpp
        src/DataSOR.cpp
        include/DataSOR.h
)

target_include_directories(sorLib PUBLIC include)
