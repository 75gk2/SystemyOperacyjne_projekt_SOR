#include  "SharedMemory.hpp"

#include <cstdio>
#include <sys/shm.h>

#include "utils.hpp"

// Create or repleace link to shared memory. Assert if got no -1 vars.
SharedMemory::SharedMemory(bool isCreator)
    : GenericIPC(isCreator), shmID(-1), ptr(nullptr) {
    _LOG->info("SharedMemory: init, isCreator={}", isCreator);

    const key_t key = ftok(".", SEM_PROJ_ID);
    if (key == -1) {
        _LOG->error("SharedMemory: ftok failed");
        throw std::runtime_error("ftok failed");
    }

    shmID = shmget(key, sizeof(DataSOR), getFlag());

    if (shmID == -1) {
        _LOG->error("SharedMemory: shmget failed");
        throw std::runtime_error("shmget failed");
    }

    ptr = static_cast<DataSOR *>(shmat(shmID, nullptr, 0));
    if (isPtrNegative()) {
        _LOG->error("SharedMemory: shmat failed");
        throw std::runtime_error("shmat failed");
    }

    _LOG->info("SharedMemory: init compleated! key={}, shmID={}", key, shmID);
}


SharedMemory::~SharedMemory() {
    if (!isPtrNegative()) {
        shmdt(this->ptr);
        this->ptr = nullptr;
    }

    if (isThisCreator()) {
        _LOG->info("SharedMemory: destructor of creator, deleting segment shmID={}", shmID);
        if (shmID != -1) {
            if (const auto result = shmctl(shmID, IPC_RMID, nullptr); result == -1) {
                _LOG->error("SharedMemory: deletion of shared memory failed! RISK OF LEAK! shmID={}", shmID);
            }
            shmID = -1;
        } else {
            _LOG->error("SharedMemory: Owner lost access to schID!");
        }
    }
    _LOG->info("SharedMemory: destructor completed");
}

DataSOR *SharedMemory::getPtr() const {
    return this->ptr;
}

bool SharedMemory::isPtrNegative() const {
    return (!ptr || ptr == reinterpret_cast<DataSOR *>(-1));
}
