#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

class Instruction {
public:

    std::string content;
    std::string type;
    int virtualAddress = -1; 

    Instruction() = default;

    Instruction(const std::string& line);


    virtual void execute(
        const std::string& processName,
        int coreID,
        std::unordered_map<std::string, uint16_t>& variables,
        std::string& outputLog,
        bool& sleeping,
        int& sleepTicks,
        std::shared_ptr<MemoryManager> memoryManager
    ) = 0;

    virtual ~Instruction() = default;
};
