#pragma once
#include "messageQueue.h"
#include <sys/msg.h>
#include <sys/types.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>

MessageQueue::MessageQueue(const char* file, int proj_id,bool create ,int perms) {
    key_t key = ftok(file, proj_id);
    if (key < 0) {
        perror("MessageQueue: ftok failed");
        id_ = -1;
        return;
    }
    if(create) id_ = msgget(key, IPC_CREAT | perms);
    else       id_ = msgget(key, perms );

    if (id_ < 0) {
        perror("MessageQueue: msgget failed");
    }
}

int MessageQueue::send(long type, const char* data, size_t size) {
    if (id_ < 0) return -1;
    if (size > MAX_MSG_SIZE) {
        fprintf(stderr, "MessageQueue::send: message too large\n");
        return -1;
    }

    struct msgbuf {
        long mtype;
        char mtext[MAX_MSG_SIZE];
    } msg;

    msg.mtype = type;
    memcpy(msg.mtext, data, size);

    if (msgsnd(id_, &msg, size, 0) < 0) {
        
        perror("MessageQueue: msgsnd failed");
        return -1;
    }
    return 0;
}

ssize_t MessageQueue::receive(long type, char* buffer, size_t size) {
    if (id_ < 0) return -1;

    struct msgbuf {
        long mtype;
        char mtext[MAX_MSG_SIZE];
    } msg;

    ssize_t ret = msgrcv(id_, &msg, sizeof(msg.mtext), type, IPC_NOWAIT);
    if (ret < 0) {
        if (errno != ENOMSG) perror("MessageQueue: msgrcv failed");
        return -1;
    }

    // Copy only the requested number of bytes
    if ((size_t)ret > size) ret = size;
    memcpy(buffer, msg.mtext, ret);
    return ret;
}

void MessageQueue::destroy() {
    if (id_ >= 0) msgctl(id_, IPC_RMID, nullptr);
}
