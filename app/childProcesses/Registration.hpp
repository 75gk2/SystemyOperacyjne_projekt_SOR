#ifndef SOR_SO_REGISTRATION_HPP
#define SOR_SO_REGISTRATION_HPP
#include "MessageQueue.hpp"
#include "Process.hpp"


class Registration: public Process{
public:
    static constexpr char REGISTRATION_QUEUE_ID = 'R';
    static constexpr char REGISTRATION_QUEUE_RECEIVE_IS_NEXT_ONE_ELSE_TWO = 1;

    // static constexpr char WINDOW_1_IN = 'r';
    // static constexpr char WINDOW_1_OUT = 's';
    MessageQueue registrationQueue;
    Registration();
};


#endif //SOR_SO_REGISTRATION_HPP