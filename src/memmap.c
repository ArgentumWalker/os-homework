#include "memmap.h"
#include "LIO.h"
#include "buddy.h"

struct MultibootHeader* multibootHeaderPtr;
struct MultibootInfo* multibootInfoPtr;
struct MemoryMap actualMemoryMap;

void addNodeToMemmap(struct MemoryMap* memmap, struct MemoryMapNode node) {
    memmap->nodes[memmap->node_count++] = node;
}

struct MemoryMapNode makeNode(uintptr_t begin, uintptr_t length, uint32_t type) {
    struct MemoryMapNode node;
    node.base_addr = begin;
    node.length = length;
    node.size = 0;
    node.type = type;
    return node;
}

void initMemoryMap() {
    multibootHeaderPtr = (struct MultibootHeader*)multibootHeader;
    multibootInfoPtr = (struct MultibootInfo*)multibootInfo;
    actualMemoryMap.node_count = 0;
    
    //Put Kernel to MemMap
    uintptr_t kernelBegin = multibootHeaderPtr->load_addr;
    uintptr_t kernelLength = multibootHeaderPtr->bss_end_addr - kernelBegin + 1;
    addNodeToMemmap(&actualMemoryMap, makeNode(kernelBegin, kernelLength,MEMORY_RESERVED));
    uintptr_t pos = multibootInfoPtr->mmap_addr;
    while (pos < multibootInfoPtr->mmap_addr + multibootInfoPtr->mmap_length) {
        struct MemoryMapNode* currentNode = (struct MemoryMapNode*) pos;
        pos += currentNode->size + sizeof(currentNode->size);
        if (kernelBegin > currentNode->base_addr + currentNode->length - 1 || kernelBegin + kernelLength - 1 < currentNode->base_addr) {
            addNodeToMemmap(&actualMemoryMap, *currentNode);
            continue;
        }
        if (kernelBegin > currentNode->base_addr) {
            addNodeToMemmap(&actualMemoryMap, makeNode(currentNode->base_addr, kernelBegin - currentNode->base_addr, currentNode->type));
        }
        if (kernelBegin + kernelLength < currentNode->base_addr + currentNode->length) {
            addNodeToMemmap(&actualMemoryMap, makeNode(kernelBegin + kernelLength, currentNode->base_addr + currentNode->length - (kernelBegin + kernelLength), currentNode->type));
        }
    }
    initBuddy();
}

void showMemoryMap() {
    printf("Multiboot_header: (addr %llx)\n", (uint64_t)multibootHeaderPtr);
    printf(">magic:         %lx\n", multibootHeaderPtr->magic);
    printf(">flags:         %lx\n", multibootHeaderPtr->flags);
    printf(">checksum:      %lx\n", multibootHeaderPtr->checksum);
    printf(">header_addr:   %lx\n", multibootHeaderPtr->header_addr);
    printf(">load_addr:     %lx\n", multibootHeaderPtr->load_addr);
    printf(">load_end_addr: %lx\n", multibootHeaderPtr->load_end_addr);
    printf(">bss_end_addr:  %lx\n", multibootHeaderPtr->bss_end_addr);
    printf(">entry_addr:    %lx\n", multibootHeaderPtr->entry_addr);
    printf("\n\n");
    printf("Multiboot_info: (addr %llx)\n", (uint64_t)multibootInfoPtr);
    printf(">flags:             %lx\n", multibootInfoPtr->flags);
    printf(">mem_lower:         %lx\n", multibootInfoPtr->mem_lower);
    printf(">mem_upper:         %lx\n", multibootInfoPtr->mem_upper);
    printf(">boot_device:       %lx\n", multibootInfoPtr->boot_device);
    printf(">cmdline:           %lx\n", multibootInfoPtr->cmdline);
    printf(">mods_count:        %lx\n", multibootInfoPtr->mods_count);
    printf(">mods_addr:         %lx\n", multibootInfoPtr->mods_addr);
    printf(">syms: (addr %llx)\n",  (uint64_t)multibootInfoPtr->syms);
    for (int i = 0; i < 4; i++) {
        printf(">>syms[%d]:          %lx\n", i, multibootInfoPtr->syms[i]);
    }
    printf(">mmap_length:       %lx\n", multibootInfoPtr->mmap_length);
    printf(">mmap_addr:         %lx\n", multibootInfoPtr->mmap_addr);
    printf(">drives_length:     %lx\n", multibootInfoPtr->drives_length);
    printf(">drives_addr:       %lx\n", multibootInfoPtr->drives_addr);
    printf(">config_table:      %lx\n", multibootInfoPtr->config_table);
    printf(">boot_loader_name:  %lx\n", multibootInfoPtr->boot_loader_name);
    printf(">apm_table:         %lx\n", multibootInfoPtr->apm_table);
    printf(">vbe_control_info:  %lx\n", multibootInfoPtr->vbe_control_info);
    printf(">vbe_mode_info:     %lx\n", multibootInfoPtr->vbe_mode_info);
    printf(">vbe_mode:          %x\n", (unsigned int)multibootInfoPtr->vbe_mode);
    printf(">vbe_interface_seg: %x\n", (unsigned int)multibootInfoPtr->vbe_interface_seg);
    printf(">vbe_interface_off: %x\n", (unsigned int)multibootInfoPtr->vbe_interface_off);
    printf(">vbe_interface_len: %x\n", (unsigned int)multibootInfoPtr->vbe_interface_len);
    printf("\n\nMemmapNodes: \n");
    for (unsigned int i = 0; i < actualMemoryMap.node_count; i++) {
        printf(">node %u\n", i);
        printf(">>addr:     %llx\n", actualMemoryMap.nodes[i].base_addr);
        printf(">>length:   %llx\n", actualMemoryMap.nodes[i].length); 
    }
}
