#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <sys/wait.h>
#include "shared_defs.h"
#include "messageQueue.h"
#include "loggerSender.h"

LoggerSender logger(true);
MessageQueue* msgQueue = nullptr;    // główna kolejka procesów

using Clock = std::chrono::steady_clock;

int shmId = -1;
SharedTimeData* shmPtr = nullptr;
bool stopRequested = false;
pid_t kasjerPid = -1;
pid_t zwiedzajacyPid = -1;
pid_t loggerPid = -1;

void handleSignal(int) {
    stopRequested = true;
    if (shmPtr) shmPtr->running = false;

    if (kasjerPid > 0) kill(kasjerPid, SIGINT);
    if (zwiedzajacyPid > 0) kill(zwiedzajacyPid, SIGINT);
    if (loggerPid > 0) kill(loggerPid, SIGINT);

    if (shmPtr) shmdt(shmPtr);
    if (shmId >= 0) shmctl(shmId, IPC_RMID, nullptr);
    if (msgQueue) msgQueue->destroy();

    logger.log("__LOGGER_STOP__");
}

void clockThread(int Tp, int Tk) {
    auto startWall = Clock::now();
    auto tp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(startWall.time_since_epoch()).count();
    auto tk_ms = tp_ms + (Tk - Tp) * 1000;

    shmPtr->tp_ms = tp_ms;
    shmPtr->tk_ms = tk_ms;
    shmPtr->running = true;
    shmPtr->clockStarted = true;

    while (!stopRequested && shmPtr->running) {
        auto now = Clock::now();
        shmPtr->now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        if (shmPtr->now_ms >= shmPtr->tk_ms) shmPtr->running = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Użycie: ./main Tp Tk (sekundy)\n";
        return 1;
    }

    signal(SIGINT, handleSignal);

    // Stwórz kolejkę główną procesów
    msgQueue = new MessageQueue("main_file", 1,true);
    if (msgQueue->id() < 0) {
        perror("Main queue init failed");
        return 1;
    }

    int Tp = std::stoi(argv[1]);
    int Tk = std::stoi(argv[2]);

    // Stwórz shared memory
    shmId = shmget(SHM_KEY, sizeof(SharedTimeData), IPC_CREAT | 0666);
    if (shmId < 0) { perror("shmget"); return 1; }

    shmPtr = reinterpret_cast<SharedTimeData*>(shmat(shmId, nullptr, 0));
    if (shmPtr == reinterpret_cast<void*>(-1)) { perror("shmat"); return 1; }

    shmPtr->running = false;
    shmPtr->clockStarted = false;

    // Uruchom logger
    loggerPid = fork();
    if (loggerPid == 0) {
        execl("./logger", "logger", nullptr);
        perror("execl logger");
        _exit(1);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Uruchom kasjera
    kasjerPid = fork();
    if (kasjerPid == 0) {
        execl("./kasjer", "kasjer", nullptr);
        perror("execl kasjer");
        _exit(1);
    }

    // Uruchom generator zwiedzajacych
    zwiedzajacyPid = fork();
    if (zwiedzajacyPid == 0) {
        execl("./zwiedzajacy", "origin", nullptr);
        perror("execl zwiedzajacy");
        _exit(1);
    }

    // Start zegara w wątku
    std::thread zegar(clockThread, Tp, Tk);
    zegar.join();

    // Czekamy na zakończenie procesów
    if (kasjerPid > 0) waitpid(kasjerPid, nullptr, 0);
    logger.log("Kasjer sie skonczyl");

    logger.log("Przed zwiedzajacymi");
    if (zwiedzajacyPid > 0) waitpid(zwiedzajacyPid, nullptr, 0);
    logger.log("Zwiedzajacy sie skonczyli");
    if (shmPtr) shmdt(shmPtr);
    logger.log("Usunalem pamiec dzielona");
    if (shmId >= 0) shmctl(shmId, IPC_RMID, nullptr);
    if (msgQueue) msgQueue->destroy();

    logger.log("__LOGGER_STOP__");
    return 0;
}
