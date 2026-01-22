#pragma once

#include "GenericIPC.hpp"

class MessageQueue : public GenericIPC {
    int msqId;
    const char queueId;

public:
    explicit MessageQueue(char queueId, bool isCreator);
    ~MessageQueue() override;

    [[nodiscard]] int getMsgId() const {
        return msqId;
    }

    [[nodiscard]] char getQueueName() const {
        return queueId;
    }

    template <typename T>
    int send(const T &msg, long mtype = 1);

    template <typename T>
    int receive(T &msg, long mtype = 0);
};

#include "MessageQueue.tpp"