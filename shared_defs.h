#pragma once
#include <cstdint>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>


#define QUEUE_PATH_TICKET "queue_ticket"
#define QUEUE_PATH_KLADKI_ENTR "queue_kladki_entr"

#define MSG_KEY 0x9975
#define MAX_CHILDREN 5
#define SHM_KEY 0x12345

struct msg_ticket_req {
    long mtype;
    int visitor_id;
    int adult_age;
    int pref_route;
    bool is_repeat;
    int num_children;
    int children_ages[MAX_CHILDREN];
};


struct SharedTimeData {
    int64_t tp_ms;       // czas startu w ms od epoki
    int64_t tk_ms;       // czas końca w ms od epoki
    int64_t now_ms;      // aktualny symulacyjny czas w ms
    bool running;        // flaga symulacji
    bool clockStarted;   // flaga: zegar wystartował
};

key_t getKeyFromPath(const char* path,int proj_id);
