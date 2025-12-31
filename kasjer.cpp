#include <iostream>
#include <fstream>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include "common.h"

using namespace std;

int main() {
    // 1. Generowanie tych samych kluczy co w main i zwiedzajacy
    key_t msg_key = ftok(FTOK_PATH, PROJ_ID_MSG);
    key_t shm_key = ftok(FTOK_PATH, PROJ_ID_SHM);

    // 2. Podpięcie do istniejących zasobów
    int msgid = msgget(msg_key, 0666);
    int shmid = shmget(shm_key, sizeof(CaveStatus), 0666);

    if (msgid == -1 || shmid == -1) {
        cerr << "[KASJER] Błąd: Nie można uzyskać dostępu do IPC. Czy MAIN działa?" << endl;
        return 1;
    }

    CaveStatus* status = (CaveStatus*)shmat(shmid, NULL, 0);
    
    // Otwarcie pliku raportu
    ofstream report("raport_jaskinia.txt", ios::app);
    if (!report.is_open()) {
        cerr << "[KASJER] Błąd otwarcia pliku raportu!" << endl;
        return 1;
    }

    cout << "[KASJER] Kasa otwarta. Czekam na klientów..." << endl;

    msg_ticket_req msg;

    // 3. Pętla pracy Kasjera
    // Pracujemy dopóki kasa jest otwarta LUB w kolejce są jeszcze wiadomości do odebrania
    while (true) {
        // Sprawdzamy czy są wiadomości bez blokowania na stałe (IPC_NOWAIT)
        // Dzięki temu możemy co chwilę sprawdzać stan status->ticket_office_open
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, IPC_NOWAIT) != -1) {
            
            string route = (msg.pref_route == 1) ? "Trasa Krótka" : "Trasa Długa";
            int price = (msg.pref_route == 1) ? 30 : 50;
            int total_people = 1 + msg.num_children;

            // Logowanie do pliku
            report << "ID: " << msg.visitor_id 
                   << " | Wiek: " << msg.age 
                   << " | Grupa: " << total_people
                   << " | " << route 
                   << " | Koszt: " << price * total_people << " PLN" << endl;
            
            report.flush(); // Natychmiastowy zapis na dysk

            cout << "[KASJER] Obsłużono ID: " << msg.visitor_id 
                 << " (Grupa: " << total_people << ")" << endl;
        } else {
            // Jeśli nie ma wiadomości, sprawdźmy czy kasa nie została zamknięta
            if (!status->ticket_office_open) {
                // Dodatkowe sprawdzenie, czy w międzyczasie coś nie wpadło
                if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, IPC_NOWAIT) == -1) {
                    break; // Kolejka pusta i kasa zamknięta -> kończymy
                }
            }
            // Mała pauza, żeby nie zużywać 100% CPU w pętli non-blocking
            usleep(50000); 
        }
    }

    cout << "[KASJER] Kasa zamknięta. Raport zapisany." << endl;
    
    report.close();
    shmdt(status);

    return 0;
}
