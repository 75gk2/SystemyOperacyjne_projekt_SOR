#include "constants.hpp"
#include "MessageQueue.hpp"
#include "childProcesses/Registration.hpp"

enum PatientStatus {
    IN_QUEUE_TO_REGISTRATION,
    REGISTERING,
    IN_QUEUE_TO_WAITING_ROOM,
    IN_WAITING_ROOM_WAITING_FOR_TRIAGE,
    IN_TRIAGE,
    IN_WAITING_ROOM_WAITING_FOR_DOCTOR,
    WITH_DOCTOR,
    RELEASED_HOME,
    ADMITTED_TO_HOSPITAL,
    REDIRECTED_TO_SPECIALIST_FACILITY,
};

int main(int argc, char *argv[]) {
    MessageQueue rejestracja = MessageQueue(Registration::REGISTRATION_QUEUE_ID, false);
    // czeka do okienka w rejestracji, nasłuchuje indeks okienka na pipe
    bool isOneElseTwo;
    for (int i = 0; i < 3; i++) {
        rejestracja.receive<bool>(isOneElseTwo, Registration::REGISTRATION_QUEUE_RECEIVE_IS_NEXT_ONE_ELSE_TWO);
        spdlog::info("Patient: received registration window index: {}", isOneElseTwo ? 1 : 2);
    }

    spdlog::info("Patient: finished registration process");
    return 0;

}
