#include "MemoryManager.h"
#include <iostream>
#include <limits>

MemoryManager::MemoryManager(int maxMem, int frameSize, int memPerProcess)
    : maxMemory(maxMemory), frameSize(frameSize), memoryPerProcess(memPerProcess) {
    totalFrames = maxMemory / frameSize;
    frameTable.resize(totalFrames, -1);
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