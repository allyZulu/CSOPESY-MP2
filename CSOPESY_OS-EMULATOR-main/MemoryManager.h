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

struct PageInfo {
    int pid;
    int pageNumber;
};

class MemoryManager {
public:
    MemoryManager(int maxMem, int frameSize, int memPerProcess);

    int allocateMemory(int pid); // allocates memory for process (returns base frame)
    void deallocateMemory(int pid);
    bool ensurePageLoaded(int pid, int virtualAddress);
    void accessPage(int pid, int pageNumber); // triggers LRU update

    int getFrameFromVirtualAddress(int pid, int virtualAddress);
    int getFrameSize() const;

private:
    int maxMemory;
    int frameSize;
    int memoryPerProcess;
    int totalFrames;


    std::vector<bool> frameTable; // true = used, false = free
    std::unordered_map<int, std::set<int>> processPages; // pid -> set of pages loaded
    std::unordered_map<int, std::vector<int>> pageToFrame; // pid -> list of frame numbers per page
    std::list<std::pair<int, int>> lruList; // (pid, pageNumber)
    std::unordered_map<int, std::unordered_map<int, std::list<std::pair<int, int>>::iterator>> lruMap;

    int getFreeFrame();
    void evictPageLRU();
    void updateLRU(int pid, int pageNumber);
};
