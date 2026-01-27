#pragma once
#include "Process.hpp"

class Patient : public Process {
public:

    enum Illness {
        HEART_HURTS,
        HEAD_HURTS,
        EYE_HURTS,
        EAR_HURTS,
        BROKEN_BONE,
        INFECTION,
    };

    enum Color {
        RED,
        YELLOW,
        GREEN,
        DISMISSED,
        COUNT,
    };

    struct BasicData {
        int socialId;
        bool isVIP ;
        char name[32];
        char address[64];
        short phone;
        Illness ill;
        bool diesNow;
    };

    Patient(int index, bool isVip=false, bool allowDiesNow=true);
};