#pragma once
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <thread>
#include <chrono>
#include <cstdlib>
#include "shared_defs.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "loggerSender.h"

LoggerSender logger;



void* child_thread(void* arg) { return NULL; }

void visitor_logic(int id) {
	srand(time(NULL) ^ (getpid() << 16));
	int msgid = msgget(MSG_KEY, 0666);

	msg_ticket_req req;
	req.mtype = 1;
	req.visitor_id = id;

	// Losujemy wiek procesu (od 8 do 90 lat)
	req.adult_age = rand() % 83 + 8; 
	req.pref_route = rand() % 2 + 1;
	req.is_repeat = (rand() % 100 < 10);

	// Logika dzieci: tylko jeśli proces ma przynajmniej 18 lat
	if (req.adult_age >= 18) {
		req.num_children = 0;
		if(rand()%10<2){
			req.num_children = rand() % (MAX_CHILDREN + 1);
		}
	} else {
		req.num_children = 0; // Osoba 8-17 lat nie może mieć dzieci
	}

	pthread_t th[MAX_CHILDREN];
	for (int i = 0; i < req.num_children; i++) {
		// Dziecko (wątek) ma zawsze 1-7 lat
		req.children_ages[i] = rand() % 7 + 1; 
		pthread_create(&th[i], NULL, child_thread, NULL);
	}

	msgsnd(msgid, &req, sizeof(req) - sizeof(long), 0);

	for (int i = 0; i < req.num_children; i++) {
		pthread_join(th[i], NULL);
	}
	exit(0);
}

int main(int argc,char** argv) {
	if(std::string(argv[0]) != "origin"){
		visitor_logic(getpid());
		exit(0);
	}
	int shmId = shmget(SHM_KEY, sizeof(SharedTimeData), 0666);
	if (shmId < 0) { perror("zwiedzajacy shmget"); return 1; }

	auto* shmPtr = reinterpret_cast<SharedTimeData*>(shmat(shmId, nullptr, 0));
	if (shmPtr == reinterpret_cast<void*>(-1)) { perror("zwiedzajacy shmat"); return 1; }

	// czekamy na start zegara
	while (!shmPtr->clockStarted) std::this_thread::sleep_for(std::chrono::milliseconds(50));

	int visitorId = 0;
	while (shmPtr->running) {
		if (fork() == 0) execl("./zwiedzajacy", "dziecko", nullptr);

		usleep(300000);
	}

	shmdt(shmPtr);
	return 0;
}
