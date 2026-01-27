#pragma once

#define POCZEKALNIA_SIZE 20 // 4 or more  to keep sense
// Values K1 and K2 already mapped as semaphore guards
// #define K1 ((POCZEKALNIA_SIZE + 1) / 2 + 1)
// #define K2 ((POCZEKALNIA_SIZE) / 3 - 1)

#define LEKARZE_COUNT 3

// IPC Keys
#define SHM_PROJ_ID 'M'
#define SEM_PROJ_ID 'S'
#define MSG_PROJ_ID 'Q'

enum class ProcessType {
    DOCTOR,
    PATIENT,
    REGISTRATION,
    TRIAGE,
    WAITING_ROOM,
    TEST,
    __COUNT_SENTINEL
};

inline const char *getProcessExecPath(const ProcessType type) {
    switch (type) {
        case ProcessType::DOCTOR:
            return DOCTOR_PROC_PATH;
        case ProcessType::PATIENT:
            return PATIENT_PROC_PATH;
        case ProcessType::REGISTRATION:
            return REGISTRATION_PROC_PATH;
        case ProcessType::TRIAGE:
            return TRIAGE_PROC_PATH;
        case ProcessType::WAITING_ROOM:
            return WAITING_ROOM_PROC_PATH;
        case ProcessType::TEST:
            return DOOM_PATH;
        default:
            return nullptr;
    }
};



enum class SEM_TYPE {
    REGISTRATION_QUEUE,
    TEST,
    __COUNT_SENTINEL, // NOLINT(*-reserved-identifier)
};