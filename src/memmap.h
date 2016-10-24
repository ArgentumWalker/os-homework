#ifndef _MEMMAP_H_
#define _MEMMAP_H_
#include <stdint.h>
extern uint32_t multibootHeader[];
extern uint64_t multibootInfo;
extern struct MultibootHeader* multibootHeaderPtr;
extern struct MultibootInfo* multibootInfoPtr;
extern struct MemoryMap actualMemoryMap;

//I like CamelCase, but documentation is documentation. Sorry :(
struct MultibootHeader {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
    uint32_t entry_addr;
}__attribute__((packed));
struct MultibootInfo {    
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
}__attribute__((packed));
struct MemoryMapNode {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    #define MEMORY_AVAILABLE 1
    #define MEMORY_RESERVED  2
    uint32_t type;
}__attribute__((packed));

#define MEMORY_MAP_MAX_SIZE 1024
struct MemoryMap {
    struct MemoryMapNode nodes[MEMORY_MAP_MAX_SIZE];
    uint32_t node_count;
};

void initMemoryMap();
void showMemoryMap();
#endif
