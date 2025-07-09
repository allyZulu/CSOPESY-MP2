#pragma once

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include "Process.h"

class MemoryManager {
public:
    MemoryManager(int totalMem, int frameSize, int memPerProcess);

    bool allocate(std::shared_ptr<Process> process);
    void deallocate(std::shared_ptr<Process> process);
    void snapshot(int quantumCycle);
    int getNumProcessesInMemory() const;
    int getExternalFragmentation() const;

private:
    struct Block {
        int start;
        int size;
        std::shared_ptr<Process> process;
    };

    int totalMemory;
    int frameSize;
    int memoryPerProcess;
    std::vector<Block> memoryBlocks;


    std::string getTimestamp() const;

    int findFirstFitIndex(int requiredSize) const;
};
