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

void printfTest() {
    printf("Printf: %d %c %ho %s \n", 42, 'V', 177, "Hello");
}

void snprintfTest() {
    char str[20]; str[0] = 0;
    snprintf(str, 20, "Snprintf: %s %lld", "Test", (unsigned long long)111111111);
    printf("%s \n", str);
}

void callTests() {
    int wait = 0;
    ///Here must be calls of tests
    comPortTest();
    while (wait);
    interruptionsTest();
    printfTest();
    snprintfTest(); 
    pitTest();
}


