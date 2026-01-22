#pragma once
#include "DataSOR.h"
#include "GenericIPC.hpp"

class SharedMemory:GenericIPC {
public:
    explicit SharedMemory(bool isCreator = false);

    ~SharedMemory() override;

    [[nodiscard]] DataSOR *getPtr() const;

private:
    int shmID;
    DataSOR *ptr;

    [[nodiscard]] bool isPtrNegative() const;
};
