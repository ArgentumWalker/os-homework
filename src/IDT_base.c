#include <stdint.h>
#include "desc.h"
#include "memory.h"
#include "IDT.h"
#include "IDT_handlers.h"

#define i16 __uint128_t 
#define TYPE_F (uint64_t)0x0

extern uint64_t tbl[];
i16 idt_table[256];
uint32_t PART = -1;

void IDT_init() {
    for (int i = 0; i < 100; i++) {
        idt_table[i] = 0;
        idt_table[i] = ((i16)(tbl[i] >> 32) << 64) | ((i16)(((uint64_t)1 << (7+8)) | (TYPE_F << 6)) << 48) | ((i16)(tbl[i] & PART) << 16) | ((i16)(KERNEL_CS));
    }
    struct desc_table_ptr ptr = {(uint64_t)(idt_table), 100};
    write_idtr(&ptr);
}

#define call_handler(no) \
        if (hnum == no) {\
            handler##no();\
        }\

void idt_c_handler(uint64_t* stackPointer) {
    uint64_t hnum = *stackPointer;
    call_handler(0);
    call_handler(1);
    //Do nothing
}

