#include "innerTests.h"
#include "LIO.h"
#include "IDT.h"

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

	doTests();

	while (1);
}
