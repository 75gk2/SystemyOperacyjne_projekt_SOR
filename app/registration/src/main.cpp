#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <unistd.h>

#include "MessageQueue.hpp"
#include "SemaphoreArray.hpp"
#include "SharedMemory.hpp"
#include "childProcesses/Registration.hpp"
#include "constants.hpp"
#include "utils.hpp"

int window1_id = 1;
int window2_id = 2;
std::atomic<int> g_delayMs{0};


struct WindowState {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    int shutdownTakenId = 0; // Only on thread can take it
    bool signal_shutdown = false; // request for shu on window
};

struct WindowArgs {
    int windowId;
    WindowState *state;
};


void *registrationWindow(void *arg) {
    const auto *args = static_cast<WindowArgs *>(arg);
    int windowId = args->windowId;
    bool isOneElseTwo = windowId == window1_id;
    spdlog::info("RegistrationWindow, initialized window, windowId={}", windowId);
    WindowState *state = args->state;
    MessageQueue broadcast{Registration::Q_REGISTRATION_ID, false};
    MessageQueue myWindowReceive{Registration::QID_WINDOW_1_IN, false};
    MessageQueue myWindowSend{Registration::QID_WINDOW_1_OUT, false};
    bool tokenSent = false;
    SemaphoreArray semaphores(false);
    while (true) {
        spdlog::info("Semcount={}", semaphores.getValue(SEM_TYPE::REGISTRATION_QUEUE));
        if (!tokenSent) {
            // guard for double continuation
            if (broadcast.send(Registration::Q_REGISTRATION_STRUCT{isOneElseTwo}) < 0) {
                spdlog::warn("RegistrationWindow failed to send availability token! windowId={}", windowId);
            } else {
                tokenSent = true;
            }
        }

        Registration::Q_WINDOW_STRUCT msg{};
        int r = myWindowReceive.receive(msg, -Registration::QTYPE_WINDOW_IN, true);

        if (r < 0) {
            spdlog::warn("RegistrationWindow failed to get message! windowId={}", windowId);
            continue;
        }

        spdlog::debug("Registration window got {} message! windowId={}", msg.isVIP ? "VIP" : "standard", windowId);
        tokenSent = false;


        Registration::Q_WINDOW_OUT_STRUCT response{
            msg.isVIP || msg.diesNow
        };
        const int delayMs = g_delayMs.load();
        if (delayMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }
        if (myWindowSend.send(response, msg.socialId)) {
            spdlog::warn("RegistrationWindow failed to send message! windowId={}, socialId={}", windowId, msg.socialId);
        }
        pthread_mutex_lock(&state->lock);

        //check for shutdown request and if shutdown free to take it
        if (state->signal_shutdown && state->shutdownTakenId == 0) {
            // take the shutdown
            spdlog::info("RegistrationWindow: received shutdown in window {}", windowId);
            state->shutdownTakenId = windowId;
            pthread_cond_signal(&state->cond);
            pthread_mutex_unlock(&state->lock);
            break;
        }
        //if taken or not set continue;
        pthread_mutex_unlock(&state->lock);
    }

    spdlog::info("RegistrationWindow: Shutting down window {}", windowId);
    return nullptr;
}


bool registrationIncreaseWindowsLock(const int n) {
    const int K1 = n / 2;
    const SemaphoreArray semaphores(false);
    MessageQueue ctrlQueue(Registration::QID_REGISTRATION_CTRL, false);

    while (true) {
        int value = semaphores.getValue(SEM_TYPE::REGISTRATION_QUEUE);
        if (value < 0) {
            spdlog::error("Registration: Failed to read REGISTRATION_QUEUE semaphore");
            return false;
        }
        if (value > K1) {
            return true;
        }
        Registration::Q_REGISTRATION_CTRL_STRUCT event{};
        if (ctrlQueue.receive(event, Registration::QTYPE_REGISTRATION_CTRL, true) < 0) {
            spdlog::error("Registration: failed to receive registration ctrl event");
            return false;
        }
    }
}

