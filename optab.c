#include "optab.h"

int hash(char str[]) {
	/*
	 * the hash function modulo 20
	 * Arguments:
	 * 	char str[] - the string on which the hash is computed
	 * Returns:
	 * 	the bucket id within the range 0 ~ 19 (total of 20 buckets)
	 */
	const int len = strlen(str);
	int ret = 0;
	for(int i=0; i<len; i++) ret += (int)str[i]; 
	return ret % 20;
}

void construct_optab(HASH_NODE *optab[20]) {
	/*
	 * constructs the optab
	 * Arguments:
	 *	HASH_NODE *optab[] - array of 20 linked lists
	 */
	int idx;
	char line[100];
	char tok1[20]; char tok2[20]; char tok3[20];
	FILE *fp = fopen("opcode.txt","r"); 

	/* file error handling */
	if(!fp) {
		fprintf(stderr,"opcode.txt not found!\n");
		exit(1);
	}

	/* read opcode.txt and add each line to optab */
	while(fscanf(fp,"%[^\n]s",line)!=EOF) {
		fgetc(fp);

		/* parse line */
		idx=0;
		get_token(line, tok1, &idx, 100); /* opcode */
		get_token(line, tok2, &idx, 100); /* mnemonic */
		get_token(line, tok3, &idx, 100); /* opcode_len */

		/* add line to optab */
		add_optab(optab, tok2, parse_op(tok1), (int)(tok3[0]-'0')); 	
	}
	fclose(fp);
}

void add_optab(HASH_NODE *optab[20], char mnemonic[10], int opcode, int opcode_len) {
	/*
	 * adds (opcode, mnemonic) into optab
	 * Arguments:
	 * 	HASH_NODE *optab[] - array of 20 linked lists
	 * 	char mnemonic[]
	 * 	int opcode
	 * 	int opcode_len
	 */
	int bucket;

	/* make HASH_NODE */
	HASH_NODE* item = (HASH_NODE*)malloc(sizeof(HASH_NODE));
	strcpy(item->mnemonic,mnemonic);
	item->opcode = opcode;
	item->opcode_len = opcode_len;

	/* get bucket id */
	bucket = hash(mnemonic);
	
	/* push item in front of optab */
	item->link = optab[bucket];
	optab[bucket] = item;
}

void opcodelist(HASH_NODE *optab[20]) {
	/*
	 * prints out the whole optab
	 * Arguments:
	 * 	HASH_NODE *optab[] - array of 20 linked lists
	 */
	HASH_NODE *curr;
	int arrow; /* flag for printing arrow */
	for(int i=0; i<20; i++) {
		curr = optab[i];
		printf("%d : ",i);
		arrow = 0;
		while(curr) {
			if(arrow) printf(" -> ");
			printf("[%s,%.2X]", curr->mnemonic, curr->opcode); 
			curr = curr->link;
			arrow = 1;
		}
		printf("\n");
	}	
}

HASH_NODE *search_optab(HASH_NODE *optab[20], char mnemonic[10]) {
	/* 
	 * searches the node containing the given mnemonic
	 * Arguments:
	 * 	HASH_NODE *optab[] - array of 20 linked lists
	 *	char mnemonic[] - the key for the search
	 */
	int bucket = hash(mnemonic);
	HASH_NODE *curr = optab[bucket];
	while(curr) {
		if(!strcmp(mnemonic,curr->mnemonic)) {
			/* mnemonic found in optab */
			return curr;
		}
		curr = curr->link;
	}

	/* mnemonic not found in optab */
	return NULL;
}
