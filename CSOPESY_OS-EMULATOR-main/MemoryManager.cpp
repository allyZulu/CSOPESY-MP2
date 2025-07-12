#include "MemoryManager.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <ctime>
#include <chrono>
#include <iomanip>


MemoryManager::MemoryManager(int totalMem, int frameSize, int memPerProcess)
    : totalMemory(totalMem), frameSize(frameSize), memoryPerProcess(memPerProcess) {}

bool MemoryManager::allocate(std::shared_ptr<Process> process) {
    int needed = ((memoryPerProcess + frameSize - 1) / frameSize) * frameSize;

    // Sort blocks by start address
    std::sort(memoryBlocks.begin(), memoryBlocks.end(), [](const Block& a, const Block& b) {
        return a.start < b.start;
    });

    int lastEnd = 0;
    for (auto& block : memoryBlocks) {
        if (block.start - lastEnd >= needed) {
            // Found a gap
            memoryBlocks.push_back({ lastEnd, needed, process });
            return true;
        }
        lastEnd = block.start + block.size;
    }

    if (totalMemory - lastEnd >= needed) {
        memoryBlocks.push_back({ lastEnd, needed, process });
        return true;
    }

    return false; // No space
}

void MemoryManager::deallocate(std::shared_ptr<Process> process) {
    int pid = process->getPID();  // Get process ID for comparison

    memoryBlocks.erase(std::remove_if(memoryBlocks.begin(), memoryBlocks.end(),
        [&](const Block& block) {
             return block.process && block.process->getPID() == pid;
        }), memoryBlocks.end());
}

int MemoryManager::getNumProcessesInMemory() const {
    std::unordered_set<std::shared_ptr<Process>> unique;
    for (const auto& block : memoryBlocks) {
        if (block.process)
            unique.insert(block.process);
    }
    
    //return static_cast<int>(memoryBlocks.size());
    return static_cast<int>(unique.size());
}

int MemoryManager::getExternalFragmentation() const {
    int externalFrag = 0;
    int lastEnd = 0;

    std::vector<Block> sorted = memoryBlocks;
    std::sort(sorted.begin(), sorted.end(), [](const Block& a, const Block& b) {
        return a.start < b.start;
    });

    for (const auto& block : sorted) {
        if (block.start > lastEnd) {
            externalFrag += (block.start - lastEnd);
        }
        lastEnd = block.start + block.size;
    }

    if (lastEnd < totalMemory) {
        externalFrag += (totalMemory - lastEnd);
    }

    return externalFrag / 1024;
}

std::string MemoryManager::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << "(" << std::put_time(&tm, "%m/%d/%Y %I:%M:%S%p") << ")";
    return oss.str();
}

void MemoryManager::snapshot(int quantumCycle) {
    std::ostringstream filename;
    filename << "memory_stamp_" << quantumCycle << ".txt";
    std::ofstream out(filename.str());

    if (!out.is_open()) return;

    out << "Timestamp: " << getTimestamp() << "\n";
    out << "Number of processes in memory: " << getNumProcessesInMemory() << "\n";
    out << "Total external fragmentation in KB: " << getExternalFragmentation() << "\n\n";

    out << "----end---- = " << totalMemory << "\n";

    std::vector<Block> sorted = memoryBlocks;
    std::sort(sorted.begin(), sorted.end(), [](const Block& a, const Block& b) {
        return a.start > b.start;
    });

    for (const auto& block : sorted) {
        out << block.start + block.size << "\n";
        out << block.process->getName() << "\n";
        out << block.start << "\n\n";
    }

    out << "----start---- = 0\n";
    out.close();
}
