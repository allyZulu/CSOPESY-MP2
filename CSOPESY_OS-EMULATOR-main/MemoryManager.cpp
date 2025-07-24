#include "MemoryManager.h"
#include <iostream>
#include <limits>
#include <unordered_set>

// added new: int isntructionSize -  size of a single instruction (in bytes)
MemoryManager::MemoryManager(int maxMem, int frameSize, int memPerProcess, int instructionSize)
    : maxMemory(maxMemory), frameSize(frameSize), memoryPerProcess(memPerProcess) {
    totalFrames = maxMemory / frameSize;
    frameTable.resize(totalFrames, -1);
    // new alculates how many instructions can fit into one memory page (or frame)
    instructionsPerPage = frameSize / instructionSize;
}

int MemoryManager::getFrameSize() const {
    return frameSize;
}

int MemoryManager::allocateMemory(int pid) {
    // No-op in demand paging — memory is allocated per-page as needed
    return 0;
}

void MemoryManager::deallocateMemory(int pid) {
    for (int i = 0; i < totalFrames; ++i) {
        if (frameTable[i] == pid) {
            frameTable[i] = -1;
        }
    }
    processPages.erase(pid);
    pageToFrame.erase(pid);

    if (lruMap.count(pid)) {
        for (auto& entry : lruMap[pid]) {
            lruList.erase(entry.second);
        }
        lruMap.erase(pid);
    }
}

int MemoryManager::getFreeFrame() {
    for (int i = 0; i < totalFrames; ++i) {
        if (frameTable[i] == -1) return i;
    }
    return -1; // No free frame
}

void MemoryManager::evictPageLRU() {
    if (lruList.empty()) return;

    auto [evictPid, evictPage] = lruList.back();
    lruList.pop_back();

    int frameToFree = pageToFrame[evictPid][evictPage];
    frameTable[frameToFree] = -1;
    processPages[evictPid].erase(evictPage);
    pageToFrame[evictPid][evictPage] = -1;
    lruMap[evictPid].erase(evictPage);
}

void MemoryManager::updateLRU(int pid, int pageNumber) {
    if (lruMap[pid].count(pageNumber)) {
        lruList.erase(lruMap[pid][pageNumber]);
    }
    lruList.push_front({pid, pageNumber});
    lruMap[pid][pageNumber] = lruList.begin();
}

bool MemoryManager::ensurePageLoaded(int pid, int virtualAddress) {
    int pageNumber = virtualAddress / frameSize;

    if (processPages[pid].count(pageNumber)) {
        updateLRU(pid, pageNumber);
        return true;
    }
    
    // new checks if the requested page for a process exists in the backing store
    if (!backingStore[pid].count(pageNumber)) {
        std::cerr << "Page not in backing store: PID " << pid << ", Page " << pageNumber << "\n";
        return false;
    }
    // new

    int frame = getFreeFrame();
    if (frame == -1) {
        evictPageLRU();
        frame = getFreeFrame();
        if (frame == -1) {
            std::cerr << "Failed to load page for PID " << pid << " - Out of memory\n";
            return false;
        }
    }

    frameTable[frame] = pid;
    processPages[pid].insert(pageNumber);
    if (pageToFrame[pid].size() <= pageNumber)
        pageToFrame[pid].resize(pageNumber + 1, -1);
    pageToFrame[pid][pageNumber] = frame;
    updateLRU(pid, pageNumber);
    return true;
}

int MemoryManager::getFrameFromVirtualAddress(int pid, int virtualAddress) {
    int pageNumber = virtualAddress / frameSize;
    if (pageToFrame.count(pid) && pageToFrame[pid].size() > pageNumber)
        return pageToFrame[pid][pageNumber];
    return -1;
}

// new print the current state of memory, specifically showing which frames are free or occupied and by which PID (process ID).
void MemoryManager::printMemoryState() {
    std::cout << "Frame Table:\n";
    for (int i = 0; i < frameTable.size(); ++i) {
        std::cout << "Frame " << i << ": ";
        if (frameTable[i] == -1) std::cout << "Free\n";
        else std::cout << "PID " << frameTable[i] << "\n";
    }
}
 // new


// new: Prepares all valid pages of a process (instructions + symbol table) in the backing store
void MemoryManager::registerProcess(int pid, int instructionCount) {
    const int SYMBOL_TABLE_PAGES = 1;
    int instructionPages = (instructionCount + instructionsPerPage - 1) / instructionsPerPage;
    int totalPages = instructionPages + SYMBOL_TABLE_PAGES;

    for (int i = 0; i < totalPages; ++i) {
        backingStore[pid].insert(i);  // These pages are available
    }

}
 // new


