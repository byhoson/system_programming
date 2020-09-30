#ifndef ASSEM
#define ASSEM

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symtab.h"
#include "utils.h"

typedef struct _M_REC {
	int address;
	int length;
	struct _M_REC *link;
} M_REC;

void assemble(char filename[80], SYMBOL **sym_list, SYMBOL *symtab[20], HASH_NODE *optab[20]);
int pass1(char filename[80], SYMBOL *symtab[20], HASH_NODE *optab[20], int *prog_len);
int pass2(char prefix[80], SYMBOL *symtab[20], HASH_NODE *optab[20], int prog_len);
#endif
