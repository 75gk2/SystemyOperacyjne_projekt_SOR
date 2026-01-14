#pragma once
#include "DataSOR.h"

class SharedMemory {
public:
    explicit SharedMemory(bool isCreator = false);

    ~SharedMemory();

    DataSOR *getPtr() const;

private:
    int shmID;
    bool isThisCreator;
    DataSOR *ptr;

    bool isPtrNegative() const;
};
