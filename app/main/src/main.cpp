
#include <iostream>
#include <exception>

#include "scenarios/manageSimulations.hpp"
#include "spdlog/spdlog.h"

using namespace std;


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
    try {
        const bool runMenu = shouldRunMenu(argc, argv);
        if (!runMenu) {
            simulateDefault();
            return 0;
        }

        spdlog::info("\n\n\n==========================\nMAIN: Initializing program\n==========================\n");

        while (true) {
            cout << "\n=== MENU ===\n";
            cout << "1) Symulacja własna\n";
            cout << "2) Symulacja: Test sekwencji - wymaga modyfikacji\n";
            cout << "3) Symulacja: Okienka rejestracji\n";
            cout << "4) Symulacja początkowo bez doktorów\n";
            cout << "5) Symulacja 50k (instant)\n";
            cout << "6) Symulacja domyślna\n";
            cout << "0) Exit\n";

            switch (readInt("Scenariusz: ", 0, 6)) {
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
                case 5:
                    simulateFullLargeFlowNoDelay();
                    break;
                case 6:
                    simulateDefault();
                    break;
                default:
                    cout << "Nieprawidlowy wybor. Sprobuj ponownie.\n";
                    break;
            }
        }
    } catch (const std::exception &e) {
        spdlog::critical("MAIN: Unhandled exception: {}", e.what());
        return 1;
    } catch (...) {
        spdlog::critical("MAIN: Unhandled unknown exception");
        return 1;
    }
}
