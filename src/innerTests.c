#include "innerTests.h"
#include "LIO.h"

void comPortTest() {
    outToCOMport("COM port test\n");
}

void interruptionsTest() {
    __asm__ volatile("int $30":::);
}

void callTests() {
    int wait = 0;
    ///Here must be calls of tests
    comPortTest();
    while (wait);
    interruptionsTest(); 
}

