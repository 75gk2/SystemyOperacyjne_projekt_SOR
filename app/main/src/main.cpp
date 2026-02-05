
#include <iostream>

#include "scenarios/manageSimulations.hpp"
#include "spdlog/spdlog.h"

using namespace std;


void defaultSimulation() { simulateTriage(); }

namespace {
    bool shouldRunMenu(int argc, char **argv) {
        if (argc <= 1) {
            return false;
        }
        const std::string mode = argv[1];
        if (mode == "1") {
            return true;
        }
        return false;
    }
}
int main(int argc, char **argv) {
    const bool runMenu = shouldRunMenu(argc, argv);
    if (!runMenu) {
        defaultSimulation();
        return 0;
    }

    spdlog::info("\n\n\n==========================\nMAIN: Initializing program\n==========================\n");

    while (true) {
        cout << "\n=== MENU ===\n";
        cout << "1) Symulacja własna\n";
        cout << "2) Symulacja: Triage\n";
        cout << "3) Symulacja: Okienka rejestracji\n";
        cout << "4) Symulacja bez doktorów\n";
        cout << "0) Exit\n";

        switch (readInt("Scenariusz: ", 0, 4)) {
            case 0:
                return 0;
            case 1:
                runCustomSimulation();
                break;
            case 2:
                simulateTriage();
                break;
            case 3:

                simulateManageRegistrationWindows();
                break;
            case 4:
                simulateNoDoctors();
                break;
            default:
                cout << "Nieprawidlowy wybor. Sprobuj ponownie.\n";
                break;
        }
    }
}
