#include "LIO.h"

#define port_pos 0x3f8

void init() {
    out8(port_pos + 3, 0);
    out8(port_pos + 1, 0);
    out8(port_pos + 3, 1);
    out8(port_pos + 0, -1);
    out8(port_pos + 1, -1);
    out8(port_pos + 3, (~1));
}

void out(char[] c) {
    for (char* ch = c; *ch != 0; ch++) {
        out8(port_pos + 0, *ch);
    }
}
