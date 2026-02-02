#pragma once
#include <array>
#include <cstddef>
#include <bits/types.h>

#include "constants.hpp"

class DataSOR {
public:
    bool isOpen = true;
    int registerWindows = 0;
    std::array<__pid_t, LEKARZE_COUNT> lekarzePIDs = {0};
    std::array<__pid_t, MAX_PROCESS_PIDS> processPIDs = {0};
    std::size_t processCount = 0;
};