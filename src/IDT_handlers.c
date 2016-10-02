#include <stdint.h>
#include "LIO.h"
#include "IDT.h"

void handler0() {
    //not implemented
}

void handler1() {
    //not implemented
}

//PIT interruption handler
void handler32() {
    static uint8_t i = 0;
    if (++i == 20) {
        outToCOMport("Tick-tock...\n");
        i = 0;
    }
}

//Test interruption handler
void handler99() {
    outToCOMport("Interruption test\n");
}

#define call_handler(no) \
        if (ptr->idt_position == no) {\
            handler##no();\
        }\

void idt_c_handler(struct InterruptionStackframe* ptr) {
    call_handler(0);
    call_handler(1);
    call_handler(32);
    call_handler(99);
    if (ptr->idt_position > 31 && ptr->idt_position < 48) {
        endOfInterruptionPIC();
    }
}


