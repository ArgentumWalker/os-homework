#ifndef _BUDDY_H_
#define _BUDDY_H_
#include <stdint.h>

#define NULL 0x0000

void initBuddy();
void* buddyAlloc(uintptr_t size);
void buddyFree(void* ptr);
#endif
