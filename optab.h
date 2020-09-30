#ifndef OPTAB
#define OPTAB

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

typedef struct _HASH_NODE {
	char mnemonic[10];
	int opcode;
	int opcode_len;
	struct _HASH_NODE *link;
} HASH_NODE;

int hash(char str[]);
void construct_optab(HASH_NODE *optab[20]);
void add_optab(HASH_NODE *optab[20], char mnemonic[10], int opcode, int opcode_len);
void opcodelist(HASH_NODE *optab[20]);
HASH_NODE *search_optab(HASH_NODE *optab[20], char mnemonic[10]); 

#endif
