#include "MemoryManager.h"
#include <iostream>
#include <limits>
#include <unordered_set>

// added new: int isntructionSize -  size of a single instruction (in bytes)
MemoryManager::MemoryManager(int maxMem, int frameSize, int memPerProcess, int instructionSize)
    : maxMemory(maxMemory), frameSize(frameSize), memoryPerProcess(memPerProcess) {
    totalFrames = maxMemory / frameSize;
    frameTable.resize(totalFrames); //newest cause each entry is in FrameInfo na
    // new alculates how many instructions can fit into one memory page (or frame)
    instructionsPerPage = frameSize / instructionSize;
}

int MemoryManager::getFrameSize() const {
    return frameSize;
}

//newest: removed registerProcess(pid, totalPages); kasi redundant na
bool MemoryManager::allocateMemory(std::shared_ptr<Process> proc) {
    // No-op in demand paging — memory is allocated per-page as needed
    //new
    int pid = proc->getPID();
    int instrCount = proc->getInstructions().size();
   
    int instructionPages = (instrCount + instructionsPerPage - 1) / instructionsPerPage; // How many pages are needed for instructions
    int totalPages = instructionPages + 1; // +1 page for the symbol table

    if (totalPages * frameSize > memoryPerProcess) return false; // Check per-process memory limit

    processTotalPages[pid] = totalPages;
    pageTables[pid] = std::unordered_map<int, PageTableEntry>();
    backingStore[pid] = std::unordered_set<int>();

    // === Allocate the symbol table page (page 0) first ===
    int frame = getFreeFrame();
    if (frame == -1) {
        // No frame available, symbol table is not loaded yet
        pageTables[pid][0] = {-1, false, 0};  
        backingStore[pid].insert(0);
    } else {
        // Place symbol table in memory
        frameTable[frame] = true;
        pageTables[pid][0] = {frame, true, 0};
        updateLRU(pid, 0);
    }

    // === Allocate instruction pages (pages 1..n) ===
    for (int page = 1; page < totalPages; ++page) {
        int frame = getFreeFrame();
        if (frame == -1) {
            // Not in memory yet, mark for backing store
            pageTables[pid][page] = {-1, false, 0};
            backingStore[pid].insert(page);
        } else {
            frameTable[frame] = true; 
            pageTables[pid][page] = {frame, true, 0};
            updateLRU(pid, page);
        }
    }
    return true;
}
//newest

// newest edited this one 
void MemoryManager::deallocateMemory(std::shared_ptr<Process> process) {
   //new
   int pid = process->getPID();   // Check if the process has a page table
    if (pageTables.find(pid) != pageTables.end()) {
        for (auto& entry : pageTables[pid]) {
            int frame = entry.second.frameNumber; 
            if (entry.second.valid && entry.second.frameNumber != -1) {
                frameTable[frame] = false; // mark frame as free 
            }
        }
        pageTables.erase(pid);

    }
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

 //newest checks if an address is valid for a given process.
 bool MemoryManager::isAddressValid(int pid, int virtualAddress) {
    if (processTotalPages.find(pid) == processTotalPages.end()) return false;
    int totalPages = processTotalPages[pid];
    int pageNumber = virtualAddress / instructionsPerPage;

    return pageNumber >= 0 && pageNumber < totalPages;
}



