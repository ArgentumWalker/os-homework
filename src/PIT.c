#include "PIT.h"
#include "ioport.h"

#define PIT_FREQ 1193182
#define RATE_GENERATOR_MODE 0x2
#define SQUAREWAVE_GENERATOR_MODE 0x3
#define SW_TRIGGERED_STROBE_MODE 0x4
#define HW_TRIGGERED_STROBE_MODE 0x5
#define PIT_COMMAND_PORT 0x43
#define PIT_DATA_PORT 0x40

void initPIT() {
    out8(PIT_COMMAND_PORT, (0x3 << 4) | (RATE_GENERATOR_MODE << 1));
    setPitFreq(20);
}

void setPitFreq(uint32_t freq) {
    if (freq < 20) freq = 20;
    uint16_t d = (PIT_FREQ / freq);
    out8(PIT_DATA_PORT, d & 0xff);
    out8(PIT_DATA_PORT, (d >> 8) & 0xff);
}
