#include "LIO.h"

#define PORT_POS 0x3f8

void initCOMport() {
    out8(PORT_POS + 3, 0x00);
    out8(PORT_POS + 1, 0x00); //Disable Interrupts
    out8(PORT_POS + 3, 0x80);
    out8(PORT_POS + 0, 0xef); //Maximal value
    out8(PORT_POS + 1, 0xef);
    out8(PORT_POS + 3, 0x5f); //without 7nth bit
}

void outToCOMport(char* c) {
    for (char* ch = c; *ch != 0; ch++) {
        out8(PORT_POS + 0, *ch);
    }
}
