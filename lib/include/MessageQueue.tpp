#pragma once
#include "spdlog/spdlog.h"
#include <sys/msg.h>

template<typename T>
int MessageQueue::send(const T &msg, long mtype) {
    spdlog::debug("MessageQueue: sending message, queueId={}, mtype={}", queueId, mtype);

    struct {
        long mtype;
        T data;
    } buffer;

    buffer.mtype = mtype;
    buffer.data = msg;

    int result = msgsnd(msqId, &buffer, sizeof(T), 0);

    if (result == -1) {
        spdlog::error("MessageQueue: msgsnd failed! queueId={}, mtype={}", queueId, mtype);
        return -1;
    }

    spdlog::debug("MessageQueue: message sent successfully");
    return result;
}

template<typename T>
int MessageQueue::receive(T &msg, long mtype, bool wait) {
    spdlog::debug("MessageQueue: receiving message, queueId={}, mtype={}", queueId, mtype);

    struct {
        long mtype;
        T data;
    } buffer;

    int result = msgrcv(msqId, &buffer, sizeof(T), mtype, wait ? 0 : IPC_NOWAIT);
    if (result == -1) {
        if (errno == ENOMSG && !wait) {
            return -2; // brak wiadomości
        }
        spdlog::error("MessageQueue: msgrcv failed! queueId={}, mtype={}", queueId, mtype);
        return -1;
    }
    msg = buffer.data;
    return result;
}
