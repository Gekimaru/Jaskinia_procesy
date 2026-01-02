#pragma once
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <string>
#include <iostream>

#define MAX_LOG_SIZE 512

class LoggerSender {
public:
    LoggerSender() {
        key_t key = ftok("./logger_file", 0);  // Same file and project ID
        if (key < 0) { perror("ftok"); }

        msgid = msgget(key, IPC_CREAT | 0666);
        if (msgid < 0) { perror("msgget in LoggerSender"); }
    }

    void log(const std::string &text) {
        if (msgid < 0) return;

        struct log_msg {
            long mtype = 1;
            char text[MAX_LOG_SIZE];
        } msg;

        strncpy(msg.text, text.c_str(), MAX_LOG_SIZE - 1);
        msg.text[MAX_LOG_SIZE - 1] = '\0';

        if (msgsnd(msgid, &msg, sizeof(msg.text), 0) < 0) {
            perror("msgsnd");
        }
    }

private:
    int msgid;
};
