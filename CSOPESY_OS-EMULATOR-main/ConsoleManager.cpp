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
#include <string>
#include <algorithm>
#include <cctype>


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

bool isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}


bool isValidMemSize(int size) {
    return size >= 64 && size <= 65536 && isPowerOfTwo(size);
} 

//New trim string
std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        ++start;
    }

    auto end = s.end();
    do {
        --end;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}


void ConsoleManager::run() {
    std::string input;
    headerprnt();
    Option1(); 

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
            //new --> Allows memory size input per spec, checks power-of-two & range.
            std::istringstream iss(input.substr(10));
            std::string name; 
            uint16_t memSize;
            if (!(iss >> name >> memSize) || !isValidMemSize(memSize)) {
                std::cout << "Invalid memory allocation.\n";
                continue;
            }
            //new
            screenAttach(name, memSize);
        } else if (input.rfind("screen -r ", 0) == 0) {
            std::string name = input.substr(10);
            screenReattach(name);
        } else if (input == "scheduler-start") {
            startScheduler();
        } else if (input == "scheduler-stop") {
            stopScheduler();
        } else if (input == "report-util") {
            generateReport();
        //new - screen -c
        } else if(input.rfind("screen -c ", 0) == 0){
            //parse: name, memSize, instructions string in quotes
            //Enables screen‑c command per spec with custom instructions.
            std::istringstream iss(input.substr(10));
            std::string name;
            uint16_t memSize;
            std::string instructions;

            iss >> name >> memSize;
            std::getline(iss, instructions);
            instructions = trim(instructions); //remove any leading and trailing spaces
            if (!isValidMemSize(memSize)) {
                std::cout << "Invalid memory allocation.\n";
                return;
            }

            screenCustom(name, memSize, instructions);

        //new - vmstat 
        } else if (input == "vmstat") {
            if(!memoryManager){
                std::cout << "Memory Manager not initialized.\n";
            } else {
                memoryManager->printVMStat();
            }
        // new - process-smi in main menu
        } else if (input == "process-smi"){
            memoryManager->printProcessSMI();
        } else {
            std::cout << "Unknown command.\n";
        }
        Option2();
    }
}

void ConsoleManager::initialize() {
    loadConfig();
    //new
    int instructionSize = 4; //fixed mem ng mga instructions
     memoryManager = std::make_unique<MemoryManager>(
        maxOverallMem, memPerFrame, maxMemPerProc, instructionSize   
    );
    //new
    isInitialized = true;
    std::cout << "System initialized successfully.\n";
}

void ConsoleManager::printConfig() const {
    std::cout << "=== Current Configuration ===\n";
    std::cout << "Number of CPUs: " << numCPU << "\n";
    std::cout << "Quantum Cycles: " << quantumCycles << "\n";
    std::cout << "Batch Process Frequency: " << batchProcessFreq << " seconds\n";
    std::cout << "Instruction Range: " << minInstructions << " - " << maxInstructions << "\n";
    std::cout << "Delay per Execution: " << delayPerExec << " ms\n"; 
    std::cout << "Max Overall Memory: " << maxOverallMem << " Bytes\n";
    std::cout << "Memory per Frame: " << memPerFrame << " Bytes\n";
    std::cout << "Min/Max Memory per Process: " << minMemPerProc 
              << "/" << maxMemPerProc << " Bytes\n";
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
        else if (key == "max-overall-mem") file >> maxOverallMem; 
        else if (key == "mem-per-frame") file >> memPerFrame;
        else if (key == "min-mem-per-proc") file >> minMemPerProc;
        else if (key == "max-mem-per-proc") file >> maxMemPerProc;

        else {
            std::cout << "Unknown config key: " << key << "\n";
        }
    }

    std::cout << "Config loaded: " << numCPU << " CPUs, Scheduler = " << schedulerAlgo
          << ", Quantum = " << quantumCycles
          << ", Min/Max Instructions = " << minInstructions << "/" << maxInstructions
          << ", Delay = " << delayPerExec << "ms\n"
          << "Max Overall Memory: " << maxOverallMem << " Bytes, Memory per Frame: "
          << memPerFrame << " Bytes\n"
          << "Min/Max Memory per Process: " << minMemPerProc << "/" << maxMemPerProc << " Bytes\n";

}

