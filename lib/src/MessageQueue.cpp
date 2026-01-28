#include "MessageQueue.hpp"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <cerrno>
#include <cstdio>

#include "constants.hpp"
#include "spdlog/spdlog.h"

MessageQueue::MessageQueue(const char queueId, bool isCreator)
    : GenericIPC(isCreator), msqId(-1), queueId(queueId) {
    spdlog::debug("MessageQueue: init, queueId={}, isCreator={}", queueId, isCreator);

    const char proj_id = queueId  ? queueId : MSG_PROJ_ID;
    const key_t key = ftok(".", proj_id);
    if (key == -1) {
        spdlog::error("MessageQueue: ftok failed for queueId={}", queueId);
        perror("MessageQueue: ftok failed");
        throw std::runtime_error("ftok failed");
    }

    msqId = msgget(key, getFlag());

    if (msqId == -1) {
        spdlog::error("MessageQueue: msgget failed, queueId={}", queueId);
        perror("MessageQueue: msgget failed");
        throw std::runtime_error("msgget failed");
    }

    spdlog::debug("MessageQueue: init completed! queueId={}, key={}, msqId={}", queueId, key, msqId);
}

MessageQueue::~MessageQueue() {
    if (isThisCreator()) {
        spdlog::debug("MessageQueue: destructor of creator, deleting message queue msqId={}", msqId);
        if (msqId != -1) {
            if (const auto result = msgctl(msqId, IPC_RMID, nullptr); result == -1) {
                spdlog::error("MessageQueue: deletion of message queue failed! RISK OF LEAK! msqId={}", msqId);
                perror("MessageQueue: msgctl(IPC_RMID) failed");
            }
            msqId = -1;
        } else {
            spdlog::error("MessageQueue: Owner lost access to msqId!");
        }
    }
}
