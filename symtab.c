#include "symtab.h"

void free_symtab(SYMBOL *symtab[20]) {
	/*
	 * free symtab
	 * Arguments:
	 * 	SYMBOL *symtab[]
	 */

	SYMBOL *curr, *tmp;
	
	for(int i=0; i<20; i++) {
		curr = symtab[i];
		while(curr) {
			tmp = curr;
			curr = curr->link;
			free(tmp);
		}
		symtab[i] = NULL;
	}
}

void free_sym_list(SYMBOL **sym_list) {
	/*
	 * free sym_list
	 * Arguments:
	 * 	SYMBOL **sym_list
	 */

	SYMBOL *curr, *tmp;
	
	curr = *sym_list;
	while(curr) {
		tmp = curr;
		curr = curr->link;
		free(tmp);
	}

	*sym_list = NULL;	
}

void sort_sym(SYMBOL **sym_list, SYMBOL *symtab[20]) {	
	/*
	 * sort sym_list
	 * Arguments:
	 * 	SYMBOL **sym_list
	 * 	SYMBOL *symtab[]
	 */
	SYMBOL *curr, *item;
	char tmp_label[80];
	int tmp_address;
	int len=0;
	int is_first=1;	
	int idx=0;

	/* flatten symbols */
	for(int i=0; i<20; i++) {
		/* copy symbol in front of sym_list */
		curr = symtab[i];
		while(curr) {
			/* copy curr into item */
			item = (SYMBOL *)malloc(sizeof(SYMBOL));
			strcpy(item->label,curr->label);
			item->address = curr->address;

			/* insert item in fromt of sym_list */
			item->link = *sym_list;
			*sym_list = item;

			curr = curr->link;
		}
	}

	/* bubble sort sym_list */
	if(!(*sym_list)) return;
	while(is_first || len) {
		curr = *sym_list;
		if(is_first) len = 1;
		idx=0;
		while((is_first&&curr->link) || (!is_first&&idx<len)) {
			if(strcmp(curr->label,curr->link->label)>0) {
				/* swap labels */
				strcpy(tmp_label,curr->label);
				strcpy(curr->label,curr->link->label);
				strcpy(curr->link->label,tmp_label);
				/* swap address */
				tmp_address = curr->address;
				curr->address = curr->link->address;
				curr->link->address = tmp_address;
			}
			curr = curr->link;
			idx++;
			if(is_first) len++;
		}
		is_first = 0;
		if(!is_first) len--;
	}
}

void add_symtab(SYMBOL *symtab[20], char label[80], int address) {
	/*
	 * adds (label, address) into symtab
	 * Arguments:
	 * 	SYMTAB *symtab[] - array of 20 linked lists
	 * 	char label[]
	 * 	int address - LOCCTR of the label
	 */
	int bucket;

	/* make SYMBOL */
	SYMBOL* item = (SYMBOL*)malloc(sizeof(SYMBOL));
	strcpy(item->label,label);
	item->address = address;

	/* get bucket id */
	bucket = hash(label);
	
	/* push item in front of symtab */
	item->link = symtab[bucket];
	symtab[bucket] = item;

}

SYMBOL *search_symtab(SYMBOL *symtab[20], char label[80]) {
	/* 
	 * searches the node containing the given label
	 * Arguments:
	 * 	SYMBOL *symtab[] - array of 20 linked lists
	 *	char label[] - the key for the search
	 * Returns:
	 * 	the pointer of the found symbol, and NULL if not found
	 */
	int bucket = hash(label);
	SYMBOL *curr = symtab[bucket];
	while(curr) {
		if(!strcmp(label,curr->label)) {
			/* found */
			return curr;
		}
		curr = curr->link;
	}
	/* not found */
	return NULL;
}


