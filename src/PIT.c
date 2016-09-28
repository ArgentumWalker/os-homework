#include "PIT.h"
#include "ioport.h"

#define PIT_FREQ 1193182
#define RATE_GENERATOR_MODE 0x2
#define PIT_COMMAND_PORT 0x43
#define PIT_DATA_PORT 0x40

void initPit() {
    out8(PIT_COMMAND_PORT, (RATE_GENERATOR_MODE << 5) | (0x3 << 3));
    setPitFreq(1)
}

void setPitFreq(uint16_t freq) {
    uint16_t d = (PIT_FREQ / freq);
    out8(PIT_DATA_PORT, d & 0xff);
    out8(PIT_DATA_PORT, (d >> 8) & 0xff);
}
