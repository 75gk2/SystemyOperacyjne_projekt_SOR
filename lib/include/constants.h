#pragma once

#define POCZEKALNIA_SIZE 20
#define K1 (POCZEKALNIA_SIZE / 2)
#define K2 (POCZEKALNIA_SIZE / 3)
#define LEKARZE_COUNT 3

// IPC Keys
#define SHM_PROJ_ID 'M'
#define SEM_PROJ_ID 'S'
#define MSG_PROJ_ID 'Q'

enum class ProcessType {
    LEKARZ,
    REJESTRACJA,
    PACJENT,
    POCZEKALNIA,
    TEST
    // __COUNT_SENTINEL
};

inline const char* getProcessExecPath(const ProcessType type) {
    switch (type) {
        case ProcessType::LEKARZ:
            return "/lekarz";
        case ProcessType::REJESTRACJA:
            return "/rejestracja";
        case ProcessType::PACJENT:
            return "/pacjent";
        case ProcessType::POCZEKALNIA:
            return "/poczekalnia";
        case ProcessType::TEST:
            return DOOM_PATH;
        default:
            return nullptr;
    }
};