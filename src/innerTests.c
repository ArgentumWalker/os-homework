#include "innerTests.h"
#include "LIO.h"
#include "IDT.h"
#include "PIT.h"

void comPortTest() {
    outToCOMport("COM port test\n");
}

void interruptionsTest() {
    __asm__ volatile("int $99":::);
}

void pitTest() {
    outToCOMport("Start PIT test\n");
    setMasterPICInterruptions(~0x01);
    initPIT();
}

void callTests() {
    int wait = 0;
    ///Here must be calls of tests
    comPortTest();
    while (wait);
    interruptionsTest(); 
    pitTest();
}

