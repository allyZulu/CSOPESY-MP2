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


void Scheduler::addProcess(std::shared_ptr<Process> process) {
    // process->setState(Process::READY);
    //new
    int result = memoryManager->allocateMemory(process);
    if (result == 0) {
        process->setState(Process::READY);
        readyQueue.push(process);
    } else {
    //   std::cerr << "Memory allocation failed for PID " << process->getPID() << "\n";
    }
    //new
}

void Scheduler::tick() {
    if (!isRunning) return;
    assignProcessesToCores();
    executeProcesses();
}

void Scheduler::assignProcessesToCores() {
    for (int i = 0; i < numCores; ++i) {
        auto& core = cores[i];

        // If the core has a process and finished, deallocate memory
        if (core.currentProcess) {
            if (core.currentProcess->isFinished()) {
                memoryManager->deallocateMemory(core.currentProcess);
                core.currentProcess->setState(Process::FINISHED);
                core.currentProcess = nullptr;
            }
        }

        // If the core  empty assign a process 
        if (!core.currentProcess) {
            int queueSize = readyQueue.size(); // avoid infinite loop
            for (int attempt = 0; attempt < queueSize; ++attempt) {
                auto nextProcess = readyQueue.front();
                readyQueue.pop();

                // Memory is already allocated in demand paging — proceed directly
                nextProcess->setCoreID(i);
                nextProcess->setState(Process::RUNNING);
                core.currentProcess = nextProcess;
                core.remainingQuantum = quantum;
                break;
            }
        }
    }
}
 

void Scheduler::executeProcesses() {
    for (int i = 0; i < numCores; ++i) {
        auto& core = cores[i];
        if (core.currentProcess && !core.currentProcess->isFinished()) {
            auto pid = core.currentProcess->getPID();
            auto instr = core.currentProcess->getCurrentInstruction();

            // Demand paging: ensure page is loaded
            if (instr && !memoryManager->ensurePageLoaded(pid, instr->virtualAddress)) {
                std::cerr << "Page fault: PID " << pid << " could not load page for address "
                          << instr->virtualAddress << "\n";
                continue;
            }

            core.currentProcess->executeNextInstruction(i);

            if(delayPerExec > 0){
                volatile uint64_t busy = 0;
                for (int j = 0; j < delayPerExec; ++j){
                    busy += j;
                }

            }

            if (schedulingAlgorithm == "rr") {
                core.remainingQuantum--;

                if (core.remainingQuantum <= 0 && !core.currentProcess->isFinished()) {
                    // Preempt and requeue
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
