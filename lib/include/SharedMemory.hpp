#pragma once
#include "DataSOR.h"

// !TODO: test this feature before implementing usage
class SharedMemory {
private:
    int shmID;
    DataSOR *ptr;
    bool isPtrNegative() const;

public:
    explicit SharedMemory(const char *path);

    ~SharedMemory();

    DataSOR *getPtr() const;
};
