#include <stdint.h>
#include "desc.h"
#include "memory.h"
#include "ioport.h"
#include "IDT.h"

#define INTERRUPT_GATE (uint64_t)0xe
#define MASTER_PIC_COMMAND_PORT 0x20
#define MASTER_PIC_DATA_PORT 0x21
#define SLAVE_PIC_COMMAND_PORT 0xA0
#define SLAVE_PIC_DATA_PORT 0xA1

struct IDTDesc{
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t type_attr; // type and attributes
   uint16_t offset_2; // offset bits 16..31
   uint32_t offset_3; // offset bits 32..63
   uint32_t zero;     // reserved
}__attribute__((__packed__ ));

extern uint64_t tbl[];
struct IDTDesc idt_table[100];
uint32_t PART = -1;

void initPIC() {
    out8(MASTER_PIC_COMMAND_PORT, 0x11);
    out8(MASTER_PIC_DATA_PORT, 32);
    out8(MASTER_PIC_DATA_PORT, 0x04);
    out8(MASTER_PIC_DATA_PORT, 0x01);
    out8(SLAVE_PIC_COMMAND_PORT, 0x11);
    out8(SLAVE_PIC_COMMAND_PORT, 40);
    out8(SLAVE_PIC_COMMAND_PORT, 0x02);
    out8(SLAVE_PIC_COMMAND_PORT, 0x01);
}

void initIDT() {
    for (int i = 0; i < 100; i++) {
        idt_table[i].offset_1 = tbl[i];
        idt_table[i].selector = KERNEL_CS;
        idt_table[i].ist = 1 << 7;
        idt_table[i].type_attr = INTERRUPT_GATE << 4;
        idt_table[i].offset_2 = tbl[i] >> 16;
        idt_table[i].offset_3 = tbl[i] >> 32;
        idt_table[i].zero = 0;
    }
    struct desc_table_ptr ptr = {(uint64_t)(idt_table), sizeof(struct IDTDesc)*100 - 1};
    write_idtr(&ptr);
    initPIC();
}

void disableInterruptions() {
    __asm__ volatile("cli":::"cc");
}
void enableInterruptions() {
    __asm__ volatile("sti":::"cc");
}

void setMasterPICInterruptions(char mask) {
    out8(MASTER_PIC_DATA_PORT, mask);
}
void setSlavePICInterruptions(char mask) {
    out8(SLAVE_PIC_DATA_PORT, mask);
}

