#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include "shared_defs.h"
#include "loggerSender.h"
#include "messageQueue.h"

LoggerSender logger;

void* child_thread(void* arg) { return nullptr; }

void visitor_logic(int id) {
    srand(time(nullptr) ^ (getpid() << 16));

    // Connect to main message queue
    MessageQueue mq("main_file", 1,false);
    if (mq.id() < 0) {
        perror("zwiedzajacy: failed to connect to main queue");
        _exit(1);
    }

    msg_ticket_req req{};
    req.mtype = 1;
    req.visitor_id = id;

    // Random age 8-90
    req.adult_age = rand() % 83 + 8;
    req.pref_route = (rand() % 10 < 3) ? 2 : 1;
    if (req.adult_age > 75) req.pref_route = 2;
    req.is_repeat = (rand() % 100 < 10);

    // Children logic: only for adults >= 18
    if (req.adult_age >= 18) {
        req.num_children = (rand() % 10 < 3) ? rand() % (MAX_CHILDREN + 1) : 0;
    } else {
        req.num_children = 0;
    }

    // Spawn threads for children
    pthread_t th[MAX_CHILDREN];
    for (int i = 0; i < req.num_children; ++i) {
        req.children_ages[i] = rand() % 7 + 1;
        pthread_create(&th[i], nullptr, child_thread, nullptr);
    }

    // Send visitor struct to the queue
    if (mq.send(req.mtype, reinterpret_cast<const char*>(&req), sizeof(req)) == -1) {
        perror("zwiedzajacy: send failed");
        exit(1);
    }

    // Join child threads
    for (int i = 0; i < req.num_children; ++i) {
        pthread_join(th[i], nullptr);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    logger.log("Zwiedzajacy o pidzie " + std::to_string(getpid()) + " sie skonczyl");
    exit(0);
}

int main(int argc, char** argv) { 
    
    // If this is a child visitor process, just run visitor_logic
    if (argc > 0 && std::string(argv[0]) != "origin") {
        visitor_logic(getpid());
        return 0;
    }

    // Connect to shared memory
    int shmId = shmget(SHM_KEY, sizeof(SharedTimeData), 0666);
    if (shmId < 0) { perror("zwiedzajacy shmget"); return 1; }

    auto* shmPtr = reinterpret_cast<SharedTimeData*>(shmat(shmId, nullptr, 0));
    if (shmPtr == reinterpret_cast<void*>(-1)) { perror("zwiedzajacy shmat"); return 1; }

    // Wait for clock to start
    while (!shmPtr->clockStarted) std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Spawn visitor processes while clock is running
    while (shmPtr->running) {
        if (fork() == 0) execl("./zwiedzajacy", "dziecko", nullptr);
        usleep(300000);
    }
    logger.log("Generator zwiedzajacych sie skonczyl");
    
    shmdt(shmPtr);
    return 0;
}