//ALLIYAH 
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
    generating = true;

    // Thread 1: ticking scheduler
    std::thread([this]() {
        while (ticking) {
            scheduler->tick();
            quantumCounter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(cpuCycleTicks));
        }
    }).detach();

    // Thread 2: generating processes
    std::thread([this]() {
        try {
            while (generating) {
                std::this_thread::sleep_for(std::chrono::seconds(batchProcessFreq));
                std::string procName = "p" + std::to_string(currentPID + 1);
                int instCount = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
                createProcess(procName, instCount, false);
                scheduler->addProcess(allProcesses.back());
            }
        } catch (const std::exception& e) {
            std::cerr << "[Error in process generation thread] " << e.what() << std::endl;
        }
    }).detach();

    std::cout<<"Scheduler started\n"; 
}
//ALLIYAH 

/*
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
    generating = true;


    //thick thread
    schedulerThread = std::thread([this](){
        while (ticking){
            scheduler->tick();
            quantumCounter++;  
            std::this_thread::sleep_for(std::chrono::milliseconds(cpuCycleTicks));
        }

    });

    //make processes in the background
    std::thread([this](){

        try {
        while(generating){
            std::this_thread::sleep_for(std::chrono::seconds(batchProcessFreq));
            std::string procName = "p" + std::to_string(currentPID);
            int instCount = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
            createProcess(procName, instCount, false);
            scheduler->addProcess(allProcesses.back());
        }
        } catch (const std::exception& e) {
            std::cerr << "[Error in process generation thread] " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[Unknown error in process generation thread]\n";
        }

    }).detach();

  
    std::cout<<"Scheduler started\n"; 

}
    */

void ConsoleManager::stopScheduler() {

    if (!generating) {
        std::cout << "Process generation is already stopped.\n";
        return;
    }

    std::cout << "Stopping automatic process generation...\n";
    generating = false;

     // new
    ticking = true; // will still run sched algo
     // new

    std::cout << "No new processes will be auto-generated. Scheduler is still running.\n";
}

void ConsoleManager::createProcess(const std::string& name, int instructionCount, bool silent) {
   auto proc = std::make_shared<Process>(++currentPID, name, instructionCount);
    proc->setMemoryManager(memoryManager.get());

    //new generate memory between min/max memperproc
    int memoryBytes = minMemPerProc;
    if(maxMemPerProc > minMemPerProc){
        memoryBytes += rand() % (maxMemPerProc - minMemPerProc + 1);
    }

    proc->setMemoryRequirement(memoryBytes);

    int frameSize = memoryManager->getFrameSize();
    int totalPages = (memoryBytes + frameSize - 1) / frameSize; // division

    memoryManager->registerProcess(proc->getPID(), totalPages);

    if(!memoryManager->allocateMemory(proc)){
            std::cout << "Insufficient memory for process " << name << "\n";
            return; // Don't create process
    }

    // Generate dummy instructions
    std::vector<std::shared_ptr<Instruction>> insts;
    int varCounter = 0;

    for (int i = 0; i < instructionCount; ++i) {

        int opcode = rand() % 6; //include read write as instructions
        std::string varName = "v" + std::to_string(varCounter);

        switch(opcode){
            case 0:
                insts.push_back(std::make_shared<DeclareInstruction>(varName, rand() % 100));
                varCounter++;
                break;
            case 1:
                insts.push_back(std::make_shared<AddInstruction>("x", "x", "1"));
                break;
            case 2:
                insts.push_back(std::make_shared<SubtractInstruction>("x", "x", "1"));
                break;
            case 3:
                insts.push_back(std::make_shared<PrintInstruction>("Instruction executed."));
                break;
            case 4: {
                // Generate a random address within allocated space
                int address = memoryManager->getRandomValidAddress(proc->getPID());

                std::stringstream addrStream;
                addrStream << std::hex << address;
                std::string hexAddr = "0x" + addrStream.str();

                std::string readVar = "r" + std::to_string(varCounter++);
                insts.push_back(std::make_shared<ReadInstruction>(readVar, address, memoryManager.get()));
                break;
            }
            case 5: {
                int address = memoryManager->getRandomValidAddress(proc->getPID());
                uint16_t value = rand() % 65536;

                std::stringstream addStream;
                addStream << std::hex << address;
                std::string hexAddr = "0x" + addStream.str();

                insts.push_back(std::make_shared<WriteInstruction>(hexAddr, std::to_string(value)));
                break;
            }
        }
    }

    proc->setInstructions(insts);

    processTable[name] = proc;
    allProcesses.push_back(proc);

    if(silent){
        std::cout << "Process " << name << " created with " << instructionCount << " instructions.\n";
    }
}

