#pragma once
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <cstdio>

key_t getKeyFromPath(const char *path, int proj_id){

    key_t key = ftok(path, proj_id);
    if (key < 0) {
        perror("ftok failed");
        return -1;
    }
    return key;
}

