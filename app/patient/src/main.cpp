#include "constants.hpp"
#include "MessageQueue.hpp"
#include "SemaphoreArray.hpp"
#include "childProcesses/Registration.hpp"
#include "childProcesses/Triage.hpp"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <random>
#include <thread>
#include <unistd.h>

#include "spdlog/spdlog.h"

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

namespace {
    const char *colorToStr(Patient::Color color) {
        switch (color) {
            case Patient::RED:
                return "RED";
            case Patient::YELLOW:
                return "YELLOW";
            case Patient::GREEN:
                return "GREEN";
            case Patient::DISMISSED:
                return "DISMISSED";
            default:
                return "UNKNOWN";
        }
    }

    const char *specialistToStr(Triage::Specialist specialist) {
        switch (specialist) {
            case Triage::CARDIOLOGIST:
                return "kardiolog";
            case Triage::NEUROLOGIST:
                return "neurolog";
            case Triage::OPHTHALMOLOGIST:
                return "okulista";
            case Triage::LARYNGOLOGIST:
                return "laryngolog";
            case Triage::SURGEON:
                return "chirurg";
            case Triage::PEDIATRICIAN:
                return "pediatra";
            default:
                return "unknown";
        }
    }

    Patient::Illness randomIllness() {
        std::mt19937 rng(static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(0, static_cast<int>(Patient::INFECTION));
        return static_cast<Patient::Illness>(dist(rng));
    }
}

int main(int argc, char *argv[]) {
    MessageQueue registrationQueue(Registration::Q_REGISTRATION_ID, false);
    MessageQueue windowIn(Registration::QID_WINDOW_1_IN, false);
    MessageQueue windowOut(Registration::QID_WINDOW_1_OUT, false);
    MessageQueue registrationCtrl(Registration::QID_REGISTRATION_CTRL, false);
    MessageQueue triageIn(Triage::QID_TRIAGE_IN, false);
    MessageQueue triageOut(Triage::QID_TRIAGE_OUT, false);
    SemaphoreArray semaphores(false);

    Patient::BasicData data{};
    data.socialId = getpid();
    if (argc > 2) {
        data.isVIP = std::atoi(argv[2]) != 0;
    } else {
        data.isVIP = (data.socialId % 10 == 0);
    }
    std::strncpy(data.name, "Nazywam sie: <imie> <nazwisko>", sizeof(data.name) - 1);
    std::strncpy(data.address, "Jakiś adres", sizeof(data.address) - 1);
    data.phone = static_cast<short>(100 + (getpid() % 10000));
    data.ill = randomIllness();
    if (argc > 3) {
        data.diesNow = std::atoi(argv[3]) != 0;
    } else {
        data.diesNow = (data.socialId % 20 == 0);
    }

    // Join registration queue - increase counter
    if (!semaphores.pullUp(SEM_TYPE::REGISTRATION_QUEUE, 1)) {
        spdlog::error("Patient: failed to join registration queue");
        return 1;
    }

    // Notify registration about joining queue
    Registration::Q_REGISTRATION_CTRL_STRUCT joinEvent{+1};
    if (registrationCtrl.send(joinEvent, Registration::QTYPE_REGISTRATION_CTRL) < 0) {
        spdlog::error("Patient: failed to notify registration about queue joining");
        return 1;
    }

    // Wait in queue to see first free window
    Registration::Q_REGISTRATION_STRUCT registration{};
    if (registrationQueue.receive(registration, Registration::Q_REGISTRATION_RECEIVE_MID, true) < 0) {
        spdlog::error("Patient: failed to receive registration window token");
        return 1;
    }

    //Leave queue - decrease counter, but dont wait - flow doesn't allow negative counter, every client takes back own increment
    if (!semaphores.pullDown(SEM_TYPE::REGISTRATION_QUEUE, 1)) {
        spdlog::error("Patient: failed to leave registration queue");
        return 1;
    }

    // Notify registration about leaving queue
    Registration::Q_REGISTRATION_CTRL_STRUCT leaveEvent{-1};
    if (registrationCtrl.send(leaveEvent, Registration::QTYPE_REGISTRATION_CTRL) < 0) {
        spdlog::error("Patient: failed to notify registration about queue leaveing");
        return 1;
    }

    if (!data.isVIP) {
        // Send data to my window
        spdlog::info("Patient: assigned to registration window {}", registration.isFreeOneElseTwo ? 1 : 2);
        if (windowIn.send(data, Registration::QTYPE_WINDOW_IN) < 0) {
            spdlog::error("Patient: failed to send registration data");
            return 1;
        }
    } else {
        // Send data to my window, but with vip mark
        spdlog::info("Patient: VIP, skipping queue");
        if (windowIn.send(data, Registration::QTYPE_WINDOW_IN_VIP) < 0) {
            spdlog::error("Patient: failed to send VIP registration data");
            return 1;
        }
    }

    // Response get
    Registration::Q_WINDOW_OUT_STRUCT response{};
    if (windowOut.receive(response, data.socialId, true) < 0) {
        spdlog::error("Patient: failed to receive registration response");
        return 1;
    }

    bool canHurry = response.youCanHurry;
    spdlog::info("Patient: finished registration process, canHurry={}", canHurry);

    // join waiting room (occupy seat)
    if (!semaphores.pullDown(SEM_TYPE::WAITING_ROOM_QUEUE, 1)) {
        spdlog::error("Patient: failed to enter waiting room");
        return 1;
    }

    spdlog::info("Patient: entered waiting room, waiting for triage");
    if (triageIn.send(data, Triage::QTYPE_TRIAGE_IN) < 0) {
        spdlog::error("Patient: failed to send data to triage");
        return 1;
    }

    Triage::Q_TRIAGE_OUT_STRUCT triageResponse{};
    if (triageOut.receive(triageResponse, data.socialId, true) < 0) {
        spdlog::error("Patient: failed to receive triage response");
        return 1;
    }

    spdlog::info(
        "Patient: triage result color={}, dismissed={}, specialist={}",
        colorToStr(triageResponse.color),
        triageResponse.dismissed,
        specialistToStr(triageResponse.specialist)
    );

    // patient leaves waiting room when directed to doctor or dismissed
    if (!semaphores.pullUp(SEM_TYPE::WAITING_ROOM_QUEUE, 1)) {
        spdlog::error("Patient: failed to leave waiting room");
        return 1;
    }


    return 0;
}
