#pragma once
#include "Process.hpp"
#include "childProcesses/Patient.hpp"

class Triage : public Process {
public:
    static constexpr char QID_TRIAGE_IN = 'T';
    static constexpr long QTYPE_TRIAGE_IN = 1;
    using Q_TRIAGE_IN_STRUCT = Patient::BasicData;

    static constexpr char QID_TRIAGE_OUT = 'U';

    enum Specialist {
        CARDIOLOGIST,
        NEUROLOGIST,
        OPHTHALMOLOGIST,
        LARYNGOLOGIST,
        SURGEON,
        PEDIATRICIAN,
        COUNT
    };

    struct Q_TRIAGE_OUT_STRUCT {
        Patient::Color color;
        Specialist specialist;
        bool dismissed;
    };

    MessageQueue triageIn;
    MessageQueue triageOut;

    Triage() : Process(ProcessType::TRIAGE), triageIn(QID_TRIAGE_IN, true), triageOut(QID_TRIAGE_OUT, true) {
    }
};
