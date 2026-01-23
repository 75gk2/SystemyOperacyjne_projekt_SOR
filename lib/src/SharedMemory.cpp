#include  "SharedMemory.hpp"

#include <cstdio>
#include <sys/shm.h>

#include "utils.hpp"

// Create or repleace link to shared memory. Assert if got no -1 vars.
SharedMemory::SharedMemory(bool isCreator)
    : GenericIPC(isCreator), shmID(-1), ptr(nullptr) {
    spdlog::debug("SharedMemory: init, isCreator={}", isCreator);

    const key_t key = ftok(".", SEM_PROJ_ID);
    if (key == -1) {
        spdlog::error("SharedMemory: ftok failed");
        throw std::runtime_error("ftok failed");
    }

    shmID = shmget(key, sizeof(DataSOR), getFlag());

    if (shmID == -1) {
        spdlog::error("SharedMemory: shmget failed");
        throw std::runtime_error("shmget failed");
    }

    ptr = static_cast<DataSOR *>(shmat(shmID, nullptr, 0));
    if (isPtrNegative()) {
        spdlog::error("SharedMemory: shmat failed");
        throw std::runtime_error("shmat failed");
    }

    spdlog::debug("SharedMemory: init compleated! key={}, shmID={}", key, shmID);
}


SharedMemory::~SharedMemory() {
    if (!isPtrNegative()) {
        shmdt(this->ptr);
        this->ptr = nullptr;
    }

    if (isThisCreator()) {
        spdlog::debug("SharedMemory: destructor of creator, deleting segment shmID={}", shmID);
        if (shmID != -1) {
            if (const auto result = shmctl(shmID, IPC_RMID, nullptr); result == -1) {
                spdlog::error("SharedMemory: deletion of shared memory failed! RISK OF LEAK! shmID={}", shmID);
            }
            shmID = -1;
        } else {
            spdlog::error("SharedMemory: Owner lost access to schID!");
        }
    }
    spdlog::debug("SharedMemory: destructor completed");
}

DataSOR *SharedMemory::getPtr() const {
    return this->ptr;
}

bool SharedMemory::isPtrNegative() const {
    return (!ptr || ptr == reinterpret_cast<DataSOR *>(-1));
}
