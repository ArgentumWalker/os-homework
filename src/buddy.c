#include "buddy.h"
#include "memmap.h"
#include "memory.h"
#include "LIO.h"

#define MAGIC1 0x42424242
#define MAGIC2 0x66613666
#define bool int
#define true 1
#define false 0


struct BorderTag {
    uint64_t  magic1;
    uintptr_t pairBorderTag;
    uint64_t  state;
    uint64_t  inListPosition;
    uint64_t  length;
    uint64_t  magic2;
}__attribute__((packed));


uint64_t pow2[20];
#define MAX_PARTITION_ELEMENTS 2048
uintptr_t freeSpaces[MAX_PARTITION_ELEMENTS];
uint64_t fsCount;

void initBuddy() {
    pow2[0] = 1;
    for (unsigned int i = 1; i < 20; i++) {
        pow2[i] = pow2[i-1]*2;
    }
    fsCount = 0;
    for (unsigned int i = 0; i < actualMemoryMap.node_count; i++) {
        if (actualMemoryMap.nodes[i].type == MEMORY_RESERVED) {
            continue;
        }
        uintptr_t begin = actualMemoryMap.nodes[i].base_addr;
        uintptr_t length = actualMemoryMap.nodes[i].length;
        printf("MemapNode %u with %llx (+ %llx), %llx\n", i, begin, length, begin + length -1);
        //Allign
        if (begin % PAGE_SIZE) {
            length = length - (PAGE_SIZE - begin % PAGE_SIZE);
            begin = begin + (PAGE_SIZE - begin % PAGE_SIZE);            
        }
        if (length % PAGE_SIZE) {
            length = length - (length % PAGE_SIZE);
        }
        //write BorderTags
        struct BorderTag* headBorderTag = (struct BorderTag*)begin;
        struct BorderTag* tailBorderTag = (struct BorderTag*)(begin + length - sizeof(struct BorderTag));
        headBorderTag->magic1 = tailBorderTag->magic1 = MAGIC1;
        headBorderTag->magic2 = tailBorderTag->magic2 = MAGIC2;
        headBorderTag->state = tailBorderTag->state = MEMORY_AVAILABLE;
        headBorderTag->inListPosition = tailBorderTag->inListPosition = fsCount;
        headBorderTag->length = tailBorderTag->length = length;
        headBorderTag->pairBorderTag = (uintptr_t)tailBorderTag;
        tailBorderTag->pairBorderTag = (uintptr_t)headBorderTag;
        freeSpaces[fsCount++] = begin; 
    }
}
void* buddyAlloc(uintptr_t size) {
    unsigned int k = 0;
    while (size < pow2[k]*PAGE_SIZE - 2*sizeof(struct BorderTag)) {
        k++;
    }
    for (unsigned int i = 0; i < fsCount; i++) {
        struct BorderTag* head = (struct BorderTag*)freeSpaces[fsCount];
        struct BorderTag* tail = (struct BorderTag*)(head->pairBorderTag);
        if (head->length >= pow2[k]*PAGE_SIZE + 2*sizeof(struct BorderTag)) {
            struct BorderTag* ntail = (struct BorderTag*)(freeSpaces[fsCount] + pow2[k]*PAGE_SIZE - sizeof(struct BorderTag));
            struct BorderTag* nhead = (struct BorderTag*)(freeSpaces[fsCount] + pow2[k]*PAGE_SIZE);
            nhead->magic1 = ntail->magic1 = MAGIC1;
            nhead->magic2 = ntail->magic2 = MAGIC2;
            
            head->length = ntail->length = pow2[k]*PAGE_SIZE;
            nhead->length = tail->length = tail->length - pow2[k]*PAGE_SIZE;
            head->state = ntail->state = MEMORY_RESERVED;
            nhead->state = tail->state = MEMORY_AVAILABLE;
            nhead->inListPosition = tail->inListPosition;
            head->pairBorderTag = (uintptr_t)ntail;
            tail->pairBorderTag = (uintptr_t)nhead;
            nhead->pairBorderTag = (uintptr_t)tail;
            ntail->pairBorderTag = (uintptr_t)head;
            freeSpaces[i] = (uintptr_t)nhead;
            return (void*) (head + 1);
        }
        if (head->length >= pow2[k]*PAGE_SIZE) {
            freeSpaces[i] = freeSpaces[--fsCount];
            head->state = tail->state = MEMORY_RESERVED;
            return (void*) (head + 1);
        }
    }
    return NULL;
}
void buddyFree(void* ptr) {
    struct BorderTag* mhead = (struct BorderTag*)ptr;
    struct BorderTag* mtail = (struct BorderTag*)(mhead -> pairBorderTag);
    struct BorderTag* ltail = mhead - 1;
    struct BorderTag* rhead = mtail + 1;
    mtail->state = MEMORY_AVAILABLE;
    mhead->state = MEMORY_AVAILABLE;
    mtail->inListPosition = mhead->inListPosition = fsCount++;
    bool merged = false;
    if (ltail -> magic1 == MAGIC1 && ltail -> magic2 == MAGIC2) {
        struct BorderTag* lhead = (struct BorderTag*)(ltail->pairBorderTag);
        if (lhead->magic1 == MAGIC1 && lhead->magic2 == MAGIC2 && mtail->magic1 == MAGIC1 && mtail->magic2 == MAGIC2) {
            if (lhead->state == MEMORY_AVAILABLE) {
                lhead->length = mtail->length = lhead->length + mtail->length;
                lhead->pairBorderTag = (uintptr_t)mtail;
                mtail->pairBorderTag = (uintptr_t)lhead; fsCount--;
                freeSpaces[lhead->inListPosition] = freeSpaces[fsCount - 1];
                freeSpaces[fsCount - 1] = (uintptr_t)lhead;
                mtail->inListPosition = lhead->inListPosition = fsCount - 1;
                mhead = lhead;
                merged = true;
            }
        }
    }
    if (rhead -> magic1 == MAGIC1 && rhead -> magic2 == MAGIC2) {
        struct BorderTag* rtail = (struct BorderTag*)(rhead->pairBorderTag);
        if (rtail->magic1 == MAGIC1 && rtail->magic2 == MAGIC2 && mhead->magic1 == MAGIC1 && mhead->magic2 == MAGIC2) {
            if (rtail->state == MEMORY_AVAILABLE) {
                rtail->length = mhead->length = rtail->length + mhead->length;
                rtail->pairBorderTag = (uintptr_t)mhead;
                mhead->pairBorderTag = (uintptr_t)rtail; fsCount--;
                freeSpaces[rtail->inListPosition] = freeSpaces[fsCount - 1];
                freeSpaces[fsCount - 1] = (uintptr_t)mhead;
                mhead->inListPosition = rtail->inListPosition = fsCount - 1;
                mtail = rtail;
                merged = true;
            }
        }   
    }
    if (!merged) {
        freeSpaces[fsCount] = (uintptr_t)mhead;
        mhead->inListPosition = mtail->inListPosition = fsCount++;
    }
}

