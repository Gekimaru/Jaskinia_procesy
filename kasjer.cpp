#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <thread>
#include <chrono>
#include "shared_defs.h"
#include "loggerSender.h"
#include "messageQueue.h"

LoggerSender logger;

using namespace std;

int main(int argc, char** argv) {
    // podłączenie do shared memory
    //
    int shmId = shmget(SHM_KEY, sizeof(SharedTimeData), 0666);
    if (shmId < 0) { perror("kasjer shmget"); return 1; }

    auto* shmPtr = reinterpret_cast<SharedTimeData*>(shmat(shmId, nullptr, 0));
    if (shmPtr == reinterpret_cast<void*>(-1)) { perror("kasjer shmat"); return 1; }

    // czekamy na start zegara
    while (!shmPtr->clockStarted) std::this_thread::sleep_for(std::chrono::milliseconds(50));

    logger.log("[KASJER] Kasa otwarta. Obsługa zgodnie z nowym regulaminem wiekowym.");

    // tworzymy lokalny obiekt MessageQueue do odbioru
    MessageQueue mq("main_file", 1,false);
    if (mq.id() < 0) { perror("kasjer: blad inicjalizacji kolejki"); return 1; }

    msg_ticket_req msg;

    while (shmPtr->running) {
        // odbieramy komunikaty typu 1 (prośby o bilety)
        ssize_t ret = mq.receive(1, reinterpret_cast<char*>(&msg), sizeof(msg));
        if (ret == -1) {
            // brak komunikatów, krótka pauza
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        int price = (msg.is_repeat) ? 20 : 40; // Cena podstawowa
        int route = msg.pref_route;
        string type = (msg.adult_age >= 18) ? "Dorosły" : "Młodzież (samodzielny)";

        // Seniorzy (75+) -> Trasa 2
        if (msg.adult_age > 75) route = 2;

        // Obsługa wątków-dzieci (tylko u dorosłych)
        for (int i = 0; i < msg.num_children; ++i) {
            // Dziecko wymusza trasę 2
            route = 2;

            if (msg.children_ages[i] >= 3) {
                price += (msg.is_repeat) ? 20 : 40;
            }
        }

        logger.log("[KASJER] ID " + std::to_string(msg.visitor_id) + " (" + type + ", " 
            + std::to_string(msg.adult_age) + " lat). Dzieci: " + std::to_string(msg.num_children) 
            + " -> Trasa " + std::to_string(route) + " zaplacil " + std::to_string(price));
    }

    shmdt(shmPtr);
    exit(0);
    return 0;
}
