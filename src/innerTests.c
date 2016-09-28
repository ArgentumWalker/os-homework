#include "innerTests.h"
#include "LIO.h"

void comPortTest() {
    outToCOMport("COM port test\n");
}

void interruptionsTest() {
    outToCOMport("Interruption test begins\n");
    __asm__ volatile("int %0\n"::"N"((30)));
}

void callTests() {
    int wait = 0;
    ///Here must be calls of tests
    comPortTest();
    while (wait);
    interruptionsTest(); 
}

