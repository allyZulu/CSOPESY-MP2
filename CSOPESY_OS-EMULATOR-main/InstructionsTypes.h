// InstructionTypes.h
#pragma once

#include "Instruction.h"
#include "MemoryManager.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <iostream>



class PrintInstruction : public Instruction {
    std::string message;
public:
    PrintInstruction(const std::string& msg);
    void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager
    ) override;
};

// Updated DeclareInstruction to enforce symbol table page usage (page 0):
class DeclareInstruction : public Instruction {
    std::string var;
    uint16_t value;
public:
    DeclareInstruction(const std::string& var, uint16_t value);

    void execute(
        const std::string& processName,  // give it a name
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,         
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager // NEW PARAM
    ) override;
};

class AddInstruction : public Instruction {
    std::string dest, op1, op2;
    bool isOp2Value;
    uint16_t value2;

public:
    AddInstruction(const std::string& dest, const std::string& op1, const std::string& op2);

    void execute(
        const std::string&,
        int,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string&,
        bool&,
        int&,
        std::shared_ptr<MemoryManager> memoryManager //newest
    ) override;
};

class SubtractInstruction : public Instruction {
    std::string dest, op1, op2;
    bool isOp2Value;
    uint16_t value2;

public:
    SubtractInstruction(const std::string& dest, const std::string& op1, const std::string& op2);

    void execute(
        const std::string&,
        int,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string&,
        bool&,
        int&,
        std::shared_ptr<MemoryManager> memoryManager //newest
    ) override;
};


// UPDATED READ INSTRUCTION
class ReadInstruction : public Instruction {
    std::string var;
    int address;
    std::shared_ptr<MemoryManager> memoryManager;

public:
    ReadInstruction(const std::string& variable, int addr, MemoryManager* memManager);

    void execute(
            const std::string& processName,
            int coreID,
            std::unordered_map<std::string, uint16_t>& variables,
            std::string& outputLog,
            bool& sleeping,
            int& sleepTicks,
            std::shared_ptr<MemoryManager> memoryManager
        ) override;
};

// ================== WriteInstruction ==================
class WriteInstruction : public Instruction {
    std::string address;
    std::string valueOrVar;

public:
    WriteInstruction(const std::string& addressHex, const std::string& valueOrVar);

    void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager
    ) override;
};

