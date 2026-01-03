#pragma once
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstdio>
#include <cstring>

class MessageQueue {
public:
    // Constructor: file for ftok, project id, permissions
    MessageQueue(const char* file, int proj_id,bool create = false ,int perms = 0666);

    int id() const { return id_; }

    // Send arbitrary data (structs or raw bytes)
    int send(long type, const char* data, size_t size);

    // Receive arbitrary data
    // Returns number of bytes received, or -1 on error/no message
    ssize_t receive(long type, char* buffer, size_t size);

    // Remove queue from the system
    void destroy();

private:
    int id_;
    static constexpr size_t MAX_MSG_SIZE = 1024; // max message size in bytes
};
