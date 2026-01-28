#include "Registration.hpp"

#include <string>

Registration::Registration(int n, int delayMs)
  : Process(ProcessType::REGISTRATION, {std::to_string(n), std::to_string(delayMs)}),
      registrationQueue(Q_REGISTRATION_ID, true),
      registrationCtrlQueue(QID_REGISTRATION_CTRL, true),
      windowIn(QID_WINDOW_1_IN, true),
      windowOut(QID_WINDOW_1_OUT, true) {
}
