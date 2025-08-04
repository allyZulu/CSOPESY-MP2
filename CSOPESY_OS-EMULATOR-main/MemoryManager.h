#pragma once

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <map>
#include "Process.h"
#include <set>
#include <list>
#include <unordered_set>

struct PageInfo {
    int pid;
    int pageNumber;
};

// new
struct PageTableEntry {
    int frameNumber = -1;       // -1 if not in memory
    bool valid = false;         // true if loaded into RAM
    int lastUsedTick = 0;
};

struct FrameInfo {
    bool isUsed = false;
    int pid = -1;
    int pageNumber = -1;
};
// new

//new add declaration of Process Class
class Process; 

class MemoryManager {
public:
    // mew : int instructionSize
    MemoryManager(int maxMem, int frameSize, int memPerProcess, int instructionSize); 

    bool allocateMemory(std::shared_ptr<Process> proc); // allocates memory for process (returns base frame)
    void deallocateMemory(std::shared_ptr<Process> process);
    bool ensurePageLoaded(int pid, int virtualAddress);
    void accessPage(int pid, int pageNumber); // triggers LRU update

    int getFrameFromVirtualAddress(int pid, int virtualAddress);

    int getFrameSize() const;
    bool isAddressValid(int pid, int virtualAddress);
    void printMemoryState();
    void registerProcess(int pid, int totalPages);
    int getInstructionsPerPage() const;

    // new for VMSTAT, ProcessSmi, Write instruction, Read Instruction
    void printVMStat() const;
    void printProcessSMI() const;
    bool writeToAddress(int pid, uint16_t address, uint16_t value); 
    uint16_t readFromAddress(int pid, uint16_t address);
    std::string toHex(uint16_t value);
    int getRandomValidAddress(int pid);
    int getInstructionPageNumber(int programCounter);

    //ALLY
    bool isPageLoaded(int pid, int pageNumber) const;
//ALLY

private:
    int maxMemory;
    int frameSize;
    int memoryPerProcess;
    int totalFrames;
    //new
    int instructionsPerPage;
    int instructionSize;
    // new


    std::vector<bool> frameTable; // true = used, false = free

    // page management 
    std::unordered_map<int, std::set<int>> processPages; // pid -> set of pages loaded
    std::unordered_map<int, std::vector<int>> pageToFrame; // pid -> list of frame numbers per page
    std::unordered_map<int, std::unordered_map<int, std::list<std::pair<int, int>>::iterator>> lruMap;
    std::unordered_map<int, std::unordered_map<int, PageTableEntry>> pageTables; // pid → (virtualPage → PageTableEntry)
  
    // LRU
    std::list<std::pair<int, int>> lruList; // (pid, pageNumber)
   
    // new
    std::unordered_map<int, std::unordered_set<int>> backingStore; // pid -> set of pageNumbers
    std::unordered_map<int, int> processTotalPages; // pid -> number of total pages
    std::unordered_map<int, std::unordered_map<uint16_t, uint16_t>> ram; // pid → address → value
    // new

    int getFreeFrame();
    void evictPageLRU();
    void updateLRU(int pid, int pageNumber);
};
