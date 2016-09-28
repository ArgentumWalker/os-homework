#include "innerTests.h"
#include "LIO.h"
#include "IDT.h"
#include "PIT.h"

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
        
	doTests();

	while (1);
}
