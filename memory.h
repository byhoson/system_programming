#ifndef MEM
#define MEM

#include <stdio.h>

void dump(unsigned char mem[65536][16], int start, int end);
void edit(unsigned char mem[65536][16], int address, int value);
void fill(unsigned char mem[65536][16], int start, int end, int value);
void reset(unsigned char mem[65536][16]);
unsigned int read_hb(unsigned char mem[65536][16], int address, int length);
void write_hb(unsigned char mem[65536][16], int address, int length, unsigned int value);
void modify_hb(unsigned char mem[65536][16], int address, int length, int opmode, unsigned int operand);

#endif
