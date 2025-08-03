#include "MemoryManager.h"
#include <iostream>
#include <limits>
#include <unordered_set>

// updated memory manager constructor
MemoryManager::MemoryManager(int maxMem, int frameSz, int memPerProc, int instructionSz)
    : maxMemory(maxMem), frameSize(frameSz), memoryPerProcess(memPerProc) {
    totalFrames = maxMemory / frameSize;
    frameTable.resize(totalFrames, false);  // Ensure all entries are initialized
    instructionsPerPage = frameSize / instructionSz;
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
                //frameTable[frame] = false; // mark frame as free 
                frameTable[frame] = -1; //change first line
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

// int MemoryManager::getFreeFrame() {
//     for (int i = 0; i < totalFrames; ++i) {
//         if (frameTable[i] == -1) return i;
//     }
//     return -1; // No free frame
// }

//new
int MemoryManager::getFreeFrame() {
    for (int i = 0; i < totalFrames; ++i) {
        if (!frameTable[i]) return i; // free frame found
    }
    return -1;
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

// bool MemoryManager::ensurePageLoaded(int pid, int virtualAddress) {
//     //neww
//     int pageNumber = virtualAddress / instructionsPerPage;
//     if (pageTables[pid][pageNumber].valid) {
//         accessPage(pid, pageNumber);
//         return true;
//     }

//     int frame = getFreeFrame();
//     if (frame == -1) {
//         evictPageLRU();
//         frame = getFreeFrame();
//         if (frame == -1) return false;
//     }

//     //frameTable[frame] = true;
//     frameTable[frame] = pid; //change from first line before it
//     pageTables[pid][pageNumber] = {frame, true, 0};
//     backingStore[pid].insert(pageNumber);
//     updateLRU(pid, pageNumber);
//     return true;
//     //new
// }

bool MemoryManager::ensurePageLoaded(int pid, int virtualAddress) {
    int pageNumber = virtualAddress / instructionsPerPage;

    // If page is already valid, just update LRU
    // if (pageTables[pid][pageNumber].valid) {
    //     accessPage(pid, pageNumber);
    //     return true;
    // }

    //new
   // int pageNumber = virtualAddress / instructionsPerPage;

    // ✅ If page doesn't exist for this process, return false immediately
    if (processTotalPages.find(pid) == processTotalPages.end() ||
        pageNumber >= processTotalPages[pid]) {
        return false; // Invalid page → no retry
    }

    // ✅ If already loaded
    if (pageTables[pid][pageNumber].valid) {
        accessPage(pid, pageNumber);
        return true;
    }


    // Try to get a free frame
    int frame = getFreeFrame();
    // if (frame == -1) {
    //     evictPageLRU();
    //     frame = getFreeFrame();
    // }
    if (frame == -1) {
    evictPageLRU();
    frame = getFreeFrame();
        if (frame == -1) {
            return false; // ✅ Stop instead of infinite retry
        }
    }


    // If still no frame, cannot load now
    if (frame == -1) {
        return false;  // Scheduler will requeue the process
    }

    // Allocate frame and mark page as valid
    frameTable[frame] = true;
    pageTables[pid][pageNumber] = {frame, true, 0};
    backingStore[pid].insert(pageNumber);
    updateLRU(pid, pageNumber);
    return true;
}


//new
void MemoryManager::accessPage(int pid, int pageNumber) {
    updateLRU(pid, pageNumber);
}

//new function -for vmstat
void MemoryManager::printVMStat() const {
    std::cout << "\n=== VMSTAT ===\n";
    std::cout << "Total Frames: " << totalFrames << "\n";

    int used = 0;

    for (bool usedBit : frameTable){
        if (usedBit) used++;
    }

    int free = totalFrames - used;
    std::cout << "Used Frames: " << used << "\n";
    std::cout << "Free Frames: " << free << "\n";

    std::cout << "\nPer-Process Page Table Info:\n";
    for (const auto& procEntry : pageTables) {
        int pid = procEntry.first;
        const auto& pageMap = procEntry.second;

        std::cout << "PID " << pid << "\n";
        for (const auto& pagePair : pageMap) {
            int pageNumber = pagePair.first;
            const PageTableEntry& entry = pagePair.second;

            std::cout << " Page " << pageNumber << "->";
            if (entry.valid){
                std::cout << "Frame: " << entry.frameNumber;
            } else {
                std::cout << "BACKING STORE";
            }
            std::cout << ", Last Used Tick: " << entry.lastUsedTick << "\n";
           
        }
    }
    std::cout << "=================\n";

    //view of backing store
    std::ofstream backingStoreOut("csopesy-backing-store.txt");
    if(!backingStoreOut.is_open()){
        std::cerr << "Failed to open csopesy-backing-store.txt\n";
        return;
    }

    backingStoreOut << "=== BACKING STORE SNAPSHOT ===\n";
    for (const auto& [pid, pageSet] : backingStore) {
        backingStoreOut << "Process PID: " << pid << "\n";
        for (int page : pageSet) {
            backingStoreOut << "  Stored Page: " << page << "\n";
        }
        backingStoreOut << "\n";
    }
    backingStoreOut << "==============================\n";
    backingStoreOut.close();
}


//new function for process-smi
void MemoryManager::printProcessSMI() const{
    std::cout << "\n=== PROCESS-SMI ===\n";
    
    int total = frameTable.size();
    int used = 0;
    for (bool status : frameTable) {
        if (status) used++;
    }

    int free = total - used;

    std::cout << "Total Memory (bytes): " << total * frameSize << "\n";
    std::cout << "Used Memory (bytes): " << used * frameSize << "\n";
    std::cout << "Free Memory (bytes): " << free * frameSize << "\n\n";

    std::cout << "Per-Process Memory Usage:\n";

    for (const auto& [pid, table] : pageTables) {
        int frameCount = 0;
        for (const auto& [vpn, pte] : table) {
            if (pte.valid) frameCount++;
        }

        std::cout << "PID " << pid 
                  << " → Used Frames: " << frameCount 
                  << ", Bytes: " << (frameCount * frameSize) << "\n";
    }

    std::cout << "=====================\n";
}



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
 //newest checks if an address is valid for a given process.
 bool MemoryManager::isAddressValid(int pid, int virtualAddress) {
    if (processTotalPages.find(pid) == processTotalPages.end()) return false;
    int totalPages = processTotalPages[pid];
    int pageNumber = virtualAddress / instructionsPerPage;

    return pageNumber >= 0 && pageNumber < totalPages;
}


//New - read instruction
uint16_t MemoryManager::readFromAddress(int pid, uint16_t address) {
    // Check if address is valid for this process
    if (!isAddressValid(pid, address)) {
        throw std::runtime_error("Memory violation: address 0x" + 
                                 toHex(address) + " is invalid for process P" + 
                                 std::to_string(pid));
    }

    // Ensure page is loaded; simulate page fault if not
    if (!ensurePageLoaded(pid, address)) {
        throw std::runtime_error("Page fault: page not loaded for address 0x" + 
                                 toHex(address) + " in process P" + std::to_string(pid));
    }

    // Return the value from simulated RAM
    return ram[pid][address];
}

std::string MemoryManager::toHex(uint16_t value) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << value;
    return ss.str();
}

//New -write instruction to memory (RAM)
bool MemoryManager::writeToAddress(int pid, uint16_t address, uint16_t value) {
    // Check if address is valid for the process
    if (!isAddressValid(pid, address)) {
        std::cerr << "Memory violation: Invalid address 0x" << toHex(address)
                  << " for process P" << pid << std::endl;
        return false;
    }

    // Ensure the page is loaded into memory (should already be done by the instruction)
    if (!ensurePageLoaded(pid, address)) {
        std::cerr << "Page fault: Page not loaded for address 0x" << toHex(address)
                  << " in process P" << pid << std::endl;
        return false;
    }

    // Simulate writing to physical memory
    ram[pid][address] = value;

    return true;
}

//New - check for valid address
int MemoryManager::getRandomValidAddress(int pid) {
    if (processTotalPages.find(pid) == processTotalPages.end()) {
        return 0x0000;  // No pages allocated
    }

    int totalPages = processTotalPages[pid];
    if (totalPages == 0) return 0x0000;

    int page = rand() % totalPages;
    int offset = rand() % frameSize;

    return page * instructionsPerPage + offset;  // Virtual address
}

