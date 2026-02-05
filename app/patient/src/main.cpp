#include "constants.hpp"
#include "MessageQueue.hpp"
#include "SemaphoreArray.hpp"
#include "childProcesses/Registration.hpp"
#include "childProcesses/Triage.hpp"
#include "childProcesses/Doctor.hpp"

#include <cstdlib>
#include <cstring>
#include <optional>
#include <random>
#include <csignal>
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

    const char *outcomeToStr(Doctor::Outcome outcome) {
        switch (outcome) {
            case Doctor::RELEASED_HOME:
                return "home";
            case Doctor::ADMITTED_TO_HOSPITAL:
                return "hospital";
            case Doctor::REDIRECTED_TO_SPECIALIST_FACILITY:
                return "redirect";
            default:
                return "unknown";
        }
    }

    Patient::Illness randomIllness(std::mt19937 &rng) {
        std::uniform_int_distribution<int> dist(0, static_cast<int>(Patient::INFECTION));
        return static_cast<Patient::Illness>(dist(rng));
    }
}

static volatile sig_atomic_t g_signal2 = 0;

static void handleSignal(int signum) {
    if (signum == SIGUSR2) {
        g_signal2 = 1;
    }
}


struct ChildState {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    bool inDiagnose = false;
    int socialId;
};


void *child(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);

    auto childState = static_cast<ChildState *>(arg);

    spdlog::info("Patient(child): initialized, socialId={}", childState->socialId);

    pthread_mutex_lock(&childState->lock);
    while (!childState->inDiagnose) {
        spdlog::info("Patient(child): waiting for diagnose call, socialId={}", childState->socialId);
        pthread_cond_wait(&childState->cond, &childState->lock);
        pthread_testcancel();
    }
    pthread_mutex_unlock(&childState->lock);

    spdlog::info("Patient(child): called in, preparing diagnosis, socialId={}", childState->socialId);

    Doctor::Q_DOCTOR_DIAGNOSE diagnose{};
    std::uniform_int_distribution<int> heartDist(60, 120);
    std::uniform_int_distribution<int> bpDist(110, 139);
    std::uniform_real_distribution<float> tempDist(36.5f, 42.0f);

    std::random_device rd;
    std::mt19937 rng(rd());
    diagnose.lifeData = Patient::LifeData{
        heartDist(rng),
        bpDist(rng),
        tempDist(rng),
    };

    diagnose.left = false;
    MessageQueue doctorsRoom(Doctor::QID_DOCTORS_ROOM, false);
    const long diagType = Doctor::diagnoseType(childState->socialId);
    doctorsRoom.send(diagnose, diagType);
    spdlog::debug(
        "Patient(child): sent diagnosis to doctor, socialId={}, mtype={}",
        childState->socialId,
        diagType);
    return nullptr;
}


