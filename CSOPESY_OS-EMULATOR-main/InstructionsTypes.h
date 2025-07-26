// InstructionTypes.h
#pragma once

#include "Instruction.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <ctime>


class PrintInstruction : public Instruction {
    std::string message;
public:
    PrintInstruction(const std::string& msg) : message(msg) {}

    void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager
    ) override {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_time);

        std::ostringstream oss;
         oss << "Core " << coreID << " | " << processName << ": " << message;
         oss << " [" << std::put_time(&local_tm, "%H:%M:%S %m/%d/%Y") << "]\n";

        outputLog += oss.str();
    }
};

// Updated DeclareInstruction to enforce symbol table page usage (page 0):
class DeclareInstruction : public Instruction {
    std::string var;
    uint16_t value;
public:
    DeclareInstruction(const std::string& var, uint16_t value) : var(var), value(value) {}

    void execute(
        const std::string& processName,  // give it a name
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,         
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager // NEW PARAM
    ) override {
        // Get PID from process name (e.g. "p1")
        int pid = std::stoi(processName.substr(1));

        // Ensure the symbol table page (page 0) is loaded
        if (!memoryManager->ensurePageLoaded(pid, 0)) {
            // Page could not be loaded -> halt process
            outputLog += "Process " + processName + 
                         " terminated: could not load symbol table page.\n";
            sleeping = true; // stops the process
            return;
        }
        // If page 0 is loaded, perform declaration
        variables[var] = value;
    }
};

class AddInstruction : public Instruction {
    std::string dest, op1, op2;
    bool isOp2Value;
    uint16_t value2;

public:
    AddInstruction(const std::string& dest, const std::string& op1, const std::string& op2)
        : dest(dest), op1(op1), op2(op2), isOp2Value(false) {
        try {
            value2 = std::stoi(op2);
            isOp2Value = true;
        } catch (...) {}
    }

    void execute(
        const std::string&,
        int,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string&,
        bool&,
        int&,
        std::shared_ptr<MemoryManager> memoryManager //newest
    ) override {
        uint16_t v1 = variables[op1];
        uint16_t v2 = isOp2Value ? value2 : variables[op2];
        variables[dest] = v1 + v2;
    }
};

class SubtractInstruction : public Instruction {
    std::string dest, op1, op2;
    bool isOp2Value;
    uint16_t value2;

public:
    SubtractInstruction(const std::string& dest, const std::string& op1, const std::string& op2)
        : dest(dest), op1(op1), op2(op2), isOp2Value(false) {
        try {
            value2 = std::stoi(op2);
            isOp2Value = true;
        } catch (...) {}
    }

    void execute(
        const std::string&,
        int,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string&,
        bool&,
        int&,
        std::shared_ptr<MemoryManager> memoryManager //newest
    ) override {
        uint16_t v1 = variables[op1];
        uint16_t v2 = isOp2Value ? value2 : variables[op2];
        variables[dest] = v1 - v2;
    }
};
// newest
class ReadInstruction : public Instruction {
    std::string var;
    int address;

public:
    ReadInstruction(const std::string& var, int address)
        : var(var), address(address) {}

    void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager
    ) override {
        int pid = std::stoi(processName.substr(1));

        // Validate address
        if (!memoryManager->isAddressValid(pid, address)) {
            outputLog += "Memory access violation at address 0x" +
                         std::to_string(address) + " in process " + processName + "\n";
            sleeping = true;
            return;
        }

        // Ensure page loaded
        if (!memoryManager->ensurePageLoaded(pid, address)) {
            outputLog += "Page fault on READ at 0x" + std::to_string(address) +
                         " for process " + processName + "\n";
            sleeping = true;
            return;
        }

        // For now, just store a dummy value
        variables[var] = address;
    }
};

// ================== WriteInstruction ==================
class WriteInstruction : public Instruction {
    int address;
    std::string valueOrVar;

public:
    WriteInstruction(int address, const std::string& valueOrVar)
        : address(address), valueOrVar(valueOrVar) {}

    void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager
    ) override {
        int pid = std::stoi(processName.substr(1));

        // Validate address
        if (!memoryManager->isAddressValid(pid, address)) {
            outputLog += "Memory access violation at address 0x" +
                         std::to_string(address) + " in process " + processName + "\n";
            sleeping = true;
            return;
        }

        // Ensure page loaded
        if (!memoryManager->ensurePageLoaded(pid, address)) {
            outputLog += "Page fault on WRITE at 0x" + std::to_string(address) +
                         " for process " + processName + "\n";
            sleeping = true;
            return;
        }

        // Resolve value to write
        uint16_t value = 0;
        if (variables.find(valueOrVar) != variables.end()) {
            value = variables[valueOrVar];
        } else {
            value = static_cast<uint16_t>(std::stoi(valueOrVar));
        }

        // For now, just log it
        outputLog += "Wrote value " + std::to_string(value) +
                     " to address 0x" + std::to_string(address) + "\n";
    }
};
