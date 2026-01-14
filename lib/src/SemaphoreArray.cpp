#include "SemaphoreArray.hpp"

#include <sys/ipc.h>
#include <sys/sem.h>

#include "constants.h"
#include "utils.hpp"

SemaphoreArray::SemaphoreArray(bool isCreator)
    : GenericIPC(isCreator), semID(-1), SEM_COUNT(static_cast<int>(SEM_TYPE::__COUNT_SENTINEL)) {
    LOG->info("SemaphoreArray: init, isCreator={}", isCreator);

    const key_t key = ftok(".", SEM_PROJ_ID);
    if (key == -1) {
        LOG->error("SemaphoreArray: ftok failed");
        throw std::runtime_error("ftok failed");
    }

    semID = semget(key, SEM_COUNT, getFlag());

    if (semID == -1) {
        LOG->error("SemaphoreArray: semget failed");
        throw std::runtime_error("semget failed");
    }

    LOG->info("SemaphoreArray: init compleated! key={}, semID={}", key, semID);
}

SemaphoreArray::~SemaphoreArray() {
    if (isThisCreator()) {
        LOG->info("SemaphoreArray: destructor of creator, deleting semaphores semID={}", semID);
        if (semID != -1) {
            if (const auto result = semctl(semID, 0, IPC_RMID); result == -1) {
                LOG->error("SemaphoreArray: deletion of semaphore array failed! RISK OF LEAK! semID={}", semID);
            }
            semID = -1;
        } else {
            LOG->error("SemaphoreArray: Owner lost access to semID!");
        }
    }
}

template<int N>
bool SemaphoreArray::operate(struct sembuf (&sops)[N]) const{
    if (semop(semID, sops, N) == -1) {
        LOG->error("SemaphoreArray: operate (atomic) semop failed!");
        return false;
    }
    return true;
}

bool SemaphoreArray::pullUp(SEM_TYPE semNum, const unsigned short int byN) const {
    struct sembuf sops[1];
    sops[0].sem_num = static_cast<unsigned short int>(semNum);
    sops[0].sem_op = static_cast<short int>(byN);
    sops[0].sem_flg = 0;

    if (!operate(sops)) {
        LOG->error("SemaphoreArray: pullUp semop failed!");
        return false;
    }
    return true;
}

bool SemaphoreArray::pullDown(SEM_TYPE semNum, const unsigned short int byN) const {
    struct sembuf sops[1];
    sops[0].sem_num = static_cast<unsigned short int>(semNum);
    sops[0].sem_op = -1 * static_cast<short int>(byN);
    sops[0].sem_flg = 0;

    if (!operate(sops)) {
        LOG->error("SemaphoreArray: pullDown semop failed!");
        return false;
    }
    return true;
}
