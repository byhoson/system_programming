#include "estab.h"

void add_estab(ES_NODE *estab[20], char symbol[], int address, int length) {
	/*
	 * adds (symbol, address,length) into symtab
	 * Arguments:
	 * 	ES_NODE *estab[] - array of 20 linked lists
	 * 	char symbol[]
	 * 	int address - for ordinary symbols
	 *	int length - for control sections
	 */
	int bucket;

	/* make ES_NODE */
	ES_NODE* item = (ES_NODE*)malloc(sizeof(ES_NODE));
	strcpy(item->symbol,symbol);
	item->address = address;
	item->length = length;

	/* get bucket id */
	bucket = hash(symbol);
	
	/* push item in front of symtab */
	item->link = estab[bucket];
	estab[bucket] = item;
}

ES_NODE *search_estab(ES_NODE *estab[20], char symbol[]) {
	/* 
	 * searches the node containing the given symbol
	 * Arguments:
	 * 	ES_NODE *Estab[] - array of 20 linked lists
	 *	char symbol[] - the key for the search
	 * Returns:
	 * 	the pointer of the found node, and NULL if not found
	 */
	int bucket = hash(symbol);
	ES_NODE *curr = estab[bucket];
	while(curr) {
		if(!strcmp(symbol,curr->symbol)) {
			/* found */
			return curr;
		}
		curr = curr->link;
	}
	/* not found */
	return NULL;
}

void free_estab(ES_NODE *estab[20]) {
	/*
	 * free estab
	 * Arguments:
	 * 	ES_NODE *estab[]
	 */

	ES_NODE *curr, *tmp;
	
	for(int i=0; i<20; i++) {
		curr = estab[i];
		while(curr) {
			tmp = curr;
			curr = curr->link;
			free(tmp);
		}
		estab[i] = NULL;
	}
}

void print_estab(ES_NODE *estab[20]) {
	ES_NODE *curr;
	int arrow; /* flag for printing arrow */
	for(int i=0; i<20; i++) {
		curr = estab[i];
		printf("%d : ",i);
		arrow = 0;
		while(curr) {
			if(arrow) printf(" -> ");
			printf("[%s,%.2X, %.2X]", curr->symbol, curr->address, curr->length); 
			curr = curr->link;
			arrow = 1;
		}
		printf("\n");
	}	
}