int main(int argc, char *argv[]) {
    struct sigaction sa{};
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, nullptr);

    std::random_device rd;
    std::mt19937 rng(rd());
    MessageQueue registrationQueue(Registration::Q_REGISTRATION_ID, false);
    MessageQueue windowIn(Registration::QID_WINDOW_1_IN, false);
    MessageQueue windowOut(Registration::QID_WINDOW_1_OUT, false);
    MessageQueue registrationCtrl(Registration::QID_REGISTRATION_CTRL, false);
    MessageQueue triageIn(Triage::QID_TRIAGE_IN, false);
    MessageQueue triageOut(Triage::QID_TRIAGE_OUT, false);
    MessageQueue doctorsIn(Doctor::QID_DOCTOR_IN, false);
    MessageQueue doctorsRoom(Doctor::QID_DOCTORS_ROOM, false);
    MessageQueue doctorsVerdict(Doctor::QID_DOCTORS_VERDICT, false);
    SemaphoreArray semaphores(false);

    Patient::BasicData data{};
    std::optional<ChildState> childState;
    std::optional<pthread_t> childThread;

    const auto cleanupChildThread = [&]() {
        if (childState.has_value() && childThread.has_value()) {
            pthread_mutex_lock(&childState->lock);
            childState->inDiagnose = true;
            pthread_cond_signal(&childState->cond);
            pthread_mutex_unlock(&childState->lock);
            if (g_signal2) {
                pthread_cancel(*childThread);
                pthread_join(*childThread, nullptr);
                return;
            }
            pthread_join(*childThread, nullptr);
        }
    };

    const auto checkEvacuation = [&]() -> bool {
        if (g_signal2) {
            spdlog::warn("Patient: received SIGUSR2, shutting down");
            cleanupChildThread();
            return true;
        }
        return false;
    };

        // auto sid_dist = std::uniform_int_distribution<int>(0, 1000000);
        data.socialId = getpid();//+sid_dist(rng);
        if (argc > 2) {
            data.isVIP = std::atoi(argv[2]) != 0;
        } else {
            data.isVIP = (data.socialId % 10 == 0);
        }
        std::strncpy(data.name, "Nazywam sie: <imie> <nazwisko>", sizeof(data.name) - 1);
        std::strncpy(data.address, "Jakiś adres", sizeof(data.address) - 1);
        data.phone = static_cast<short>(100 + (getpid() % 10000));
        data.ill = randomIllness(rng);
        if (argc > 3) {
            data.diesNow = std::atoi(argv[3]) != 0;
        } else {
            data.diesNow = (data.socialId % 20 == 0);
        }

        if (argc > 4) {
            data.isThisParentWithChildren = std::atoi(argv[4]) != 0;
        } else {
            data.isThisParentWithChildren = false;
        }
        const int charisNumber = data.isThisParentWithChildren ? 2 : 1;

        if (data.isThisParentWithChildren) {
            childState.emplace();
            childState->socialId = data.socialId;
            pthread_create(&childThread.emplace(), nullptr, child, childState.operator->());
        }

        if (!data.isVIP) {
            if (checkEvacuation()) {
                return 0;
            }
            // Join registration queue - increase counter
            if (!semaphores.pullUp(SEM_TYPE::REGISTRATION_QUEUE, 1)) {
                if (checkEvacuation()) {
                    return 0;
                }
                spdlog::error("Patient: failed to join registration queue");
                cleanupChildThread();
                return 1;
            }

            // Notify registration about joining queue
            Registration::Q_REGISTRATION_CTRL_STRUCT joinEvent{+1};
            if (registrationCtrl.send(joinEvent, Registration::QTYPE_REGISTRATION_CTRL) < 0) {
                spdlog::error("Patient: failed to notify registration about queue joining");
                cleanupChildThread();
                return 1;
            }

            // Wait in queue to see first free window
            Registration::Q_REGISTRATION_STRUCT registration{};
            if (checkEvacuation()) {
                return 0;
            }
            if (registrationQueue.receive(registration, Registration::Q_REGISTRATION_RECEIVE_MID, true) < 0) {
                if (checkEvacuation()) {
                    return 0;
                }
                spdlog::error("Patient: failed to receive registration window token");
                cleanupChildThread();
                return 1;
            }

            //Leave queue - decrease counter, but dont wait - flow doesn't allow negative counter, every client takes back own increment
            if (!semaphores.pullDown(SEM_TYPE::REGISTRATION_QUEUE, 1)) {
                if (checkEvacuation()) {
                    return 0;
                }
                spdlog::error("Patient: failed to leave registration queue");
                cleanupChildThread();
                return 1;
            }

            // Notify registration about leaving queue
            Registration::Q_REGISTRATION_CTRL_STRUCT leaveEvent{-1};
            if (registrationCtrl.send(leaveEvent, Registration::QTYPE_REGISTRATION_CTRL) < 0) {
                spdlog::error("Patient: failed to notify registration about queue leaveing");
                cleanupChildThread();
                return 1;
            }

            // Send data to my window
            spdlog::info("Patient: assigned to registration window {}", registration.isFreeOneElseTwo ? 1 : 2);
            if (windowIn.send(data, Registration::QTYPE_WINDOW_IN) < 0) {
                if (checkEvacuation()) {
                    return 0;
                }
                spdlog::error("Patient: failed to send registration data");
                cleanupChildThread();
                return 1;
            }
        } else {
            // Send data to my window, but with vip mark
            spdlog::info("Patient: VIP, skipping queue");
            if (windowIn.send(data, Registration::QTYPE_WINDOW_IN_VIP) < 0) {
                if (checkEvacuation()) {
                    return 0;
                }
                spdlog::error("Patient: failed to send VIP registration data");
                cleanupChildThread();
                return 1;
            }
        }


        // Response get
        Registration::Q_WINDOW_OUT_STRUCT response{};
        if (checkEvacuation()) {
            return 0;
        }
        if (windowOut.receive(response, data.socialId, true) < 0) {
            if (checkEvacuation()) {
                return 0;
            }
            spdlog::error("Patient: failed to receive registration response");
            cleanupChildThread();
            return 1;
        }

        bool canHurry = response.youCanHurry;
        spdlog::info("Patient: finished registration, going to waithing room, canHurry={}", canHurry);

        // join waiting room (occupy seat)
        if (checkEvacuation()) {
            return 0;
        }
        if (!semaphores.pullDown(SEM_TYPE::WAITING_ROOM_QUEUE, charisNumber)) {
            if (checkEvacuation()) {
                return 0;
            }
            spdlog::error("Patient: failed to enter waiting room");
            cleanupChildThread();
            return 1;
        }

        spdlog::info("Patient: entered waiting room", canHurry);


        // send data to triage
        spdlog::info("Patient: entered waiting room, waiting for triage");
        if (checkEvacuation()) {
            return 0;
        }
        if (triageIn.send(data, Triage::QTYPE_TRIAGE_IN) < 0) {
            if (checkEvacuation()) {
                return 0;
            }
            spdlog::error("Patient: failed to send data to triage");
            cleanupChildThread();
            return 1;
        }

        spdlog::info("Patient: queued to triage, and waiting for response");

        //triage response
        Triage::Q_TRIAGE_OUT_STRUCT triageResponse{};
        if (checkEvacuation()) {
            return 0;
        }
        if (triageOut.receive(triageResponse, data.socialId, true) < 0) {
            if (checkEvacuation()) {
                return 0;
            }
            spdlog::error("Patient: failed to receive triage response");
            cleanupChildThread();
            return 1;
        }

        //response info
        spdlog::info(
            "Patient: triage result color={}, dismissed={}, specialist={}",
            colorToStr(triageResponse.color),
            triageResponse.dismissed,
            specialistToStr(triageResponse.specialist)
        );


        if (triageResponse.dismissed || triageResponse.color == Patient::DISMISSED) {
            spdlog::info("Patient: dismissed after triage, leaving");
            if (checkEvacuation()) {
                return 0;
            }
            if (!semaphores.pullUp(SEM_TYPE::WAITING_ROOM_QUEUE, charisNumber)) {
                if (checkEvacuation()) {
                    return 0;
                }
                spdlog::error("Patient: failed to leave waiting room");
                cleanupChildThread();
                return 1;
            }
            cleanupChildThread();
            return 0;
        }

        Doctor::Q_DOCTOR_IN_STRUCT doctorRequest{data, triageResponse.color, triageResponse.specialist};
        long priority = Doctor::priorityToType(triageResponse.specialist, triageResponse.color);

        spdlog::info("Patient: waiting for doctor, priority={}, specialist={}", priority,
                     specialistToStr(triageResponse.specialist));

        if (doctorsIn.send(doctorRequest, priority) < 0) {
            if (checkEvacuation()) {
                return 0;
            }
            spdlog::error("Patient: failed to enqueue for doctor");
            cleanupChildThread();
            return 1;
        }

        auto getCalledIn = Doctor::Q_DOCTOR_CALLS_IN{};
        if (checkEvacuation()) {
            return 0;
        }
        const long callType = Doctor::callInType(data.socialId);
        if (doctorsRoom.receive(getCalledIn, callType, true) < 0) {
            if (checkEvacuation()) {
                return 0;
            }
            spdlog::error("Patient: failed to receive doctor call");
            cleanupChildThread();
            return 1;
        }
        spdlog::info("Patient: called in by doctor, going to diagnosis");

        if (data.isThisParentWithChildren) {
            if (!childState.has_value() || !childThread.has_value()) {
                spdlog::error("Patient: child state not set");
                Doctor::Q_DOCTOR_DIAGNOSE diagnose{};
                diagnose.left = true;
                doctorsRoom.send(diagnose, Doctor::diagnoseType(data.socialId));
                cleanupChildThread();
                return 1;
            }
            pthread_mutex_lock(&childState->lock);
            childState->inDiagnose = true;
            pthread_cond_signal(&childState->cond);
            pthread_mutex_unlock(&childState->lock);
        } else {
            Doctor::Q_DOCTOR_DIAGNOSE diagnose{};
            diagnose.left = false;

            std::uniform_int_distribution<int> heartDist(60, 99);
            std::uniform_int_distribution<int> bpDist(110, 139);
            std::uniform_real_distribution<float> tempDist(36.5f, 42.0f);

            diagnose.lifeData = Patient::LifeData{
                heartDist(rng),
                bpDist(rng),
                tempDist(rng),
            };
            const long diagType = Doctor::diagnoseType(data.socialId);
            doctorsRoom.send(diagnose, diagType);
            spdlog::debug(
                "Patient: sent diagnosis to doctor, socialId={}, mtype={} (heartRate={}, bloodPressure={}, bodyTemperature={})",
                data.socialId,
                diagType,
                diagnose.lifeData.heartRate,
                diagnose.lifeData.bloodPressure,
                diagnose.lifeData.bodyTemperature);
        }


        Doctor::Q_DOCTOR_OUT_STRUCT doctorResponse{};
        if (checkEvacuation()) {
            return 0;
        }
        if (doctorsVerdict.receive(doctorResponse, data.socialId, true) < 0) {
            if (checkEvacuation()) {
                return 0;
            }
            spdlog::error("Patient: failed to receive doctor response");
            cleanupChildThread();
            return 1;
        }
        //todo add second stage of doctor communication
        if (checkEvacuation()) {
            return 0;
        }
        if (!semaphores.pullUp(SEM_TYPE::WAITING_ROOM_QUEUE, charisNumber)) {
            if (checkEvacuation()) {
                return 0;
            }
            spdlog::error("Patient: failed to leave waiting room");
            cleanupChildThread();
            return 1;
        }
        spdlog::info("Patient: doctor outcome={}", outcomeToStr(doctorResponse.outcome));

        cleanupChildThread();

        return 0;
    }
