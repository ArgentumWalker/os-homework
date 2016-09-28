#ifndef __IDT_H__
#define __IDT_H__
#include <stdint.h>

struct InterruptionStackframe {
    //registers
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    //other information
    uint64_t err_code;
    uint64_t idt_position;
}__attribute__((packed));

void initIDT();

void disableInterruptions();
void enableInterruptions();

void setMasterPICInterruptions(char mask); 
void setSlavePICInterruptions(char mask);

#endif