//new - screeen -c custom input
void ConsoleManager::screenCustom(const std::string& name, uint32_t memSize, const std::string& instructionStr) {
    std::vector<std::shared_ptr<Instruction>> insts;

    std::istringstream stream(instructionStr);
    std::string token;

    while (std::getline(stream, token, ';')){
        std::istringstream line(token);
        std::string type;
        line >> type;

        if (type == "READ") {
            std::string var;
            std::string addrStr;
            line >> var >> addrStr;
            int addr = std::stoi(addrStr, nullptr, 16);

            auto readInst = std::make_shared<ReadInstruction>(var, addr, memoryManager.get());
            insts.push_back(readInst);
        } else if (type == "WRITE") {
            std::string addrStr;
            int value;
            line >> addrStr >> value;
            int addr = std::stoi(addrStr, nullptr, 16);

            std::stringstream ss;
            ss << std::hex << addr;
            std::string hexAddr = ss.str();

            auto writeInst = std::make_shared<WriteInstruction>(hexAddr, std::to_string(value));
            insts.push_back(writeInst);
        } else {
            std::cout << "Unknown instruction type: " << type << "\n";
            return;
        }
    }
    
    if (insts.empty() || insts.size() > 50) {
        std::cout << "Invalid number of instructions (1–50 allowed).\n";
        return;
    }
    
    auto proc = std::make_shared<Process>(++currentPID, name, insts.size());
    proc->setInstructions(insts);
    proc->setMemoryRequirement(memSize);

    int frameSize = memoryManager->getFrameSize();
    int totalPages = (memSize + frameSize - 1) / frameSize;
    memoryManager->registerProcess(proc->getPID(), totalPages);

     if (!memoryManager->allocateMemory(proc)) {
        std::cout << "Insufficient memory for process " << name << "\n";
        return;
    }

    processTable[name] = proc;
    allProcesses.push_back(proc);

    if (scheduler) {
        scheduler->addProcess(proc);
    } else {
        std::cout << "Scheduler not running. Process idle.\n";
    }

    processScreen(proc);
}
//new

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
        }
    }

    if (!anyShown) {
        std::cout << "  No active processes.\n";
    }
 
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
                      << ", Memory Requirement: " << proc->getMemoryRequirement() << " bytes"
                      << ")\n";
        }
    }
    if (!anyFinished) {
        std::cout << "  No finished processes yet.\n";
    }
}

//added memSize
// screen -s make process 
void ConsoleManager::screenAttach(const std::string& name, int memSize) {
    // If process does not exist, create it
    if (processTable.count(name) == 0) {
        int instructionCount = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
        createProcess(name, instructionCount, true);

        //new
        if (processTable.find(name) == processTable.end()) {
            // Process creation failed due to memory
            return;
        }
        //new

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
    // access violation check
    // If process has terminated due to access violation, print error message
    auto proc = it->second;
    if (proc->hasTerminatedDueToViolation()) {
    std::cout << "Process " << name 
              << " shut down due to memory access violation error that occurred at "
              << proc->getViolationTime() << ". "
              << proc->getViolationAddress() << " invalid.\n";
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
