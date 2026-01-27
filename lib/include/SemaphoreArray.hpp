#pragma once
#include "GenericIPC.hpp"
#include "constants.hpp"

class SemaphoreArray : GenericIPC {
public:
    explicit SemaphoreArray(bool isCreator = false);

    ~SemaphoreArray() override;

    template<int N> // Template is workaround for not providing sops length manually
    bool operate(struct sembuf (&sops)[N]) const;

    [[nodiscard]] bool pullUp(SEM_TYPE semNum, unsigned short int byN = 1) const;

    [[nodiscard]] bool pullDown(SEM_TYPE semNum, unsigned short int byN = 1) const;

    [[nodiscard]] bool setValue(SEM_TYPE semNum, int value) const;

    [[nodiscard]] int getValue(SEM_TYPE semNum) const;


private:
    int semID;
    const int SEM_COUNT;
};
