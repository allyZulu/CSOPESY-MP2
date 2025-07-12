#include "ConsoleManager.h"
#include "Instruction.h"
#include "InstructionsTypes.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <random>


const int cpuCycleTicks = 100; //constant ticks ng CPU
ConsoleManager* ConsoleManager::instance = nullptr;

ConsoleManager* ConsoleManager::getInstance() {
    if (!instance) {
        instance = new ConsoleManager();
    }
    return instance;
}

ConsoleManager::ConsoleManager() {}

void headerprnt() {
    std::cout << "================================================================================" << std::endl;
    std::cout << " ____   ______   ___   ______   _____   _____  __  __     _   " << std::endl;
    std::cout << "/ ___| /   ___| / _ \\  |  _  \\ |  ___| /   ___  \\ \\ / / " << std::endl;
    std::cout << "| |    | |__   | | | | | |_|  ||  |__  | |__       \\ V /   " << std::endl;
    std::cout << "| |     \\___ \\ | | | | |   _/  |   __|  \\___    \\| |  " << std::endl;
    std::cout << "| |__  ____| | | |_| | |  |    |  |__   ____| |      | |  " << std::endl;
    std::cout << "\\____||______/ \\___/   |__|    |_____| |______/      |_| " << std::endl;
    std::cout << "Hello, Welcome to CSOPESY OS-Emulator!" << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << "Developers: " << std::endl;
    std::cout << "- Keira Gabrielle C. Alcantara" << std::endl;
    std::cout << "- Charlize Kirsten M. Brodeth" << std::endl;
    std::cout << "- Candice Aura T. Fernandez" << std::endl;
    std::cout << "- Alliyah Gaberielle D. Zulueta" << std::endl;
    std::cout << "================================================================================" << std::endl;
}

void Option1() {
    std::cout << "\nOptions:" << std::endl;
    std::cout << "- initialize" << std::endl;
    std::cout << "- exit" << std::endl;
}

void Option2() {
    std::cout << "\nOptions:" << std::endl;
    std::cout << "- screen -ls" << std::endl;
    std::cout << "- screen -s [process name]" << std::endl;
    std::cout << "- screen -r [process name]" << std::endl;
    std::cout << "- scheduler-start" << std::endl;
    std::cout << "- scheduler-stop" << std::endl;
    std::cout << "- report-util" << std::endl;
    std::cout << "- exit" << std::endl;
}

void ConsoleManager::run() {
    std::string input;
    headerprnt();
    Option1(); 
 //   std::cout << "Welcome to the OS Emulator Shell\n";

    while (true) {
        std::cout << "\nRoot:\\> ";
        std::getline(std::cin, input);

        if (input == "exit") break;

        if (!isInitialized && input != "initialize") {
            std::cout << "Please initialize the system first using `initialize` command.\n";
            continue;
        }

        if (input == "initialize") {
            initialize();
        } else if (input == "screen -ls") {
            listScreens();
        } else if (input.rfind("screen -s ", 0) == 0) {
            std::string name = input.substr(10);
            screenAttach(name);
        } else if (input.rfind("screen -r ", 0) == 0) {
            std::string name = input.substr(10);
            screenReattach(name);
        } else if (input == "scheduler-start") {
            startScheduler();
        } else if (input == "scheduler-stop") {
            stopScheduler();
        } else if (input == "report-util") {
            generateReport();
        } else {
            std::cout << "Unknown command.\n";
        }
        Option2();
    }
}

void ConsoleManager::initialize() {
    loadConfig();
    memoryManager = std::make_unique<MemoryManager>(16384, 16, 4096);
    isInitialized = true;
    std::cout << "System initialized successfully.\n";
}

void ConsoleManager::printConfig() const {
    std::cout << "=== Current Configuration ===\n";
    std::cout << "Number of CPUs: " << numCPU << "\n";
    std::cout << "Quantum Cycles: " << quantumCycles << "\n";
    std::cout << "Batch Process Frequency: " << batchProcessFreq << "seconds\n";
    std::cout << "Instruction Range: " << minInstructions << " - " << maxInstructions << "\n";
    std::cout << "Delay per Execution: " << delayPerExec << "ms\n"; 
}

