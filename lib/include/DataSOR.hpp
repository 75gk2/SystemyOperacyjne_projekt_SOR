#pragma once
#include <array>
#include <bits/types.h>

#include "constants.hpp"

class DataSOR {
public:
    bool isOpen = true;
    int registerWindows = 0;
    std::array<__pid_t, LEKARZE_COUNT> lekarzePIDs = {0};
};
