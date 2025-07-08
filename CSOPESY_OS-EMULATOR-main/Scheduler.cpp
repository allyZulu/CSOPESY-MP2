#include "Scheduler.h"
#include <iostream>
#include <cstdint>

Scheduler::Scheduler(int numCores, const std::string& algorithm, int quantum, int delay)
    : numCores(numCores), schedulingAlgorithm(algorithm), quantum(quantum), isRunning(true), delayPerExec(delay){
    cores.resize(numCores);
}

void Scheduler::addProcess(std::shared_ptr<Process> process) {
    process->setState(Process::READY);
    readyQueue.push(process);
}

void Scheduler::tick() {
    if (!isRunning) return;
    assignProcessesToCores();
    executeProcesses();
}

void Scheduler::assignProcessesToCores() {
    for (int i = 0; i < numCores; ++i) {
        auto& core = cores[i];
        if (!core.currentProcess || core.currentProcess->isFinished()) {
            if (core.currentProcess && core.currentProcess->isFinished()) {
                core.currentProcess->setState(Process::FINISHED);
                core.currentProcess = nullptr;
            }

            if (!readyQueue.empty()) {
                auto nextProcess = readyQueue.front();
                readyQueue.pop();
                nextProcess->setCoreID(i);
                nextProcess->setState(Process::RUNNING);
                core.currentProcess = nextProcess;
                core.remainingQuantum = quantum;
            }
        }
    }
}

void Scheduler::executeProcesses() {
    for (int i = 0; i < numCores; ++i) {
        auto& core = cores[i];
        if (core.currentProcess && !core.currentProcess->isFinished()) {
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