void ConsoleManager::loadConfig() {
    std::ifstream file("config.txt");
    if (!file.is_open()) {
        std::cout << "Failed to open config.txt. Using defaults.\n";
        return;
    }

    std::string key;
    while (file >> key) {
        if (key == "num-cpu") file >> numCPU;
        else if (key == "scheduler") file >> schedulerAlgo;
        else if (key == "quantum-cycles") file >> quantumCycles;
        else if (key == "batch-process-freq") file >> batchProcessFreq;
        else if (key == "min-ins") file >> minInstructions;
        else if (key == "max-ins") file >> maxInstructions;
        else if (key == "delay-per-exec") file >> delayPerExec;
    }

    std::cout << "Config loaded: " << numCPU << " CPUs, Scheduler = " << schedulerAlgo
              << ", Quantum = " << quantumCycles << ", Min/Max Instructions = "
              << minInstructions << "/" << maxInstructions << ", Delay = " << delayPerExec << "\n";
}

void ConsoleManager::startScheduler() {
    std::cout << "Starting process generation...\n";

    scheduler = std::make_unique<Scheduler>(numCPU, schedulerAlgo, quantumCycles, delayPerExec, memoryManager.get());

    //scheduler = std::make_unique<Scheduler>(numCPU, schedulerAlgo, quantumCycles);
    // For simulation
    for (int i = 0; i < batchProcessFreq; ++i) {
        std::string procName = "p" + std::to_string(currentPID + 1);

        int instCount = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
        createProcess(procName, instCount, true);
        scheduler->addProcess(allProcesses.back());
    }

    //start ticking
    ticking = true;

    //thick thread
    schedulerThread = std::thread([this](){
        while (ticking){
            scheduler->tick();
            quantumCounter++;
            memoryManager->snapshot(quantumCounter);
            std::this_thread::sleep_for(std::chrono::milliseconds(cpuCycleTicks));
        }

    });

    //make processes in the background
    std::thread([this](){
        while(ticking){
            std::this_thread::sleep_for(std::chrono::seconds(batchProcessFreq));

            // Generate a new dummy process
            std::string procName = "p" + std::to_string(++currentPID);
            int instCount = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
            createProcess(procName, instCount, false);
            scheduler->addProcess(allProcesses.back());

            //std::cout << "Auto-created process: " << procName << " with " << instCount << " instructions\n";
        }

    }).detach();

  
    std::cout<<"Scheduler started\n"; 

}

void ConsoleManager::stopScheduler() {
    if (!ticking) {
        std::cout << "Scheduler is not running.\n";
        return;
    }

    std::cout << "Stopping scheduler...\n";
    ticking = false;

    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }

    std::cout << "Scheduler stopped.\n";
}

void ConsoleManager::createProcess(const std::string& name, int instructionCount, bool silent) {
   auto proc = std::make_shared<Process>(++currentPID, name, instructionCount);

    // Generate dummy instructions
    std::vector<std::shared_ptr<Instruction>> insts;
    for (int i = 0; i < instructionCount; ++i) {
        if (i % 4 == 0)
            insts.push_back(std::make_shared<DeclareInstruction>("x", i));
        else if (i % 4 == 1)
            insts.push_back(std::make_shared<AddInstruction>("x", "x", "1"));
        else if (i % 4 == 2)
            insts.push_back(std::make_shared<SubtractInstruction>("x", "x", "1"));
        else
            insts.push_back(std::make_shared<PrintInstruction>("Instruction executed."));
    }

    proc->setInstructions(insts);

    processTable[name] = proc;
    allProcesses.push_back(proc);
    if(silent){
        std::cout << "Process " << name << " created with " << instructionCount << " instructions.\n";
    }
}

