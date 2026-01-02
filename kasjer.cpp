#pragma once
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <thread>
#include <chrono>
#include "shared_defs.h"
#include "loggerSender.h"

LoggerSender logger;



#include <fstream>
using namespace std;

int main(int argc, char** argv) {
	int shmId = shmget(SHM_KEY, sizeof(SharedTimeData), 0666);
	if (shmId < 0) { perror("kasjer shmget"); return 1; }

	auto* shmPtr = reinterpret_cast<SharedTimeData*>(shmat(shmId, nullptr, 0));
	if (shmPtr == reinterpret_cast<void*>(-1)) { perror("kasjer shmat"); return 1; }

	// czekamy na start zegara
	while (!shmPtr->clockStarted) std::this_thread::sleep_for(std::chrono::milliseconds(50));

	int counter = 0;
	logger.log("[KASJER] Kasa otwarta. Obsługa zgodnie z nowym regulaminem wiekowym.");
	while (shmPtr->running) {
		int msgid = msgget(MSG_KEY, 0666);
		ofstream report("raport_jaskinia.txt", ios::trunc);


		msg_ticket_req msg;
		while (msgrcv(msgid, &msg, sizeof(msg), 1, 0) != -1) {
			double price = (msg.is_repeat) ? 20.0 : 40.0; // Cena za proces
			int route = msg.pref_route;
			string type = (msg.adult_age >= 18) ? "Dorosły" : "Młodzież (samodzielny)";

			// Seniorzy (procesy 75+) -> Trasa 2
			if (msg.adult_age > 75) route = 2;

			// Obsługa wątków-dzieci (tylko u dorosłych)
			for (int i = 0; i < msg.num_children; ++i) {
				// Skoro to dziecko (wątek), to ma < 8 lat -> wymusza Trasę 2
				route = 2;

				if (msg.children_ages[i] >= 3) {
					price += (msg.is_repeat) ? 20.0 : 40.0; 
				}
			}
			logger.log("[KASJER] ID " + std::to_string(msg.visitor_id) + " (" + type + ", " + std::to_string(msg.adult_age) + " lat). "
           		+ "Dzieci: " + std::to_string(msg.num_children) + " -> Trasa " + std::to_string(route));

		}
	}

	shmdt(shmPtr);
	return 0;
}
