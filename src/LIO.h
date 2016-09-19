#ifndef __LIO_H__
#define __LIO_H__

#include <stdint.h>
#include "ioport.h"

void initCOMport();
void outToCOMport(char* c);

#endif
