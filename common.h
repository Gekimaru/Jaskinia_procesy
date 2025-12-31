#ifndef COMMON_H
#define COMMON_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define FTOK_PATH "main.cpp"
#define ID_MSG 'M'
#define ID_SHM 'S'
#define ID_SEM 'K'

#define DIR_IN 1
#define DIR_OUT 2
#define DIR_NONE 0

struct Bridge {
    int on_bridge;    // Ilu ludzi na kładce
    int current_dir;  // W jakim kierunku kładka teraz pracuje (1-IN, 2-OUT)
    int k_limit;      // Max osób
};

struct CaveStatus {
    bool ticket_office_open;
    bool cave_open;
    Bridge bridges[2]; // Dwie kładki
};

struct msg_ticket_req {
    long mtype;
    int visitor_id;
    int num_children;
};

// Pomocnicza funkcja do semaforów
inline void op_sem(int semid, int sem_num, int val) {
    struct sembuf sops = {(unsigned short)sem_num, (short)val, 0};
    semop(semid, &sops, 1);
}

#endif
