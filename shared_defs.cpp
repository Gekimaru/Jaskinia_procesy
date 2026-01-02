#pragma once
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include "loggerSender.h"

LoggerSender logger;

key_t getKeyFromPath(const char *path, int proj_id){
	key_t temp = ftok(path,proj_id);

	switch(temp)
	{
		case -1:
            
			break;
		default:
			break; 
	}
	return temp;
}
