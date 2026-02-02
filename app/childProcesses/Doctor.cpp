#include "Doctor.hpp"

#include <string>

Doctor::Doctor(Triage::Specialist specialist, int delayMs)
    : Process(ProcessType::DOCTOR, {std::to_string(static_cast<int>(specialist)), std::to_string(delayMs)}),
      doctorIn(QID_DOCTOR_IN, false),
      doctorsRooms(QID_DOCTORS_ROOM, false) {
}
