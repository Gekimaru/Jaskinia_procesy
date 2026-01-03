#include "loggerSender.h"
#include <cstdio>


LoggerSender::LoggerSender() : mq_("logger_file", 0) 
{ 
    if (mq_.id() < 0) { perror("LoggerSender: failed to attach message queue"); } 
}

LoggerSender::LoggerSender(bool create) : mq_("logger_file", 0,true) 
{ 
    if (mq_.id() < 0) { perror("LoggerSender: failed to init message queue"); } 
}
void LoggerSender::log(const std::string &text) {
    if (mq_.id() < 0) return;

    size_t len = text.size() + 1; // include null terminator
    if (mq_.send(1, text.c_str(), len) == -1) {
        perror("LoggerSender: send failed");
    }
}

void LoggerSender::destroy() {
    if (mq_.id() >= 0) {
        mq_.destroy();
    }
}

