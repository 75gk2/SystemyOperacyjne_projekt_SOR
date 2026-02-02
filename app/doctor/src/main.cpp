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
        MessageQueue doctorsRooms(Doctor::QID_DOCTORS_ROOM, false);

        std::mt19937 rng(static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        Doctor::Q_DOCTOR_IN_STRUCT patient{};
        const auto spec = static_cast<Triage::Specialist>(specialist);
        const long typeRed = Doctor::priorityToType(spec, Patient::RED);
        const long typeYellow = Doctor::priorityToType(spec, Patient::YELLOW);
        const long typeGreen = Doctor::priorityToType(spec, Patient::GREEN);


        while (true) {
            //accept patient by priority
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

            //call patient

            doctorsRooms.send(Doctor::Q_DOCTOR_CALLS_IN{}, patient.basic.socialId);


            //patient comes in and represents life data params
            Doctor::Q_DOCTOR_DIAGNOSE diagnose{};
            doctorsRooms.receive(diagnose, patient.basic.socialId, true);

            if (diagnose.left) {
                spdlog::warn("Doctor: patient socialId={} left during diagnosis", patient.basic.socialId);
                continue;
            }
            spdlog::info(
                "Doctor: diagnosing patient socialId={}, heartRate={}, bloodPressure={}, bodyTemperature={}",
                patient.basic.socialId,
                diagnose.lifeData.heartRate,
                diagnose.lifeData.bloodPressure,
                diagnose.lifeData.bodyTemperature);

            //diagbosing...
            if (delayMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }

            Doctor::Outcome outcome = randomOutcome(rng);
            Doctor::Q_DOCTOR_OUT_STRUCT out{outcome};

            if (doctorsRooms.send(out, patient.basic.socialId) < 0) {
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
