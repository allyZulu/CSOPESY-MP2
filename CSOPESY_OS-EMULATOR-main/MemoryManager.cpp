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

bool MemoryManager::allocateMemory(std::shared_ptr<Process> proc) {
    // No-op in demand paging — memory is allocated per-page as needed
    //new
    int pid = proc->getPID();
    int instrCount = proc->getInstructions().size();
    int totalPages = (instrCount + instructionsPerPage - 1) / instructionsPerPage;

    if (totalPages * frameSize > memoryPerProcess) return false;

    processTotalPages[pid] = totalPages;
    pageTables[pid] = std::unordered_map<int, PageTableEntry>();
    backingStore[pid] = std::unordered_set<int>();
    registerProcess(pid, totalPages);
    return true;
    //new
}

void MemoryManager::deallocateMemory(std::shared_ptr<Process> process) {
   //new
   int pid = process->getPID();
    if (pageTables.find(pid) != pageTables.end()) {
        for (auto& entry : pageTables[pid]) {
            int page = entry.first;
            if (entry.second.valid && entry.second.frameNumber != -1) {
                frameTable[entry.second.frameNumber] = false;
            }
        }
    }
    pageTables.erase(pid);
    backingStore.erase(pid);
    processPages.erase(pid);
    pageToFrame.erase(pid);
    lruMap.erase(pid);
    processTotalPages.erase(pid);
   //new
}

int MemoryManager::getFreeFrame() {
    for (int i = 0; i < totalFrames; ++i) {
        if (frameTable[i] == -1) return i;
    }
    return -1; // No free frame
}

void MemoryManager::evictPageLRU() {
    if (lruList.empty()) return;

    auto evict = lruList.back();
    lruList.pop_back();

    int pid = evict.first;
    int pageNumber = evict.second;
    if (pageTables[pid][pageNumber].valid) {
        int frame = pageTables[pid][pageNumber].frameNumber;
        frameTable[frame] = false;
        pageTables[pid][pageNumber] = {-1, false, 0};
        lruMap[pid].erase(pageNumber);
    }
}

void MemoryManager::updateLRU(int pid, int pageNumber) {
    if (lruMap[pid].count(pageNumber)) {
        lruList.erase(lruMap[pid][pageNumber]);
    }
    lruList.push_front({pid, pageNumber});
    lruMap[pid][pageNumber] = lruList.begin();
}

bool MemoryManager::ensurePageLoaded(int pid, int virtualAddress) {
    //neww
    int pageNumber = virtualAddress / instructionsPerPage;
    if (pageTables[pid][pageNumber].valid) {
        accessPage(pid, pageNumber);
        return true;
    }

    int frame = getFreeFrame();
    if (frame == -1) {
        evictPageLRU();
        frame = getFreeFrame();
        if (frame == -1) return false;
    }

    frameTable[frame] = true;
    pageTables[pid][pageNumber] = {frame, true, 0};
    backingStore[pid].insert(pageNumber);
    updateLRU(pid, pageNumber);
    return true;
    //new
}

//new
void MemoryManager::accessPage(int pid, int pageNumber) {
    updateLRU(pid, pageNumber);
}

void MemoryManager::printVMStat() const {
    std::cout << "\n=== VMSTAT ===\n";
    std::cout << "Total Frames: " << totalFrames << "\n";

    int used = 0;
    int free = 0;
    for (size_t i = 0; i < frameTable.size(); ++i) {
        if (frameTable[i])
            used++;
        else
            free++;
    }

    std::cout << "Used Frames: " << used << "\n";
    std::cout << "Free Frames: " << (totalFrames - used) << "\n";

    std::cout << "\nProcess Page Tables:\n";
    for (const auto& entry : pageTables) {
        int pid = entry.first;
        const auto& pageMap = entry.second;

        std::cout << "PID " << pid << ": ";
        for (const auto& pagePair : pageMap) {
            int frame = pagePair.second.frameNumber;
            std::cout << frame << " ";
        }
        std::cout << "\n";
    }

    std::cout << "=================\n";
}
//new

int MemoryManager::getFrameFromVirtualAddress(int pid, int virtualAddress) {
    //new
    int pageNumber = virtualAddress / instructionsPerPage;
    if (!pageTables[pid][pageNumber].valid) return -1;
    return pageTables[pid][pageNumber].frameNumber;
    //new
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


// Prepares all valid pages of a process (instructions + symbol table) in the backing store
void MemoryManager::registerProcess(int pid, int instructionCount) {
    const int SYMBOL_TABLE_PAGES = 1;
    int instructionPages = (instructionCount + instructionsPerPage - 1) / instructionsPerPage;
    int totalPages = instructionPages + SYMBOL_TABLE_PAGES;

    for (int i = 0; i < totalPages; ++i) {
        backingStore[pid].insert(i);  // These pages are available
    }
    processTotalPages[pid] = totalPages;  // Track for cleanup or diagnostics

}

// 
int MemoryManager::getInstructionsPerPage() const {
    return instructionsPerPage;
}
 // new


