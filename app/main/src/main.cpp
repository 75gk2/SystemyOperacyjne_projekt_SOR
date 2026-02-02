
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <thread>
#include <chrono>

#include "childProcesses/Registration.hpp"
#include "childProcesses/Triage.hpp"
#include "childProcesses/Doctor.hpp"
#include "childProcesses/Patient.hpp"
#include "ProcessManager.hpp"
#include "scenarios/manageSimulations.hpp"
#include "spdlog/spdlog.h"

using namespace std;

namespace {

//modded generic helper - MIT license - https://stackoverflow.com/
int readInt(const string& prompt, int minValue, int maxValue) {
    while (true) {
        cout << prompt;
        int value = 0;
        if (cin >> value && value >= minValue && value <= maxValue) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Nieprawidlowa wartosc. Sprobuj ponownie.\n";
    }
}

bool readBool(const string& prompt) {
    return readInt(prompt + " (0/1): ", 0, 1) == 1;
}

void runDefaultSimulation() {
    simulateTriage();
}

void runCustomSimulation() {
    spdlog::info("MAIN: Starting custom simulation");

    const int registrationQueueSize = readInt("Rozmiar kolejki rejestracji (N): ", 1, 1000);
    const int registrationDelayMs = readInt("Opoznienie rejestracji [ms]: ", 0, 10000);

    //  TODO add doctor delay to APIs
    const int doctorDelayMs = readInt("Opoznienie lekarzy [ms]: ", 0, 10000);

    ProcessManager pm;
    MessageQueue doctorIn(Doctor::QID_DOCTOR_IN, true);
    MessageQueue doctorOut(Doctor::QID_DOCTORS_ROOM, true);

    spdlog::info("MAIN: Assigning Registration process, result={}",
        pm.assignProcess(make_unique<Registration>(registrationQueueSize, registrationDelayMs)));
    spdlog::info("MAIN: Assigning Triage process, result={}",
        pm.assignProcess(make_unique<Triage>()));
    for (int i = 0; i < static_cast<int>(Triage::COUNT); i++) {
        spdlog::info("MAIN: Assigning Doctor process for specialist={}, result={}",
            i,
            pm.assignProcess(make_unique<Doctor>(static_cast<Triage::Specialist>(i), doctorDelayMs)));
    }

    int nextPatientId = 1;
    mt19937 rng(random_device{}());
    uniform_int_distribution<int> distPercent(0, 99);

    while (true) {
        cout << "\n--- Custom simulation ---\n";
        cout << "1) Generuj fale pacjentow\n";
        cout << "0) Exit\n";
        const int choice = readInt("Wybor: ", 0, 1);
        if (choice == 0) {
            break;
        }

        const int waveSize = readInt("Liczba pacjentow w fali: ", 1, 10000);
        const int vipPercent = readInt("Procent VIP (0-100): ", 0, 100);
        const bool allowDiesNow = false;
        const int withChildrenPercent = readInt("Procent rodziców z dziecmi (0-100): ", 0, 100);

        for (int i = 0; i < waveSize; i++) {
            const bool isVip = distPercent(rng) < vipPercent;
            const bool withChildren = distPercent(rng) < withChildrenPercent;
            spdlog::info("MAIN: Assigning Patient process, id={}, result={}",
                nextPatientId,
                pm.assignProcess(make_unique<Patient>(nextPatientId, isVip, allowDiesNow, withChildren)));
            nextPatientId++;
        }
    }

    spdlog::warn("MAIN: Exiting custom simulation");
}

bool shouldRunMenu(int argc, char** argv) {
    if (argc <= 1) {
        return false;
    }
    const string mode = argv[1];
    if (mode == "1") {
        return true;
    }
    return false;
}

}

int main(int argc, char** argv) {
    const bool runMenu = shouldRunMenu(argc, argv);
    if (!runMenu) {
        runDefaultSimulation();
        return 0;
    }

    spdlog::info("\n\n\n==========================\nMAIN: Initializing program\n==========================\n");

    while (true) {
        cout << "\n=== MENU ===\n";
        cout << "1) Symulacja (domyslna)\n";
        cout << "2) Symulacja custom (fale pacjentow)\n";
        cout << "0) Exit\n";
        const int choice = readInt("Wybor: ", 0, 2);
        if (choice == 0) {
            break;
        }
        if (choice == 1) {
            runDefaultSimulation();
        } else if (choice == 2) {
            runCustomSimulation();
        }
    }

    return 0;
}
