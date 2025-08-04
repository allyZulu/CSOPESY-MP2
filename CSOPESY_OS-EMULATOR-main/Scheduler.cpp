#include "Scheduler.h"
#include <iostream>
#include <cstdint>


Scheduler::Scheduler(int numCores, const std::string& algorithm, int quantum, int delay, 
                     MemoryManager* memoryManagerPtr)
    : numCores(numCores), schedulingAlgorithm(algorithm), quantum(quantum), isRunning(true), delayPerExec(delay)
{
    cores.resize(numCores);
    memoryManager = std::shared_ptr<MemoryManager>(memoryManagerPtr);
}

//ALLY
void Scheduler::addProcess(std::shared_ptr<Process> process) {
    // Link MemoryManager to process
    process->setMemoryManager(memoryManager);

    // Calculate total pages based on memory requirement
    int frameSize = memoryManager->getFrameSize();
    int totalPages = (process->getMemoryRequirement() + frameSize - 1) / frameSize;

    // Register in backing store — no immediate allocation
    memoryManager->registerProcess(process->getPID(), totalPages);

    // Mark ready and push into queue
    process->setState(Process::READY);
    readyQueue.push(process);
}

//void Scheduler::addProcess(std::shared_ptr<Process> process) {

    
    //Link MemoryManager to process
  //  process->setMemoryManager(memoryManager);

  //  bool result = memoryManager->allocateMemory(process);
//    if (result) { // FIXED: check correctly
 //      process->setState(Process::READY);
 //       readyQueue.push(process);
//    } //else {
    //     std::cerr << "Memory allocation failed for PID " << process->getPID() << "\n";
    // }
//}

void Scheduler::tick() {
    if (!isRunning) return;

    //new
    // if (readyQueue.empty() && isIdle()) {
    //     return; 
    // }

    assignProcessesToCores();
    executeProcesses();
}

 

void Scheduler::assignProcessesToCores() {
    //for debug
    // std::cout << "Ready Queue Size: " << readyQueue.size() << std::endl;

    for (int i = 0; i < numCores; ++i) {
        auto& core = cores[i];

        if (core.currentProcess) {
            if (core.currentProcess->isFinished()) {
                memoryManager->deallocateMemory(core.currentProcess);
                core.currentProcess->setState(Process::FINISHED);
                core.currentProcess = nullptr;
            }
        }

        if (!core.currentProcess && !readyQueue.empty()) {
            auto nextProcess = readyQueue.front();
            readyQueue.pop();

            nextProcess->setCoreID(i);
            nextProcess->setState(Process::RUNNING); // Reset to RUNNING
            core.currentProcess = nextProcess;
            core.remainingQuantum = quantum;
        }
    }
}


//ALLY 
bool MemoryManager::isPageLoaded(int pid, int pageNumber) const {
    auto it = pageTables.find(pid);
    if (it == pageTables.end()) return false;
    auto pteIt = it->second.find(pageNumber);
    if (pteIt == it->second.end()) return false;
    return pteIt->second.valid;
}

//new function for executeProcess
void Scheduler::executeProcesses() {
    for (int i = 0; i < numCores; ++i) {
        auto& core = cores[i];

        if (core.currentProcess && !core.currentProcess->isFinished()) {
            int pid = core.currentProcess->getPID();

            //Ensure symbol table page (page 0) is loaded
            // Only try loading if not already valid

            if (!memoryManager->isPageLoaded(pid, 0)) {
                if (!memoryManager->ensurePageLoaded(pid, 0)) {
                    std::cerr << "[Page Fault] PID " << pid
                            << " loading symbol table page 0\n";
                    continue;
                }
            }


            //Ensure current instruction page is loaded
            int instrPage = memoryManager->getInstructionPageNumber(
                core.currentProcess->getProgramCounter()
            );

            if (!memoryManager->ensurePageLoaded(pid, instrPage)) {
                std::cerr << "[Page Fault] PID " << pid
                          << " loading instruction page " << instrPage << "\n";
                continue; // wait for page fault resolution
            }

            // Memory access violation check (if instruction uses memory)
            auto instr = core.currentProcess->getCurrentInstruction();
            if (instr && instr->virtualAddress >= 0) {
                if (!memoryManager->isAddressValid(pid, instr->virtualAddress)) {
                    std::cerr << "[Memory Violation] PID " << pid
                              << " tried invalid address "
                              << instr->virtualAddress << "\n";
                    core.currentProcess->setTerminatedDueToViolation(
                        "Invalid Address",
                        std::to_string(instr->virtualAddress)
                    );
                    core.currentProcess->setState(Process::FINISHED);
                    core.currentProcess = nullptr;
                    continue;
                }
            }
            //Execute instruction
            core.currentProcess->executeNextInstruction(i);

            // Round-robin quantum handling
            if (schedulingAlgorithm == "rr") {
                core.remainingQuantum--;
                if (core.remainingQuantum <= 0 && !core.currentProcess->isFinished()) {
                    core.currentProcess->setState(Process::READY);
                    readyQueue.push(core.currentProcess);
                    core.currentProcess = nullptr;
                }
            }
            // ✅ If process finishes, clean up
            if (core.currentProcess && core.currentProcess->isFinished()) {
                memoryManager->deallocateMemory(core.currentProcess);
                core.currentProcess->setState(Process::FINISHED);
                core.currentProcess = nullptr;
            }

        }
    }
}
//ALLY 

void Scheduler::stop() {
    isRunning = false;
}

void Scheduler::resume() {
    isRunning = true;
}

std::string Scheduler::getAlgorithm() const {
    return schedulingAlgorithm;
}

int Scheduler::getAvailableCores() const {
    int count = 0;
    for (const auto& core : cores) {
        if (!core.currentProcess || core.currentProcess->isFinished()) {
            count++;
        }
    }
    return count;
}


//new functions
bool Scheduler::allProcessesFinished() const {
    if (!readyQueue.empty()) return false;
    for (const auto& core : cores) {
        if (core.currentProcess && !core.currentProcess->isFinished()) return false;
    }
    return true;
}

bool Scheduler::isIdle() const {
    if (!readyQueue.empty()) return false;
    for (const auto& core : cores) {
        if (core.currentProcess) return false;
    }
    return true;
}

int MemoryManager::getInstructionPageNumber(int programCounter) {
    return (programCounter / instructionsPerPage) + 1;  // +1 because page 0 is symbol table
}