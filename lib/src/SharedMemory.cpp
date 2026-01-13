#include  "SharedMemory.hpp"

#include <cstdio>
#include <stdexcept>
#include <sys/shm.h>

#include "utils.hpp"

bool SharedMemory::isPtrNegative() const {
    return (!ptr || ptr == reinterpret_cast<DataSOR *>(-1));
}

// Create or repleace link to shared memory. Assert if got no -1 vars.
SharedMemory::SharedMemory(const char *path) {
    const key_t key = ftok(path, 1);
    if (key == -1) {
        _LOG->error("SharedMemory: ftok failed");
        throw std::runtime_error("ftok failed");
    }

    shmID = shmget(key, sizeof(DataSOR), 0600); //TODO sprawdzić czy prawa dostępu są ok
    if (shmID == -1) {
        _LOG->error("SharedMemory: shmget failed");
        throw std::runtime_error("shmget failed");
    }

    ptr = static_cast<DataSOR *>(shmat(shmID, nullptr, 0));
    if (isPtrNegative()) {
        _LOG->error("SharedMemory: shmat failed");
        throw std::runtime_error("shmat failed");
    }
}


SharedMemory::~SharedMemory() {
    if (!isPtrNegative()) {
        shmdt(this->ptr);
        this->ptr = nullptr;
    }

    if (shmID != -1) {
        shmctl(shmID, IPC_RMID, nullptr);
        shmID = -1;
    }

}

DataSOR *SharedMemory::getPtr() const {
    return this->ptr;
}
