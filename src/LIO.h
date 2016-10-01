#ifndef __LIO_H__
#define __LIO_H__

#include <stdint.h>
#include <stdarg.h>
#include "ioport.h"

void initCOMport();
void outToCOMport(char* c);

int printf(const char* format, ...);
int vprintf(const char* format, va_list args);
int snprintf(char* s, uint64_t n, const char* format, ...);
int vsnprintf(char* s, uint64_t n, const char* format, va_list args);

#endif
