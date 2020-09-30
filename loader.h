#ifndef LOADER
#define LOADER

#include <string.h>
#include <stdio.h>

#include "memory.h"
#include "utils.h"
#include "estab.h"

int loader(unsigned char mem[65546][16],ES_NODE *estab[20], int PROGADDR, int *EXECADDR, char filename1[50], char filename2[50], char filename3[50]);
int loader1(ES_NODE *estab[20], int PROGADDR, int *PROGLTH, char filename1[50], char filename2[50], char filename3[50]);
int loader2(unsigned char mem[65536][16],ES_NODE *estab[20], int PROGADDR, char filename1[50], char filename2[50], char filename3[50]);

#endif
