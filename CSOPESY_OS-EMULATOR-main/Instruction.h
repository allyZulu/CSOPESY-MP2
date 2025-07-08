#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

class Instruction {
public:
    virtual void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks
    ) = 0;

    virtual ~Instruction() = default;
};
