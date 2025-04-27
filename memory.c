#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_BLOCKS 100
#define MAX_PROCESSES 20

typedef struct {
    int start;
    int size;
    bool free;
    char pid[10];
} MemoryBlock;

typedef struct {
    char pid[10];
    int arrival;
    int size;
    bool allocated;
} Process;

MemoryBlock memory[MAX_BLOCKS];
int blockCount = 1;

char fifoQueue[MAX_PROCESSES][10]; // For FIFO replacement
int fifoFront = 0, fifoRear = 0;

void enqueue(const char *pid) {
    strcpy(fifoQueue[fifoRear++], pid);
}

char* dequeue() {
    return fifoQueue[fifoFront++];
}

void initializeMemory(int totalMemory) {
    memory[0].start = 0;
    memory[0].size = totalMemory;
    memory[0].free = true;
    strcpy(memory[0].pid, "");
}

void displayMemory(int totalMemory) {
    int used = 0;
    printf("\n🧠 Memory Layout:\n");
    for (int i = 0; i < blockCount; i++) {
        printf("[%d - %d] : %s (Size: %d)\n",
               memory[i].start,
               memory[i].start + memory[i].size - 1,
               memory[i].free ? "Free" : memory[i].pid,
               memory[i].size);
        if (!memory[i].free) used += memory[i].size;
    }
    printf("Total: %d KB | Used: %d KB | Free: %d KB\n", totalMemory, used, totalMemory - used);
}

void mergeFreeBlocks() {
    for (int i = 0; i < blockCount - 1; i++) {
        if (memory[i].free && memory[i + 1].free) {
            memory[i].size += memory[i + 1].size;
            for (int j = i + 1; j < blockCount - 1; j++) {
                memory[j] = memory[j + 1];
            }
            blockCount--;
            i--; // recheck after merge
        }
    }
}

void deallocateOldest() {
    char *oldestPid = dequeue();
    printf("🔄 Replacing process %s due to lack of memory\n", oldestPid);
    for (int i = 0; i < blockCount; i++) {
        if (!memory[i].free && strcmp(memory[i].pid, oldestPid) == 0) {
            memory[i].free = true;
            strcpy(memory[i].pid, "");
            mergeFreeBlocks();
            return;
        }
    }
}

bool allocateMemory(Process *p) {
    for (int i = 0; i < blockCount; i++) {
        if (memory[i].free && memory[i].size >= p->size) {
            printf("✅ Allocating %s (%d KB) at address %d\n", p->pid, p->size, memory[i].start);

            int remaining = memory[i].size - p->size;

            memory[i].size = p->size;
            memory[i].free = false;
            strcpy(memory[i].pid, p->pid);
            p->allocated = true;
            enqueue(p->pid);

            if (remaining > 0) {
                for (int j = blockCount; j > i + 1; j--) {
                    memory[j] = memory[j - 1];
                }

                memory[i + 1].start = memory[i].start + p->size;
                memory[i + 1].size = remaining;
                memory[i + 1].free = true;
                strcpy(memory[i + 1].pid, "");
                blockCount++;
            }

            return true;
        }
    }

    return false; // Not enough space
}

int main() {
    int totalMemory;
    printf("Enter total memory size (KB): ");
    scanf("%d", &totalMemory);
    initializeMemory(totalMemory);

    Process processes[MAX_PROCESSES] = {
        {"P1", 0, 100, false},
        {"P2", 1, 200, false},
        {"P3", 2, 300, false},
        {"P4", 3, 150, false},
        {"P5", 4, 80,  false},
        {"P6", 5, 220, false},
        {"P7", 6, 90,  false},
        {"P8", 7, 130, false},
        {"P9", 8, 250, false},
        {"P10", 9, 60, false}
    };

    int n = 10;
    int currentTime = 0;

    printf("\n📦 Simulating Dynamic Partitioning with FIFO Replacement...\n");

    while (1) {
        bool anyLeft = false;

        for (int i = 0; i < n; i++) {
            if (!processes[i].allocated && processes[i].arrival <= currentTime) {
                // Try to allocate; if it fails, run replacement
                while (!allocateMemory(&processes[i])) {
                    deallocateOldest();
                }

                displayMemory(totalMemory);
            }

            if (!processes[i].allocated)
                anyLeft = true;
        }

        if (!anyLeft) break;
        currentTime++;
    }

    return 0;
}
// This code simulates a dynamic partitioning memory management system with FIFO replacement.
// It initializes memory blocks, allocates memory to processes, and handles deallocation and merging of free blocks.