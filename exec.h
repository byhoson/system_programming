#ifndef EXEC
#define EXEC

#include "memory.h"

typedef struct {
	int arr[100];
	int size;
} BP_NODE;


void run(unsigned char mem[65536][16], int PROGADDR, int PROGLTH, int reg[10], BP_NODE *breakpoint);
void bp(BP_NODE *breakpoint, int address);

#endif
