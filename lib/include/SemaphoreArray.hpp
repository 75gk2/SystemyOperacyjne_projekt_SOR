#pragma once
#include "GenericIPC.hpp"

class SemaphoreArray : GenericIPC {
public:
    explicit SemaphoreArray(bool isCreator = false);

    ~SemaphoreArray() override;

    enum class SEM_TYPE {
        FIRST,
        __COUNT_SENTINEL, // NOLINT(*-reserved-identifier)
    };

    template<int N> // Template is workaround for not providing sops length manually
    bool operate(struct sembuf (&sops)[N]) const;

    [[nodiscard]] bool pullUp(SEM_TYPE semNum, unsigned short int byN = 1) const;

    [[nodiscard]] bool pullDown(SEM_TYPE semNum, unsigned short int byN = 1) const;

private:
    int semID;
    const int SEM_COUNT;
};
