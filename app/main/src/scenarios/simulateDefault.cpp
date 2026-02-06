#include <unistd.h>

#include "manageSimulations.hpp"
#include "ProcessManager.hpp"
#include "childProcesses/Patient.hpp"
#include "childProcesses/Registration.hpp"
#include "childProcesses/Triage.hpp"
#include "childProcesses/Doctor.hpp"
#include "spdlog/spdlog.h"

void simulateDefault() {
    spdlog::info("MAIN: Starting defult simulation");

    // Główny process manager - do niego należy przypisać wszytkie procesy
    ProcessManager pm;

    //argumanty do klas pochodnych ustandaryzowane tak jak w klasach i dokumentacji

    //Tworzenie Rejestracji
    spdlog::info("MAIN: Assigning Registration process, result={}",
                 pm.assignProcess(std::make_unique<Registration>(1000,50))); // N = 1000, delayMs = 50
                 
    //Tworzenie Triage'u
    spdlog::info("MAIN: Assigning Triage process, result={}",
                 pm.assignProcess(std::make_unique<Triage>()));

    //Teraz już można wygenerować pacjentów lub doktorów i przypisać ich do process managera


    // Tworzenie doktorów - po jednym na specjalizację
    for (int i = 0; i < static_cast<int>(Triage::COUNT); i++) {
        spdlog::info("MAIN: Assigning Doctor process for specialist={}, result={}",
                     i,
                     pm.assignProcess(std::make_unique<Doctor>(static_cast<Triage::Specialist>(i), 50)));
    }

    //Tworzenie pacjentów - można ich tworzyć stopniowo, ale dla uproszczenia od razu 10k
    
    for (int i = 1; i <= 10000; i++) {
        spdlog::info("MAIN: Assigning Patient process, id={}, result={}",
                     i,
                     pm.assignProcess(std::make_unique<Patient>(i, false, false, false))); 
    }

    
    spdlog::warn("MAIN: Simulation running for 15 seconds");

    //sleep, aby symulacja trwała przez jakiś czas
    (void) sleep(15);
    spdlog::warn("MAIN: Time is up, termination");

    //Tu się wywołują destruktory wszystkich procesów, które powinny zamknąć procesy + reaper, i wszsystkie IPCs
}
