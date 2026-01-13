#pragma once
#include <array>
#include <bits/types.h>

#include "constants.h"

class DataSOR {
public:
    bool isOpen = true;
    std::array<__pid_t, LEKARZE_COUNT> lekarzePIDs = {0};
};
