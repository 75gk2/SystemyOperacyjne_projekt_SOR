#include "SharedMemory.hpp"

#include <csignal>
#include <unordered_set>
#include <unistd.h>

#include "spdlog/spdlog.h"

namespace {
    void sendSignalIfValid(pid_t pid, int sig) {
        //exclude self process
        if (pid <= 0 || pid == getpid()) {
            return;
        }

        if (kill(pid, sig) == -1) {
            spdlog::warn("Director: failed to signal pid={} sig={}", pid, sig);
        }
    }
}

int main(int argc, char *argv[]) {
    SharedMemory memory(false);

    if (argc > 1) {
        // arg=1 -> SIGUSR1 to all doctors (break)
        // arg=2 -> SIGUSR2 to all patients and doctors (evacuate)
        // arg=3-9 -> SIGUSR1 to doctor index (arg-2, to single doctor)
        const int cmd = std::atoi(argv[1]);
        const auto *data = memory.getPtr();
        if (!data) {
            spdlog::error("Director: shared memory not available");
            return 1;
        }

        // signal all doctors to break receiving patients
        if (cmd == 1) {
            for (const auto pid: data->lekarzePIDs) {
                sendSignalIfValid(pid, SIGUSR1);
            }
            return 0;
        }

        // signal single doctor to break receiving patients
        if (cmd >= 3 && cmd <= 9) {
            const int indexOneBased = cmd - 2;
            const int index = indexOneBased - 1;

            //ensure that index is in range,
            if (index >= 0 && index < static_cast<int>(data->lekarzePIDs.size())) {
                sendSignalIfValid(data->lekarzePIDs[index], SIGUSR1);
            } else {
                spdlog::warn("Director: doctor index out of range, indexOneBased={}", indexOneBased);
            }
            return 0;
        }


        // signal all processes to evacuate
        if (cmd == 2) {
            //filter all processes to unique pids
            std::unordered_set<pid_t> signaled;

            //all processes
            for (std::size_t i = 0; i < data->processCount; ++i) {
                const auto pid = data->processPIDs[i];
                if (pid <= 0 || pid == getpid()) {
                    //exclude self process
                    continue;
                }
                signaled.insert(pid);
            }

            //plus doctors
            for (const auto pid: data->lekarzePIDs) {
                if (pid > 0) {
                    signaled.insert(pid);
                }
            }

            //and send to all from list
            for (const auto pid: signaled) {
                sendSignalIfValid(pid, SIGUSR2);
            }
            return 0;
        }
    }

    return 0;
}