#include "Process.h"
#include <iostream>
#include <iomanip>
#include <sstream> 

Process::Process(int pid, const std::string& name, int lines)
    : pid(pid), name(name), commandCounter(0), linesOfCode(lines),
      coreID(-1), currentState(READY), sleeping(false), sleepTicks(0) {}

void Process::executeNextInstruction(int coreID) {
    if (isFinished()) return;

    this->coreID = coreID;

    if (sleeping) {
        if (--sleepTicks <= 0) sleeping = false;
        return;
    }

    if (commandCounter < linesOfCode && commandCounter < instructions.size()) {
        instructions[commandCounter]->execute(name, coreID, variables, outputLog, sleeping, sleepTicks);
        commandCounter++;
    }

    if (commandCounter >= linesOfCode) {
        currentState = FINISHED;
        markFinished();
    }
}

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
