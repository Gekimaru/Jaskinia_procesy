#pragma once
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <ctime>
#include <fstream>
#include <cstring>

#define MAX_LOG_SIZE 512

struct log_msg {
    long mtype;
    char text[MAX_LOG_SIZE];
};

std::string timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

int main() {
    // Use ftok to generate unique key
    key_t key = ftok("logger_file", 0);  // File must exist, 'L' is project ID
    if (key < 0) { perror("ftok"); return 1; }

    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid < 0) { perror("msgget"); return 1; }

    std::ofstream logfile("simulation.log", std::ios::app);
    if (!logfile.is_open()) { perror("ofstream"); return 1; }

    log_msg msg;
    while (true) {
        if (msgrcv(msgid, &msg, sizeof(msg.text), 0, 0) < 0) {
            perror("msgrcv"); break;
        }

        std::string line = timestamp() + " " + std::string(msg.text);
        logfile << line << std::endl;

        if (std::strcmp(msg.text, "__LOGGER_STOP__") == 0) break;
    }

    logfile.close();
    msgctl(msgid, IPC_RMID, nullptr);
    return 0;
}
