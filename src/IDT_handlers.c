#include <stdint.h>

void handler0() {
    //not implemented
}

void handler1() {
    //not implemented
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


