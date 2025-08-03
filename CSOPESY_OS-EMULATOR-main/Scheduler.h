#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <string>
#include <unordered_map>
#include "Process.h"
#include "MemoryManager.h"

class Scheduler {
public:
    
    // int maxMemory = 16384, int frameSize = 16, int memoryPerProcess = 4096);
    Scheduler(int numCores, const std::string& algorithm, int quantum = 1, int delay = 0,
          MemoryManager* memoryManager = nullptr);

    void addProcess(std::shared_ptr<Process> process);
    void tick(); // Simulates one CPU cycle
    void stop(); // Stops the scheduler loop
    void resume(); // Resumes the scheduler loop

    std::string getAlgorithm() const;
    int getAvailableCores() const;

    //new
    bool allProcessesFinished() const;
    bool isIdle() const;


private:
    int numCores;
    std::string schedulingAlgorithm; // "fcfs" or "rr"
    int quantum;
    int delayPerExec = 0;

    struct Core {
        std::shared_ptr<Process> currentProcess = nullptr;
        int remainingQuantum = 0;
    };

    std::vector<Core> cores;
    std::queue<std::shared_ptr<Process>> readyQueue;

    std::shared_ptr<MemoryManager> memoryManager;
    int currentQuantumCycle = 0;

    bool isRunning;

    void assignProcessesToCores();
    void executeProcesses();
};
