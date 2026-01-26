#include "MessageQueue.hpp"
#include "childProcesses/Registration.hpp"

int main(int argc, char *argv[]) {
    spdlog::info("Registration: init");
    MessageQueue rejestracja = MessageQueue(Registration::REGISTRATION_QUEUE_ID, false);
    rejestracja.send(true, Registration::REGISTRATION_QUEUE_RECEIVE_IS_NEXT_ONE_ELSE_TWO);
    rejestracja.send(false, Registration::REGISTRATION_QUEUE_RECEIVE_IS_NEXT_ONE_ELSE_TWO);
    rejestracja.send(true, Registration::REGISTRATION_QUEUE_RECEIVE_IS_NEXT_ONE_ELSE_TWO);
    spdlog::info("Registration: finished");
    return 0;
}
