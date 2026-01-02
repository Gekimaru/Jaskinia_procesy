#pragma once
#include <sys/ipc.h>
#include <typeinfo>
int createMessageQueue(key_t key,int messageFlag);
__ssize_t getMessageFromMessageQueue(int messageQueueId,void * bufferForTheMessage,);

