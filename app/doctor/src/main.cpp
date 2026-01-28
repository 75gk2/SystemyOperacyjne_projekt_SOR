#include "MessageQueue.hpp"
#include "childProcesses/Doctor.hpp"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
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

int main(int argc, char *argv[]) {
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
        MessageQueue doctorOut(Doctor::QID_DOCTOR_OUT, false);

        std::mt19937 rng(static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()));

        while (true) {
            Doctor::Q_DOCTOR_IN_STRUCT patient{};
            const auto spec = static_cast<Triage::Specialist>(specialist);
            const long typeRed = Doctor::priorityToType(spec, Patient::RED);
            const long typeYellow = Doctor::priorityToType(spec, Patient::YELLOW);
            const long typeGreen = Doctor::priorityToType(spec, Patient::GREEN);

            int r = doctorIn.receive(patient, typeRed, false);
            if (r == -2) {
                r = doctorIn.receive(patient, typeYellow, false);
            }
            if (r == -2) {
                r = doctorIn.receive(patient, typeGreen, true);
            }
            if (r < 0) {
                spdlog::error("Doctor: failed to receive patient");
                continue;
            }

            spdlog::info(
                "Doctor: received patient socialId={}, color={}, specialist={}",
                patient.basic.socialId,
                static_cast<int>(patient.color),
                static_cast<int>(patient.specialist));

            if (delayMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }

            Doctor::Outcome outcome = randomOutcome(rng);
            Doctor::Q_DOCTOR_OUT_STRUCT out{outcome};

            if (doctorOut.send(out, patient.basic.socialId) < 0) {
                spdlog::error("Doctor: failed to send result, socialId={}", patient.basic.socialId);
            } else {
                spdlog::info(
                    "Doctor: finished patient socialId={}, outcome={}",
                    patient.basic.socialId,
                    outcomeToStr(outcome));
            }
        }
    } catch (const std::exception &e) {
        spdlog::error("Doctor: exception: {}", e.what());
        return 1;
    }
    return 0;
}
