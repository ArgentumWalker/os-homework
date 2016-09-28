#include "innerTests.h"
#include "LIO.h"
#include "IDT.h"

void comPortTest() {
    outToCOMport("COM port test\n");
}

void interruptionsTest() {
    __asm__ volatile("int $99":::);
}

void pitTest() {
    setMasterPICInterruptions(~0x1);
}

void callTests() {
    int wait = 0;
    ///Here must be calls of tests
    comPortTest();
    while (wait);
    interruptionsTest(); 
    pitTest();
}

