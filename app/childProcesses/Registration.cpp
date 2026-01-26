#include "Registration.hpp"

Registration::Registration()
    : Process(ProcessType::REGISTRATION),
      registrationQueue(REGISTRATION_QUEUE_ID, true) {
}
