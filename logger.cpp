#pragma once
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <ctime>
#include <cstring>
#include <cerrno>
#include "loggerSender.h"
#define MAX_LOG_SIZE 512
LoggerSender logger;
struct log_msg {
    long mtype;
    char text[MAX_LOG_SIZE];
};

std::string timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf),
                  "%Y-%m-%d %H:%M:%S",
                  std::localtime(&now));
    return buf;
}

static bool write_all(int fd, const char* buf, size_t len) {
    size_t written = 0;
    while (written < len) {
        ssize_t w = write(fd, buf + written, len - written);
        if (w < 0) {
            //if (errno == EINTR) continue;   
            return false;                   
        }
        written += static_cast<size_t>(w);
    }
    return true;
}
int main() {
    key_t key = ftok("logger_file", 0);
    if (key < 0) { perror("ftok"); return 1; }

    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid < 0) { perror("msgget"); return 1; }

    // Truncate old file
    int fd = open("simulation.log", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) { perror("open(O_TRUNC)"); return 1; }
    close(fd);

    // Reopen for atomic appends
    fd = open("simulation.log", O_CREAT | O_WRONLY | O_APPEND, 0666);
    if (fd < 0) { perror("open(O_APPEND)"); return 1; }

    log_msg msg{};

    while (true) {
        if (msgrcv(msgid, &msg, sizeof(msg.text), 0, 0) < 0) {
            perror("msgrcv");
            break;
        }

        std::string line = timestamp();
        line += " ";
        line += msg.text;
        line += "\n";

        const char* buf = line.c_str();
        size_t len = line.size();

        // Write to file
        if (!write_all(fd, buf, len)) {
            perror("write(logfile)");
            break;
        }

        // Mirror to stderr
        if (!write_all(STDERR_FILENO, buf, len)) {
            perror("write(stderr)");
            break;
        }

        if (std::strcmp(msg.text, "__LOGGER_STOP__") == 0)
            break;
    }

    if(close(fd) == -1){
        perror("Couldn't close file");
    }
    logger.destroy();
    msgctl(msgid, IPC_RMID, nullptr);
    return 0;
}
