#include "Process.h"
#include <iostream>
#include <iomanip>
#include <sstream>

Process::Process(int pid, const std::string& name, int lines)
    : pid(pid), name(name), commandCounter(0), linesOfCode(lines),
      coreID(-1), currentState(READY), sleeping(false), sleepTicks(0) {}

/*
void Process::executeNextInstruction(int coreID) {
    if (isFinished()) return;

    this->coreID = coreID;

    if (sleeping) {
        if (--sleepTicks <= 0) sleeping = false;
        return;
    }
    //new
    // calculates which page the current instruction belongs to
    int currentPage = commandCounter / memoryManager->getInstructionsPerPage();

    // // Checks if the required page for this instruction is loaded 
    // if (!memoryManager->ensurePageLoaded(pid, currentPage)) {
    //     std::cout << "[Page Fault] Process " << pid << " triggered loading of page "
    //               << currentPage << "\n";
    // }

    memoryManager->ensurePageLoaded(pid, currentPage);

    // updates the Least Recently Used (LRU) tracking for that page
    memoryManager->accessPage(pid, currentPage); // Mark page access for LRU
    // new
    if (commandCounter < linesOfCode && commandCounter < instructions.size()) {

        instructions[commandCounter]->execute(
            name,
            coreID,
            variables,
            outputLog,
            sleeping,
            sleepTicks,
            memoryManager 
        );
        commandCounter++;
    }

    if (commandCounter >= linesOfCode) {
        currentState = FINISHED;
        markFinished();
    }
    // NEW: check if it was terminated due to invalid memory
    if (sleeping && outputLog.find("Memory violation") != std::string::npos) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_time);

        std::ostringstream oss;
        oss << std::put_time(&local_tm, "%H:%M:%S");

        // Extract invalid address from the log (or pass it from instruction)
        std::string address = "???"; // optional parse from log
        setTerminatedDueToViolation(oss.str(), address);
    }
}
*/   
//ALLIYAH   
void Process::executeNextInstruction(int coreID) {
    if (isFinished()) return;

    this->coreID = coreID;

    if (sleeping) {
        if (--sleepTicks <= 0) sleeping = false;
        return;
    }

    int currentPage = commandCounter / memoryManager->getInstructionsPerPage();

    // Ensure page is loaded
    if (!memoryManager->ensurePageLoaded(pid, currentPage)) {
        outputLog += "[Page Fault] Could not load page " + std::to_string(currentPage) + "\n";
        sleeping = true;
        return;
    }

    // Update LRU
    memoryManager->accessPage(pid, currentPage);

    if (commandCounter < linesOfCode && commandCounter < instructions.size()) {
        try {
            instructions[commandCounter]->execute(
                name,
                coreID,
                variables,
                outputLog,
                sleeping,
                sleepTicks,
                memoryManager 
            );
        } catch (const std::exception& e) {
            outputLog += "[Error] " + std::string(e.what()) + "\n";
            sleeping = true;
            return;
        }
        commandCounter++;
    }

    if (commandCounter >= linesOfCode) {
        currentState = FINISHED;
        markFinished();
    }

    //check if terminated due to memory violation (based on log)
    if (sleeping && outputLog.find("Memory violation") != std::string::npos) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_time);

        std::ostringstream oss;
        oss << std::put_time(&local_tm, "%H:%M:%S");
        std::string timestamp = oss.str();
       
        std::string address = "???"; // optional parse from log
        setTerminatedDueToViolation(oss.str(), address);

       //  setTerminatedDueToViolation(timestamp, "???");
    }
}
//ALLIYAH 

bool Process::isFinished() const {
    return currentState == FINISHED;
}

std::string Process::getName() const {
    return name;
}

int Process::getPID() const {
    return pid;
}

int Process::getCommandCounter() const {
    return commandCounter;
}

int Process::getLinesOfCode() const {
    return linesOfCode;
}

int Process::getCoreID() const {
    return coreID;
}

Process::ProcessState Process::getState() const {
    return currentState;
}

std::string Process::getOutput() const {
    return outputLog;
}

void Process::setCoreID(int coreID) {
    this->coreID = coreID;
}

void Process::setState(ProcessState newState) {
    currentState = newState;
}

void Process::setInstructions(const std::vector<std::shared_ptr<Instruction>>& insts) {
    instructions = insts;
}


void Process::setMemoryManager(MemoryManager* memManager) {
   memoryManager = std::shared_ptr<MemoryManager>(memManager);
}
void Process::markFinished() {
    if (!hasFinishTime) {
        finishTime = std::chrono::system_clock::now();
        hasFinishTime = true;
    }
}

std::string Process::getFinishTimeString() const {
    if (!hasFinishTime) return "N/A";
    std::time_t finish_time = std::chrono::system_clock::to_time_t(finishTime);
    std::tm local_tm = *std::localtime(&finish_time);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%H:%M:%S %m/%d/%Y");
    return oss.str();
}

Instruction* Process::getCurrentInstruction() const {
    if (commandCounter < instructions.size()) {
        return instructions[commandCounter].get();  // Return raw pointer from shared_ptr
    }
    return nullptr;
}

//new
int Process::getCurrentPageNumber() const {
    const int instructionsPerPage = memoryManager->getInstructionsPerPage(); // or define it as constant
    return commandCounter / instructionsPerPage;
}


const std::vector<std::shared_ptr<Instruction>>& Process::getInstructions() const {
    return instructions;
}

void Process::setMemoryRequirement(int bytes) {
    memoryRequirementBytes = bytes;
}

int Process::getMemoryRequirement() const {
    return memoryRequirementBytes;
}

//new

// access violation
void Process::setTerminatedDueToViolation(const std::string& time, const std::string& address) {
    terminatedDueToViolation = true;
    violationTime = time;
    violationAddress = address;
    currentState = FINISHED;
    markFinished();
}

bool Process::hasTerminatedDueToViolation() const {
    return terminatedDueToViolation;
}

std::string Process::getViolationTime() const {
    return violationTime;
}

std::string Process::getViolationAddress() const {
    return violationAddress;
}
