#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "Process.h"
#include "Scheduler.h"
#include "MemoryManager.h"
#include <thread>
#include <atomic>

class ConsoleManager {
public:
    static ConsoleManager* getInstance();

    void run(); // Starts the main menu CLI loop
    void initialize(); // Loads config and initializes the scheduler
    void startScheduler();
    void stopScheduler();
    void createProcess(const std::string& name, int instructionCount, bool silent);
    void listScreens(); // screen -ls
    void screenAttach(const std::string& name, int memSize); // screen -s <name>
    void screenReattach(const std::string& name); // screen -r <name>
    void generateReport(); // report-util
    int getCurrentPID() const;
    void printConfig() const;
    //new
    void screenCustom(const std::string& name, uint16_t memSize, const std::string& instructionStr);
    void createProcessforScreen(const std::string& name, int instructionCount, bool silent, int memSize);
    //new

private:
    ConsoleManager();
    static ConsoleManager* instance;
    bool isInitialized = false;
    int currentPID = 0;

    //adsded
    std::unique_ptr<Scheduler> scheduler;
    std::thread schedulerThread;
    std::atomic<bool> ticking{false};
    std::atomic<bool> generating{false};
    //added

    std::unordered_map<std::string, std::shared_ptr<Process>> processTable;
    std::vector<std::shared_ptr<Process>> allProcesses;

    std::unique_ptr<MemoryManager> memoryManager;
    int quantumCounter = 0;

    void processScreen(std::shared_ptr<Process> process);
    void loadConfig();

    int numCPU = 1;
    std::string schedulerAlgo = "fcfs";
    int quantumCycles = 3;
    int batchProcessFreq = 1;
    int minInstructions = 5;
    int maxInstructions = 10;
    int delayPerExec = 0;
    int maxOverallMem = 1024;   
    int memPerFrame = 64; 
    int minMemPerProc = 64;     // default 
    int maxMemPerProc = 65536;  // default  

};