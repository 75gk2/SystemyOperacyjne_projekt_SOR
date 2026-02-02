#include "MessageQueue.hpp"
#include "SemaphoreArray.hpp"
#include "childProcesses/Triage.hpp"
#include "constants.hpp"

#include "spdlog/spdlog.h"

#include <chrono>
#include <random>

namespace {
    Patient::Color pickColor(const Patient::BasicData &data, std::mt19937 &rng) {
        if (data.diesNow) {
            return Patient::RED;
        }
        std::uniform_int_distribution<int> dist(0, 99);
        const int roll = dist(rng);
        if (roll < 10) {
            return Patient::RED;
        }
        if (roll < 45) {
            return Patient::YELLOW;
        }
        if (roll < 95) {
            return Patient::GREEN;
        }
        return Patient::DISMISSED;
    }

    Triage::Specialist pickSpecialist(Patient::Illness ill) {
        switch (ill) {
            case Patient::HEART_HURTS:
                return Triage::CARDIOLOGIST;
            case Patient::HEAD_HURTS:
                return Triage::NEUROLOGIST;
            case Patient::EYE_HURTS:
                return Triage::OPHTHALMOLOGIST;
            case Patient::EAR_HURTS:
                return Triage::LARYNGOLOGIST;
            case Patient::BROKEN_BONE:
                return Triage::SURGEON;
            case Patient::INFECTION:
                return Triage::PEDIATRICIAN;
            default:
                return Triage::PEDIATRICIAN;
        }
    }
}

int main(int argc, char *argv[]) {
    spdlog::info("Triage: init");
    try {
        MessageQueue triageIn(Triage::QID_TRIAGE_IN, false);
        MessageQueue triageOut(Triage::QID_TRIAGE_OUT, false);
        SemaphoreArray semaphores(false);

        if (!semaphores.setValue(SEM_TYPE::WAITING_ROOM_QUEUE, POCZEKALNIA_SIZE)) {
            spdlog::error("Triage: failed to initialize WAITING_ROOM_QUEUE semaphore");
            return 1;
        }

        spdlog::info("Triage: initialized semaphore");

        std::mt19937 rng(static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()));

        while (true) {
            spdlog::info("Triage: waiting for next data");
            Triage::Q_TRIAGE_IN_STRUCT patient{};
            if (triageIn.receive(patient, Triage::QTYPE_TRIAGE_IN, true) < 0) {
                spdlog::error("Triage: failed to receive patient data");
                continue;
            }

            spdlog::info("Triage: received patient data, preparing response");

            const Patient::Color color = pickColor(patient, rng);
            const bool dismissed = (color == Patient::DISMISSED);
            const Triage::Specialist specialist = pickSpecialist(patient.ill);

            Triage::Q_TRIAGE_OUT_STRUCT out{color, specialist, dismissed};
            if (triageOut.send(out, patient.socialId) < 0) {
                spdlog::error("Triage: failed to send triage result, socialId={}", patient.socialId);
            }

            spdlog::info("Triage: sending response with color={} where={}", color, specialist);
        }
    } catch (const std::exception &e) {
        spdlog::error("Triage: exception: {}", e.what());
        return 1;
    }
    return 0;
}