//screen -ls (show ongoing and finished processes)
void ConsoleManager::listScreens() {
    std::cout << "=== CPU Utilization Summary ===\n";

    int totalCores = numCPU;
    int usedCores = scheduler ? (totalCores - scheduler->getAvailableCores()) : 0;
    int availableCores = scheduler ? scheduler->getAvailableCores() : totalCores;

    std::cout << "Total Cores: " << totalCores << "\n";
    std::cout << "Used Cores: " << usedCores << "\n";
    std::cout << "Available Cores: " << availableCores << "\n\n";


    std::cout << "=== Currently RUNNING/READY/WAITING processes ===\n";
    bool anyShown = false;
    for (auto& pair : processTable) {
        auto proc = pair.second;
        if (!proc->isFinished()) {
            anyShown = true;
            std::string stateStr;
            switch (proc->getState()) {
                case Process::READY: stateStr = "READY"; break;
                case Process::RUNNING: stateStr = "RUNNING"; break;
                case Process::WAITING: stateStr = "WAITING"; break;
                case Process::FINISHED: stateStr = "FINISHED"; break;
            }
            std::cout << "  " << proc->getName()
                      << " (PID: " << proc->getPID()
                      << ", Core: " << proc->getCoreID()
                      << ", State: " << stateStr << ", Progress: "
                      << proc->getCommandCounter() << "/" << proc->getLinesOfCode()
                      << ")\n";
            // std::cout << "  " << proc->getName() << " (PID: " << proc->getPID()
            //           << ", State: " << stateStr << ")\n";
        }
    }

    if (!anyShown) {
        std::cout << "  No active processes.\n";
    }
    // for (auto& pair : processTable) {
    //     if (!pair.second->isFinished()) {
    //         std::cout << "  " << pair.first << "\n";
    //     }
    // }

    std::cout << "\n=== Finished Processes ===\n";
    bool anyFinished = false;
    for (const auto& pair : processTable) {
        auto proc = pair.second;
        if (proc->isFinished()) {
            anyFinished = true;
            std::cout << "  " << proc->getName()
                      << " (PID: " << proc->getPID()
                      << ", Core: " << proc->getCoreID()
                      << ", Finished at: " << proc->getFinishTimeString()
                      << ", Total Instructions: " << proc->getLinesOfCode()
                      << ")\n";
        }
    }
    if (!anyFinished) {
        std::cout << "  No finished processes yet.\n";
    }
}

// screen -s make process 
void ConsoleManager::screenAttach(const std::string& name) {
    // If process does not exist, create it
    if (processTable.count(name) == 0) {
        int instructionCount = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
        createProcess(name, instructionCount, true);

        if (scheduler) {
            scheduler->addProcess(processTable[name]);
        } else {
            std::cout << "Scheduler not started yet. Process will be idle until scheduler starts.\n";
        }
    }

    processScreen(processTable[name]);
}


void ConsoleManager::screenReattach(const std::string& name) {
    auto it = processTable.find(name);
    if(it == processTable.end()){
        std::cout << "Process \"" << name << "\" does not exist.\n";
        return;
    }

    processScreen(it->second);
}

void ConsoleManager::processScreen(std::shared_ptr<Process> process) {
    std::string input;
    while (true) {
        std::cout << "[screen:" << process->getName() << "] > ";
        std::getline(std::cin, input);

        if (input == "exit") break;
        else if (input == "process-smi") {
            std::cout << "Name: " << process->getName() << "\n";
            std::cout << "PID: " << process->getPID() << "\n";
            std::cout << "Progress: " << process->getCommandCounter() << " / " << process->getLinesOfCode() << "\n";
            std::cout << "Core ID: " << process->getCoreID() << "\n";
            std::cout << "Logs: " << process->getOutput() << "\n";
            if (process->isFinished()) {
                std::cout << "Finished at: " << process->getFinishTimeString() << "\n";
            }
        } else {
            std::cout << "Unknown screen command.\n";
        }
    }
}

void ConsoleManager::generateReport() {
    std::ofstream outFile("csopesy-log.txt");
    if (!outFile.is_open()) {
        std::cout << "Failed to open report file.\n";
        return;
    }

    outFile << "=== CPU Utilization Report ===\n";
    for (auto& proc : allProcesses) {
        outFile << "Process: " << proc->getName()
                << " PID: " << proc->getPID()
                << " Progress: " << proc->getCommandCounter() << " / " << proc->getLinesOfCode();
        if(proc->isFinished()){
            outFile << " [" << proc->getFinishTimeString() << "]";
        }
        outFile <<"\n";
            }
    outFile.close();
    std::cout << "Report saved to csopesy-log.txt.\n";
}

int ConsoleManager::getCurrentPID() const {
    return currentPID;
}
