#include "GenericIPC.hpp"

#include <sys/ipc.h>

GenericIPC::GenericIPC(const bool isCreator) : isCreator(isCreator) {
    if (isCreator)
        this->flag = IPC_CREAT | IPC_EXCL | 0600;
}
