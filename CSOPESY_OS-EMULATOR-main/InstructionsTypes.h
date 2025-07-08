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
        int& sleepTicks
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

class DeclareInstruction : public Instruction {
    std::string var;
    uint16_t value;
public:
    DeclareInstruction(const std::string& var, uint16_t value) : var(var), value(value) {}

    void execute(
        const std::string&,
        int,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string&,
        bool&,
        int&
    ) override {
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
        int&
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
        int&
    ) override {
        uint16_t v1 = variables[op1];
        uint16_t v2 = isOp2Value ? value2 : variables[op2];
        variables[dest] = v1 - v2;
    }
};
