#ifndef SYMTAB
#define SYMTAB

#include <stdlib.h>
#include <string.h>

#include "optab.h" /* hash */

typedef struct _SYMBOL {
	char label[80];
	int address;
	struct _SYMBOL *link;	
} SYMBOL;

void free_symtab(SYMBOL *symtab[20]);
void free_sym_list(SYMBOL **sym_list);
void add_symtab(SYMBOL *symtab[20], char label[80], int address);
SYMBOL *search_symtab(SYMBOL *symtab[20], char label[80]);
void sort_sym(SYMBOL **sym_list, SYMBOL *symtab[20]);
#endif
