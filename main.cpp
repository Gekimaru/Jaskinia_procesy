#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include "common.h"

using namespace std;

int main() {
    key_t m_key = ftok(FTOK_PATH, ID_MSG);
    key_t s_key = ftok(FTOK_PATH, ID_SHM);
    key_t k_key = ftok(FTOK_PATH, ID_SEM);

    int msgid = msgget(m_key, IPC_CREAT | 0666);
    int shmid = shmget(s_key, sizeof(CaveStatus), IPC_CREAT | 0666);
    int semid = semget(k_key, 2, IPC_CREAT | 0666);

    CaveStatus* status = (CaveStatus*)shmat(shmid, NULL, 0);
    status->ticket_office_open = true;
    status->cave_open = true;
    
    // Inicjalizacja kładek: K1-Wchodzi, K2-Wchodzi, Limit 5
    for(int i=0; i<2; i++) {
        status->bridges[i] = {0, DIR_IN, 5};
        semctl(semid, i, SETVAL, 1);
    }

    if (fork() == 0) execl("./kasjer", "kasjer", NULL);
    if (fork() == 0) execl("./straznik", "straznik", NULL);
    if (fork() == 0) execl("./zwiedzajacy", "zwiedzajacy", "inf", NULL);

    cout << "[MAIN] Symulacja ruszyła. T2 za 30s." << endl;
    sleep(30);

    status->ticket_office_open = false;
    cout << "[MAIN] Kasa zamknięta." << endl;

    sleep(10);
    status->cave_open = false;

    while (wait(NULL) > 0);

    msgctl(msgid, IPC_RMID, NULL);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
