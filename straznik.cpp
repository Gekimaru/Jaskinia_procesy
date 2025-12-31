#include <iostream>
#include <unistd.h>
#include "common.h"

using namespace std;

int main() {
    key_t s_key = ftok(FTOK_PATH, ID_SHM);
    key_t k_key = ftok(FTOK_PATH, ID_SEM);
    int shmid = shmget(s_key, sizeof(CaveStatus), 0666);
    int semid = semget(k_key, 2, 0666);
    CaveStatus* status = (CaveStatus*)shmat(shmid, NULL, 0);

    while (status->ticket_office_open || status->cave_open) {
        sleep(10); // Zmieniaj konfigurację co 10 sekund
        
        op_sem(semid, 0, -1);
        op_sem(semid, 1, -1);

        // Losowa zmiana trybów dla przykładu
        status->bridges[0].current_dir = (status->bridges[0].current_dir == DIR_IN) ? DIR_OUT : DIR_IN;
        status->bridges[1].current_dir = (status->bridges[1].current_dir == DIR_IN) ? DIR_OUT : DIR_IN;

        cout << "[STRAŻNIK] Zmiana kierunków! K1: " << status->bridges[0].current_dir 
             << " | K2: " << status->bridges[1].current_dir << endl;

        op_sem(semid, 1, 1);
        op_sem(semid, 0, 1);
    }
    return 0;
}
