#include "MessageQueue.hpp"
#include "childProcesses/Doctor.hpp"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <random>
#include <thread>

namespace {
    Doctor::Outcome randomOutcome(std::mt19937 &rng) {
        std::uniform_int_distribution<int> dist(0, 999);
        const int roll = dist(rng);
        if (roll < 850) {
            return Doctor::RELEASED_HOME;
        }
        if (roll < 995) {
            return Doctor::ADMITTED_TO_HOSPITAL;
        }
        return Doctor::REDIRECTED_TO_SPECIALIST_FACILITY;
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
}

static volatile sig_atomic_t g_signal1 = 0;
static volatile sig_atomic_t g_signal2 = 0;

static void handleSignal(int signum) {
    if (signum == SIGUSR1) {
        g_signal1 = 1;
    } else if (signum == SIGUSR2) {
        g_signal2 = 1;
    }
}

static void takeBreak(std::mt19937 &rng) {
    std::uniform_int_distribution<int> distMs(1000, 5000);
    const int totalMs = distMs(rng);
    spdlog::warn("Doctor: going to ward for {} ms", totalMs);

    int remaining = totalMs;
    while (remaining > 0 && !g_signal2) {
        const int chunk = std::min(remaining, 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(chunk));
        remaining -= chunk;
    }
    if (g_signal2) {
        spdlog::warn("Doctor: received SIGUSR2 during ward break, shutting down");
        return;
    }
    spdlog::warn("Doctor: returning from ward");
}

int main(int argc, char *argv[]) {
    struct sigaction sa{};
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, nullptr);
    sigaction(SIGUSR2, &sa, nullptr);

    spdlog::info("Doctor: init");
    int delayMs = 0;
    int specialist = 0;
    if (argc > 1) {
        specialist = std::max(0, std::atoi(argv[1]));
    }
    if (argc > 2) {
        delayMs = std::max(0, std::atoi(argv[2]));
    }
    try {
        MessageQueue doctorIn(Doctor::QID_DOCTOR_IN, false);
        MessageQueue doctorsRooms(Doctor::QID_DOCTORS_ROOM, false);

        const auto sendRetryOnEintr = [&](auto &queue, const auto &msg, long mtype) -> int {
            while (true) {
                const int s = queue.send(msg, mtype);
                if (s >= 0) {
                    return s;
                }
                if (g_signal2) {
                    return -1;
                }
                if (errno == EINTR) {
                    // do not leave patient during visit; retry sending
                    continue;
                }
                return -1;
            }
        };

        const auto receiveDuringVisitIgnoreSigusr1 = [&](auto &queue, auto &msg, long mtype) -> int {
            while (true) {
                const int r = queue.receive(msg, mtype, true);
                if (r >= 0) {
                    return r;
                }
                if (g_signal2) {
                    return -1;
                }
                if (errno == EINTR) {
                    // do not leave patient during visit; retry receiving
                    continue;
                }
                return -1;
            }
        };

        std::mt19937 rng(static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        Doctor::Q_DOCTOR_IN_STRUCT patient{};
        const auto spec = static_cast<Triage::Specialist>(specialist);
        const long typeRed = Doctor::priorityToType(spec, Patient::RED);
        const long typeYellow = Doctor::priorityToType(spec, Patient::YELLOW);
        const long typeGreen = Doctor::priorityToType(spec, Patient::GREEN);


        while (true) {
            if (g_signal2) {
                spdlog::warn("Doctor: received SIGUSR2, shutting down");
                break;
            }
            if (g_signal1) {
                g_signal1 = 0;
                takeBreak(rng);
                if (g_signal2) {
                    break;
                }
            }
            //accept patient by priority
            int r = doctorIn.receive(patient, typeRed, false);
            if (r == -2) {
                r = doctorIn.receive(patient, typeYellow, false);
            }
            if (r == -2) {
                r = doctorIn.receive(patient, typeGreen, true);
            }
            if (r < 0) {
                if (g_signal2) {
                    spdlog::warn("Doctor: received SIGUSR2 during receive, shutting down");
                    break;
                }
                if (g_signal1) {
                    g_signal1 = 0;
                    takeBreak(rng);
                    if (g_signal2) {
                        break;
                    }
                }
                if (errno == EINTR) {
                    // Interrupted by a signal; Recheck signals (1|2)
                    continue;
                }
                spdlog::error("Doctor: failed to receive patient");
                continue;
            }

            spdlog::info(
                "Doctor: received patient socialId={}, color={}, specialist={}",
                patient.basic.socialId,
                static_cast<int>(patient.color),
                static_cast<int>(patient.specialist));

            // call patient
            if (sendRetryOnEintr(doctorsRooms, Doctor::Q_DOCTOR_CALLS_IN{}, patient.basic.socialId) < 0) {
                if (g_signal2) {
                    spdlog::warn("Doctor: received SIGUSR2 while calling patient, shutting down");
                    break;
                }
                spdlog::error("Doctor: failed to call patient, socialId={}", patient.basic.socialId);
                continue;
            }

            // patient comes in and represents life data params
            Doctor::Q_DOCTOR_DIAGNOSE diagnose{};
            if (receiveDuringVisitIgnoreSigusr1(doctorsRooms, diagnose, patient.basic.socialId) < 0) {
                if (g_signal2) {
                    spdlog::warn("Doctor: received SIGUSR2 while waiting for diagnosis, shutting down");
                    break;
                }
                spdlog::warn("Doctor: failed to receive diagnosis, socialId={}", patient.basic.socialId);
                continue;
            }

            if (diagnose.left) {
                spdlog::warn("Doctor: patient socialId={} left during diagnosis", patient.basic.socialId);
            } else {
                spdlog::info(
                    "Doctor: diagnosing patient socialId={}, heartRate={}, bloodPressure={}, bodyTemperature={}.",
                    patient.basic.socialId,
                    diagnose.lifeData.heartRate,
                    diagnose.lifeData.bloodPressure,
                    diagnose.lifeData.bodyTemperature);

                // diagnosing...
                if (delayMs > 0) {
                    int remaining = delayMs;
                    while (remaining > 0 && !g_signal2) {
                        const int chunk = std::min(remaining, 200);
                        std::this_thread::sleep_for(std::chrono::milliseconds(chunk));
                        remaining -= chunk;
                    }
                    if (g_signal2) {
                        spdlog::warn("Doctor: received SIGUSR2 during diagnose, shutting down");
                        break;
                    }
                }

                Doctor::Outcome outcome = randomOutcome(rng);
                Doctor::Q_DOCTOR_OUT_STRUCT out{outcome};

                if (sendRetryOnEintr(doctorsRooms, out, patient.basic.socialId) < 0) {
                    if (g_signal2) {
                        spdlog::warn("Doctor: received SIGUSR2 while sending result, shutting down");
                        break;
                    }
                    spdlog::error("Doctor: failed to send result, socialId={}", patient.basic.socialId);
                } else {
                    spdlog::info(
                        "Doctor: finished patient socialId={}, outcome={}",
                        patient.basic.socialId,
                        outcomeToStr(outcome));
                }
            }

            if (g_signal2) {
                spdlog::warn("Doctor: received SIGUSR2 after patient, shutting down");
                break;
            }
            if (g_signal1) {
                g_signal1 = 0;
                takeBreak(rng);
                if (g_signal2) {
                    break;
                }
            }
        }
    } catch (const std::exception &e) {
        spdlog::error("Doctor: exception: {}", e.what());
        return 1;
    }
    return 0;
}
