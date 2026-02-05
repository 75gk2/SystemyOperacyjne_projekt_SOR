#pragma once
#include "Process.hpp"
#include "Patient.hpp"
#include "Triage.hpp"
#include "MessageQueue.hpp"

class Doctor : public Process {
public:
    static constexpr char QID_DOCTOR_IN = 'D';
    static constexpr char QID_DOCTORS_ROOM = 'E';
    static constexpr char QID_DOCTORS_VERDICT = 'F';

    static constexpr long QTYPE_DOCTOR_RED = 1;
    static constexpr long QTYPE_DOCTOR_YELLOW = 2;
    static constexpr long QTYPE_DOCTOR_GREEN = 3;
    static constexpr long QTYPE_DOCTOR_PER_SPECIALIST = 10;

    enum Outcome {
        RELEASED_HOME,
        ADMITTED_TO_HOSPITAL,
        REDIRECTED_TO_SPECIALIST_FACILITY,
    };

    struct Q_DOCTOR_IN_STRUCT {
        Patient::BasicData basic;
        Patient::Color color;
        Triage::Specialist specialist;
    };

    struct Q_DOCTOR_CALLS_IN {
    };

    struct Q_DOCTOR_DIAGNOSE {
        Patient::LifeData lifeData;
        bool left;
    };

    struct Q_DOCTOR_OUT_STRUCT {
        Outcome outcome;
    };

    static constexpr long QTYPE_DIAGNOSE_OFFSET = 1000000000L;

    static long callInType(int socialId) {
        return static_cast<long>(socialId);
    }

    static long diagnoseType(int socialId) {
        return QTYPE_DIAGNOSE_OFFSET + static_cast<long>(socialId);
    }

    MessageQueue doctorIn;
    MessageQueue doctorsRooms;
    MessageQueue doctorVerdict;

    explicit Doctor(Triage::Specialist specialist, int delayMs = 0);

    static long priorityToType(Triage::Specialist specialist, Patient::Color color) {
        const long base = static_cast<long>(specialist) * QTYPE_DOCTOR_PER_SPECIALIST;
        switch (color) {
            case Patient::RED:
                return base + QTYPE_DOCTOR_RED;
            case Patient::YELLOW:
                return base + QTYPE_DOCTOR_YELLOW;
            case Patient::GREEN:
                return base + QTYPE_DOCTOR_GREEN;
            default:
                return base + QTYPE_DOCTOR_GREEN;
        }
    }
};