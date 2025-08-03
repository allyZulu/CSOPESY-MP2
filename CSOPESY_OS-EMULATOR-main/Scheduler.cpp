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


// void Scheduler::addProcess(std::shared_ptr<Process> process) {
//     // process->setState(Process::READY);
//     //new
//     int result = memoryManager->allocateMemory(process);
//     if (result == 0) {
//         process->setMemoryManager(memoryManager); //added
//         process->setState(Process::READY);
//         readyQueue.push(process);
//     } else {
//     //   std::cerr << "Memory allocation failed for PID " << process->getPID() << "\n";
//     }
//     //new
// }


//change to this
void Scheduler::addProcess(std::shared_ptr<Process> process) {
    // Link MemoryManager to process
    process->setMemoryManager(memoryManager);

    bool result = memoryManager->allocateMemory(process);
    if (result) { // FIXED: check correctly
        process->setState(Process::READY);
        readyQueue.push(process);
    } else {
        std::cerr << "Memory allocation failed for PID " << process->getPID() << "\n";
    }
}

void Scheduler::tick() {
    if (!isRunning) return;

    //new
    if (readyQueue.empty() && isIdle()) {
        return; 
    }


    assignProcessesToCores();
    executeProcesses();
}

// void Scheduler::assignProcessesToCores() {
//     for (int i = 0; i < numCores; ++i) {
//         auto& core = cores[i];

//         // If the core has a process and finished, deallocate memory
//         if (core.currentProcess) {
//             if (core.currentProcess->isFinished()) {
//                 memoryManager->deallocateMemory(core.currentProcess);
//                 core.currentProcess->setState(Process::FINISHED);
//                 core.currentProcess = nullptr;
//             }
//         }

//         // If the core  empty assign a process 
//         if (!core.currentProcess) {
//             int queueSize = readyQueue.size(); // avoid infinite loop
//             for (int attempt = 0; attempt < queueSize; ++attempt) {
//                 auto nextProcess = readyQueue.front();
//                 readyQueue.pop();

//                 // Memory is already allocated in demand paging — proceed directly
//                 nextProcess->setCoreID(i);
//                 nextProcess->setState(Process::RUNNING);
//                 core.currentProcess = nextProcess;
//                 core.remainingQuantum = quantum;
//                 break;
//             }
//         }
//     }
// }
 

void Scheduler::assignProcessesToCores() {
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


// void Scheduler::executeProcesses() {
//     for (int i = 0; i < numCores; ++i) {
//         auto& core = cores[i];
//         if (core.currentProcess && !core.currentProcess->isFinished()) {
//             auto pid = core.currentProcess->getPID();
//             auto instr = core.currentProcess->getCurrentInstruction();

//             // Demand paging: ensure page is loaded
//             if (instr && !memoryManager->ensurePageLoaded(pid, instr->virtualAddress)) {
//                 std::cerr << "Page fault: PID " << pid << " could not load page for address "
//                           << instr->virtualAddress << "\n";
//                 continue;
//             }

//             //new
//             if (instr && !memoryManager->ensurePageLoaded(pid, instr->virtualAddress)) {
//             std::cerr << "Page fault: PID " << pid << " could not load page for address "
//                     << instr->virtualAddress << "\n";
//             // Return process to READY instead of leaving it hanging
//             core.currentProcess->setState(Process::READY);
//             readyQueue.push(core.currentProcess);
//             core.currentProcess = nullptr;
//             continue;
//             }


//             core.currentProcess->executeNextInstruction(i);

//             if(delayPerExec > 0){
//                 volatile uint64_t busy = 0;
//                 for (int j = 0; j < delayPerExec; ++j){
//                     busy += j;
//                 }

//             }

//             if (schedulingAlgorithm == "rr") {
//                 core.remainingQuantum--;

//                 if (core.remainingQuantum <= 0 && !core.currentProcess->isFinished()) {
//                     // Preempt and requeue
//                     core.currentProcess->setState(Process::READY);
//                     readyQueue.push(core.currentProcess);
//                     core.currentProcess = nullptr;
//                 }
//             }
//         }
//     }
// }

//new function for executeProcess
void Scheduler::executeProcesses() {
    for (int i = 0; i < numCores; ++i) {
        auto& core = cores[i];

        if (core.currentProcess && !core.currentProcess->isFinished()) {
            auto pid = core.currentProcess->getPID();
            auto instr = core.currentProcess->getCurrentInstruction();

            // if (instr) {
            //     bool loaded = memoryManager->ensurePageLoaded(pid, instr->virtualAddress);
            //     if (!loaded) {
            //         std::cerr << "Page fault: PID " << pid << " page not loaded, moving to WAITING\n";

            //         core.currentProcess->setState(Process::WAITING);
            //         readyQueue.push(core.currentProcess); // Requeue for later retry
            //         core.currentProcess = nullptr;
            //         continue; // Skip execution this tick
            //     }
            // }

            if (instr) {
                if (!memoryManager->isAddressValid(pid, instr->virtualAddress)) {
                    std::cerr << "[Memory Violation] PID " << pid << " tried invalid address "
                            << instr->virtualAddress << "\n";
                    core.currentProcess->setTerminatedDueToViolation("Invalid Address", 
                                                                    std::to_string(instr->virtualAddress));
                    core.currentProcess->setState(Process::FINISHED);
                    continue;
                }

                if (!memoryManager->ensurePageLoaded(pid, instr->virtualAddress)) {
                    std::cerr << "[Page Fault] PID " << pid << " loading page for address "
                            << instr->virtualAddress << "\n";
                    continue; // Skip this tick but avoid infinite loop
                }
            }


            // Execute instruction after page is loaded
            core.currentProcess->executeNextInstruction(i);

            if (delayPerExec > 0) {
                volatile uint64_t busy = 0;
                for (int j = 0; j < delayPerExec; ++j) {
                    busy += j;
                }
            }

            if (schedulingAlgorithm == "rr") {
                core.remainingQuantum--;
                if (core.remainingQuantum <= 0 && !core.currentProcess->isFinished()) {
                    core.currentProcess->setState(Process::READY);
                    readyQueue.push(core.currentProcess);
                    core.currentProcess = nullptr;
                }
            }
        }
    }
}


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
