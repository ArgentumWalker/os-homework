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
    outToCOMport("Tick-tock...\n");
}

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
    //Do nothing
}


