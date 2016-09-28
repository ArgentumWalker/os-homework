#include <stdint.h>
#include "desc.h"
#include "memory.h"
#include "ioport.h"
#include "IDT.h"
#include "LIO.h"

#define INTERRUPT_GATE (uint64_t)0xe
#define MASTER_PIC_COMMAND_PORT 0x20
#define MASTER_PIC_DATA_PORT 0x21
#define SLAVE_PIC_COMMAND_PORT 0xA0
#define SLAVE_PIC_DATA_PORT 0xA1

struct IDTDesc{
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero0;     // free space
   uint8_t type_attr; // type and attributes
   uint16_t offset_2; // offset bits 16..31
   uint32_t offset_3; // offset bits 32..63
   uint32_t zero1;    // reserved
}__attribute__((packed));

extern uint64_t tbl[];
struct IDTDesc idt_table[100];
uint32_t PART = -1;

void initPIC() {
    out8(MASTER_PIC_COMMAND_PORT, 0x11);
    out8(MASTER_PIC_DATA_PORT, 32);
    out8(MASTER_PIC_DATA_PORT, 0x04);
    out8(MASTER_PIC_DATA_PORT, 0x01);
    out8(MASTER_PIC_DATA_PORT, 0xff);
    outToCOMport(">Master PIC initiated\n");
    out8(SLAVE_PIC_COMMAND_PORT, 0x11);
    out8(SLAVE_PIC_DATA_PORT, 40);
    out8(SLAVE_PIC_DATA_PORT, 0x02);
    out8(SLAVE_PIC_DATA_PORT, 0x01);
    out8(SLAVE_PIC_DATA_PORT, 0xff);
    outToCOMport(">Slave PIC initiated\n");
}

void initIDT() {
    for (int i = 0; i < 100; i++) {
        idt_table[i].offset_1 = tbl[i] & 0xffff;
        idt_table[i].selector = KERNEL_CS;
        idt_table[i].zero0 = 0;
        idt_table[i].type_attr = (1 << 7) | (INTERRUPT_GATE);
        idt_table[i].offset_2 = (tbl[i] >> 16) & 0xffff;
        idt_table[i].offset_3 = (tbl[i] >> 32) & 0xffffffff;
        idt_table[i].zero1 = 0;
    }
    struct desc_table_ptr ptr = {sizeof(idt_table) - 1, (uint64_t)(idt_table)};
    write_idtr(&ptr);
    outToCOMport("IDT initiated\n");
    initPIC();
    outToCOMport("PIC initiated\n");
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
void endOfInterruptionPIC() {
    out8(SLAVE_PIC_COMMAND_PORT, 0x20);
    out8(MASTER_PIC_COMMAND_PORT, 0x20);
}
