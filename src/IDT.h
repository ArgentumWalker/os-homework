#ifndef __IDT_H__
#define __IDT_H__

void initIDT();

void disableInterruptions();
void enableInterruptions();

void setMasterPICInterruptions(char mask); 
void setSlavePICInterruptions(char mask);

#endif
