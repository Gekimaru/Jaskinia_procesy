#pragma once
#include <string>
#include "messageQueue.h"

#define MAX_LOG_SIZE 512

class LoggerSender {
public:
    LoggerSender();
    LoggerSender(bool create);
    void log(const std::string &text);
    void destroy();
private:
    MessageQueue mq_;
};
