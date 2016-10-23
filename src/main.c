#include "innerTests.h"
#include "LIO.h"
#include "IDT.h"
#include "PIT.h"
#include "memmap.h"

static void doTests(void)
{
#ifdef DEBUG
    callTests();
#endif
}

#include <desc.h>

void main(void)
{
    initCOMport();
	initIDT();
    initPIT();
    enableInterruptions();
    initMemoryMap();
    
    showMemoryMap();    
	doTests();

	while (1);
}
