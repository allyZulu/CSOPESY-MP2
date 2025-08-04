// Instruction.cpp - Contains instruction subclasses

#include "InstructionsTypes.h"
#include "Instruction.h"
#include "MemoryManager.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

Instruction::Instruction(const std::string& line) : content(line) {
    std::istringstream iss(line);
    iss >> type;

    // A simple heuristic to assign a virtual address for memory access simulation
    std::string var;
    if (type == "DECLARE" || type == "WRITE" || type == "READ") {
        iss >> var;
        if (!var.empty()) {
            // Map variable name to a fake address (hashing approach)
            virtualAddress = std::hash<std::string>{}(var) % 4096; // Clamp to per-process space
        }
    }
}

//transfer all the implementation in cpp
PrintInstruction::PrintInstruction(const std::string& msg) : message(msg) {}
DeclareInstruction::DeclareInstruction(const std::string& var, uint16_t value) : var(var), value(value) {}
AddInstruction::AddInstruction(const std::string& dest, const std::string& op1, const std::string& op2) : dest(dest), op1(op1), op2(op2), isOp2Value(false) {
        try {
            value2 = std::stoi(op2);
            isOp2Value = true;
        } catch (...) {}
    }
SubtractInstruction::SubtractInstruction(const std::string& dest, const std::string& op1, const std::string& op2)
        : dest(dest), op1(op1), op2(op2), isOp2Value(false) {
        try {
            value2 = std::stoi(op2);
            isOp2Value = true;
        } catch (...) {}
    }
ReadInstruction::ReadInstruction(const std::string& variable, int addr, MemoryManager* memManager)
            : var(variable), address(addr), memoryManager(memManager) {}
WriteInstruction:: WriteInstruction(const std::string& addressHex, const std::string& valueOrVar)
        : address(addressHex), valueOrVar(valueOrVar) {} 

void PrintInstruction::execute(
    const std::string& processName,
    int coreID,
    std::unordered_map<std::string, uint16_t>& variables,
    std::string& outputLog,
    bool& sleeping,
    int& sleepTicks,
    std::shared_ptr<MemoryManager> memoryManager
) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_time);

        std::ostringstream oss;
         oss << "Core " << coreID << " | " << processName << ": " << message;
         oss << " [" << std::put_time(&local_tm, "%H:%M:%S %m/%d/%Y") << "]\n";

        outputLog += oss.str();
    }


void DeclareInstruction::execute(
    const std::string& processName,  // give it a name
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,         
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager // NEW PARAM
) {
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


void AddInstruction::execute(
    const std::string&,
        int,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string&,
        bool&,
        int&,
        std::shared_ptr<MemoryManager> memoryManager //newest
) {
    uint16_t v1 = variables[op1];
    uint16_t v2 = isOp2Value ? value2 : variables[op2];
    variables[dest] = v1 + v2;
}

void SubtractInstruction::execute(
    const std::string&,
    int,
    std::unordered_map<std::string, uint16_t>& variables,
    std::string&,
    bool&,
    int&,
    std::shared_ptr<MemoryManager> memoryManager //newest
) {
    uint16_t v1 = variables[op1];
    uint16_t v2 = isOp2Value ? value2 : variables[op2];
    variables[dest] = v1 - v2;
}

void ReadInstruction::execute(
    const std::string& processName,
    int coreID,
    std::unordered_map<std::string, uint16_t>& variables,
    std::string& outputLog,
    bool& sleeping,
    int& sleepTicks,
    std::shared_ptr<MemoryManager> memoryManager
) {
    int pid = std::stoi(processName.substr(1));

    // Check if address is valid
    if (!memoryManager->isAddressValid(pid, address)) {
        outputLog += "Memory access violation at address 0x" +
                std::to_string(address) + " in process " + processName + "\n";
                sleeping = true;
                return;
            }

    // Ensure page is loaded
    if (!memoryManager->ensurePageLoaded(pid, address)) {
                outputLog += "Page fault on READ at 0x" + std::to_string(address) +
                            " for process " + processName + "\n";
                sleeping = true;
                return;
            }

        
    uint16_t value = memoryManager->readFromAddress(pid, address); 
    variables[var] = value;

    outputLog += "Core " + std::to_string(coreID) + " | Process " + processName +
                    " read " + std::to_string(value) + " from address 0x" + memoryManager->toHex(address) +
                    " into variable '" + var + "'\n";

}

void WriteInstruction::execute(
    const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager
) {
    int pid = std::stoi(processName.substr(1));

        // Parse address
        uint16_t addr = 0;
        try {
            addr = static_cast<uint16_t>(std::stoi(address, nullptr, 16));
        } catch (...) {
            outputLog += "Invalid hex address format: " + address + "\n";
            sleeping = true;
            return;
        }

        // Check address validity
        if (!memoryManager->isAddressValid(pid, addr)) {
            outputLog += "Memory violation: invalid address 0x" + address +
                         " for process " + processName + "\n";
            sleeping = true;
            return;
        }

        // Ensure page is loaded
        if (!memoryManager->ensurePageLoaded(pid, addr)) {
            outputLog += "Page fault on WRITE at address 0x" + address +
                         " for process " + processName + "\n";
            sleeping = true;
            return;
        }

        // Resolve the value to write
        uint16_t value = 0;
        if (variables.find(valueOrVar) != variables.end()) {
            value = variables[valueOrVar];
        } else {
            try {
                value = static_cast<uint16_t>(std::stoi(valueOrVar));
            } catch (...) {
                outputLog += "Invalid value for WRITE: " + valueOrVar + "\n";
                sleeping = true;
                return;
            }
        }

        // Perform the write
        bool success = memoryManager->writeToAddress(pid, addr, value);
        if (!success) {
            outputLog += "Failed to WRITE to address 0x" + address +
                         " for process " + processName + "\n";
            sleeping = true;
            return;
        }

        // Log success
        outputLog += "Core " + std::to_string(coreID) + " | Process " + processName +
                     " wrote " + std::to_string(value) + " to address 0x" + address + "\n";
}

int ReadInstruction::getAddress() const{
    return address;
}

int WriteInstruction::getAddress() const{
    return std::stoi(address, nullptr, 16);
}