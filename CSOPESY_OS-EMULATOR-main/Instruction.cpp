// Instruction.cpp - Contains instruction subclasses

#include "Instruction.h"
#include "MemoryManager.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>


class DeclareInstruction : public Instruction {
    std::string var;
    uint16_t value;

public:
    DeclareInstruction(const std::string& var, uint16_t value)
        : var(var), value(value) {}

    void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager);
    // ) override {
    //     variables[var] = value;
    // }
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
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager);

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
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager);
 
};

class SleepInstruction : public Instruction {
    uint8_t ticks;

public:
    SleepInstruction(uint8_t ticks) : ticks(ticks) {}

    void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager);

};

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