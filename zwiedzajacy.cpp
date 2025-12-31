#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include "common.h"

using namespace std;

void przejdz_kladka(int id, int grupa, int kierunek, int semid, CaveStatus* status) {
    bool przeszedl = false;
    while (!przeszedl) {
        for (int i = 0; i < 2; i++) {
            op_sem(semid, i, -1); // Sprawdź kładkę i
            
            if (status->bridges[i].current_dir == kierunek && 
                status->bridges[i].on_bridge + grupa <= status->bridges[i].k_limit) {
                
                status->bridges[i].on_bridge += grupa;
                op_sem(semid, i, 1);
                
                cout << "[ID " << id << "] Przechodzi kładką " << i+1 << endl;
                sleep(2); // Idzie...
                
                op_sem(semid, i, -1);
                status->bridges[i].on_bridge -= grupa;
                op_sem(semid, i, 1);
                
                przeszedl = true;
                break;
            }
            op_sem(semid, i, 1);
        }
        if (!przeszedl) usleep(500000); // "Cofnij się" i czekaj na zmianę kierunku/miejsca
    }
}

void visitor_logic(int id, int msgid, CaveStatus* status, int semid) {
    int grupa = 1 + (rand() % 3); // 1 dorosły + 0-2 dzieci
    
    // Kup bilet
    msg_ticket_req req = {1, id, grupa - 1};
    msgsnd(msgid, &req, sizeof(req)-sizeof(long), 0);

    // Wejście (DIR_IN)
    przejdz_kladka(id, grupa, DIR_IN, semid, status);
    
    cout << "[ID " << id << "] W jaskini." << endl;
    sleep(5);

    // Wyjście (DIR_OUT)
    przejdz_kladka(id, grupa, DIR_OUT, semid, status);
    
    cout << "[ID " << id << "] Wyszedł." << endl;
    exit(0);
}

int main(int argc, char* argv[]) {
    key_t m_key = ftok(FTOK_PATH, ID_MSG);
    key_t s_key = ftok(FTOK_PATH, ID_SHM);
    key_t k_key = ftok(FTOK_PATH, ID_SEM);

    int msgid = msgget(m_key, 0666);
    int shmid = shmget(s_key, sizeof(CaveStatus), 0666);
    int semid = semget(k_key, 2, 0666);
    CaveStatus* status = (CaveStatus*)shmat(shmid, NULL, 0);

    if (argc > 1 && string(argv[1]) == "inf") {
        int counter = 1;
        while (status->ticket_office_open) {
            if (fork() == 0) visitor_logic(counter++, msgid, status, semid);
            usleep(800000);
        }
        while (wait(NULL) > 0);
    }
    return 0;
}
