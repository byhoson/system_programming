#ifndef ESTAB
#define ESTAB

#include <stdlib.h>
#include <string.h>

#include "optab.h"

typedef struct _ES_NODE {
	char symbol[80];
	int address;
	int length;
	struct _ES_NODE *link;
} ES_NODE;

void add_estab(ES_NODE *estab[20], char symbol[], int address, int length);
ES_NODE *search_estab(ES_NODE *estab[20], char symbol[]);
void print_estab(ES_NODE *estab[20]);
void free_estab(ES_NODE *estab[20]);

#endif
