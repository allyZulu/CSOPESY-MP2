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

struct PageTableEntry {
    int frameNumber;
    bool valid;
};

struct FrameInfo {
    bool isUsed = false;
    int pid = -1;
    int pageNumber = -1;
};


class MemoryManager {
public:
    // mew : int instructionSize
    MemoryManager(int maxMem, int frameSize, int memPerProcess, int instructionSize); 

    int allocateMemory(int pid); // allocates memory for process (returns base frame)
    void deallocateMemory(int pid);
    bool ensurePageLoaded(int pid, int virtualAddress);
    void accessPage(int pid, int pageNumber); // triggers LRU update

    int getFrameFromVirtualAddress(int pid, int virtualAddress);
    int getFrameSize() const;

    // new
    void printMemoryState();
    void registerProcess(int pid, int totalPages);
    // new

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
  
    // new
    std::unordered_map<int, std::unordered_set<int>> backingStore; // pid → valid page numbers
    // new

    // LRU
    std::list<std::pair<int, int>> lruList; // (pid, pageNumber)
   
    // new
    std::unordered_map<int, std::unordered_map<int, std::list<std::pair<int, int>>::iterator>> lruMap;
    std::unordered_map<int, std::unordered_set<int>> backingStore; // pid -> set of pageNumbers
    std::unordered_map<int, int> processTotalPages; // pid -> number of total pages
    // new

    int getFreeFrame();
    void evictPageLRU();
    void updateLRU(int pid, int pageNumber);
};
