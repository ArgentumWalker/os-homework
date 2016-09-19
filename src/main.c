#include "innerTests.h"
#include "LIO.h"

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

	struct desc_table_ptr ptr = {0, 0};

	write_idtr(&ptr);

	doTests();

	while (1);
}
