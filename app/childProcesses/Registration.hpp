#ifndef SOR_SO_REGISTRATION_HPP
#define SOR_SO_REGISTRATION_HPP
#include "MessageQueue.hpp"
#include "Process.hpp"
#include "childProcesses/Patient.hpp"

class Registration: public Process{
public:
    static constexpr char Q_REGISTRATION_ID = 'R';
    static constexpr long Q_REGISTRATION_RECEIVE_MID = 1;
    struct Q_REGISTRATION_STRUCT {
        bool isFreeOneElseTwo;
    };

    static constexpr char QID_WINDOW_1_IN = 'r';
    static constexpr long QTYPE_WINDOW_IN_VIP = 1;
    static constexpr long QTYPE_WINDOW_IN = 2;
    using Q_WINDOW_STRUCT = Patient::BasicData;

    static constexpr char QID_WINDOW_1_OUT = 's';
    struct Q_WINDOW_OUT_STRUCT {
        bool youCanHurry;
    };

    static constexpr char QID_REGISTRATION_CTRL = 'p';
    static constexpr long QTYPE_REGISTRATION_CTRL = 1;
    struct Q_REGISTRATION_CTRL_STRUCT {
        int delta;
    };

    MessageQueue registrationQueue;
    MessageQueue registrationCtrlQueue;
    MessageQueue windowIn;
    MessageQueue windowOut;

    explicit Registration(int n, int delayMs = 0);
};


#endif //SOR_SO_REGISTRATION_HPP