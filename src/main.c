#include <innerTests.c>

static void qemu_gdb_hang(void)
{
#ifdef DEBUG
    callTests();    
#endif
}

#include <desc.h>

void main(void)
{
	qemu_gdb_hang();

	struct desc_table_ptr ptr = {0, 0};

	write_idtr(&ptr);

	while (1);
}