bool registrationDecreaseWindowsLock(const int n) {
    const int K2 = n / 3;
    const SemaphoreArray semaphores(false);
    MessageQueue ctrlQueue(Registration::QID_REGISTRATION_CTRL, false);

    while (true) {
        int value = semaphores.getValue(SEM_TYPE::REGISTRATION_QUEUE);
        if (value < 0) {
            spdlog::error("Registration: Failed to read REGISTRATION_QUEUE semaphore");
            return false;
        }
        if (value < K2) {
            return true;
        }
        Registration::Q_REGISTRATION_CTRL_STRUCT event{};
        if (ctrlQueue.receive(event, Registration::QTYPE_REGISTRATION_CTRL, true) < 0) {
            spdlog::error("Registration: failed to receive registration ctrl event");
            return false;
        }
    }
}

typedef struct {
    int id;
    bool *flag;
    short *feedback;
    pthread_mutex_t *mutex;
    SemaphoreArray *semaphores;
    pthread_cond_t *feedbackCond;
} ThreadData;

bool registrationWindowsController(const int n) {
    SemaphoreArray semaphores = SemaphoreArray(false);
    pthread_t window1tid, window2tid;
    bool isWindow1running = false;
    bool isWindow2running = false;

    WindowState state;
    WindowArgs args1{window1_id, &state};
    WindowArgs args2{window2_id, &state};


    if (pthread_create(&window1tid, nullptr, registrationWindow, &args1)) {
        spdlog::error("Registration[INIT]: Failed to create window 1 thread");
        return false;
    }

    isWindow1running = true;
    spdlog::info("Registration: Initialized new thread of window{}", window1_id);

    while (true) {
        //If either of these fail, we exit the loop and kill the threads

        //Wait for queue to grow
        if (!registrationIncreaseWindowsLock(n)) return false;

        //Start new window
        if (!isWindow1running) {
            if (pthread_create(&window1tid, nullptr, registrationWindow, &args1)) {
                spdlog::error("Registration: Failed to create window 1 thread");
                return false;
            }
            isWindow1running = true;
            spdlog::info("Registration: Initialized new thread of window{}", window1_id);
        } else if (!isWindow2running) {
            if (pthread_create(&window2tid, nullptr, registrationWindow, &args2)) {
                spdlog::error("Registration: Failed to create window 2 thread");
                return false;
            }
            isWindow2running = true;
            spdlog::info("Registration: Initialized new thread of window{}", window2_id);
        } else {
            spdlog::error("Registration: Failed to second window 2 thread");
        }

        //Wait for queue to shorten
        if (!registrationDecreaseWindowsLock(n)) return false;

        pthread_mutex_lock(&state.lock);
        state.signal_shutdown = true;

        //wait for shutdown to be taken
        while (state.shutdownTakenId == 0) {
            pthread_cond_wait(&state.cond, &state.lock);
        }

        if (state.shutdownTakenId == 1) {
            if (pthread_join(window1tid, nullptr)) {
                spdlog::error("Registration: Failed to join window 1 thread");
                return false;
            }
            isWindow1running = false;

            spdlog::info("Registration: Shut down window {}", window1_id);
        } else if (state.shutdownTakenId == 2) {
            if (pthread_join(window2tid, nullptr)) {
                spdlog::error("Registration: Failed to join window 2 thread");
                return false;
            }
            isWindow2running = false;
            spdlog::info("Registration: Shut down window {}", window2_id);
        } else {
            spdlog::error("Registration: shutting down window failed - unknow id, id={}", window1_id);
            return false;
        }

        state.shutdownTakenId = 0;
        state.signal_shutdown = false;
        pthread_mutex_unlock(&state.lock);
    }
}

int main(int argc, char *argv[]) {
    spdlog::info("Registration: init");
    int n = POCZEKALNIA_SIZE;
    if (argc > 1) {
        n = std::max(1, std::atoi(argv[1]));
    }
    if (argc > 2) {
        g_delayMs.store(std::max(0, std::atoi(argv[2])));
    }
    try {
        MessageQueue registrationCtrl(Registration::QID_REGISTRATION_CTRL, false);
        SemaphoreArray semaphores(false);
        if (!semaphores.setValue(SEM_TYPE::REGISTRATION_QUEUE, 0)) {
            spdlog::error("Registration: failed to initialize REGISTRATION_QUEUE semaphore");
            return 1;
        }
    } catch (const std::exception &e) {
        spdlog::error("Registration: semaphore init failed: {}", e.what());
        return 1;
    }
    if (!registrationWindowsController(n)) {
        spdlog::error("Registration: windows controller failed ://");
        return 1;
    }
    spdlog::info("Registration: finished");
    return 0;
}