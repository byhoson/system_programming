#include "20181643.h"

char cmd[80]; /* command string of form [cmd_name operand]*/
unsigned char mem[65536][16]; /* virtural memory of size 1MB */
NODE *head_history; 
HASH_NODE *optab[20];
SYMBOL *symtab[20]; 
SYMBOL *sym_list;
ES_NODE *estab[20];
BP_NODE breakpoint;
int PROGADDR = 0;
int EXECADDR = 0;
int PROGLTH = 0;
int start = -1;
int end = -1;
int reg[10];

int main() {

	int quit = 0;
	initialize();

	/* get cmd from the shell */
	while(!quit) {
		printf("sicsim> ");
		cmd[0] = '\0';
		scanf("%[^\n]s",cmd);
		getchar();
		if(strlen(cmd)>=80) printf("Command length is too long. Command length must be less than 80 characters.\n");
		else process_cmd(cmd, &quit);
	}

	return 0;

}

void process_cmd(char cmd[80], int *quit) {
	/*
	 * processes the given command by calling the corresponding submodules.
	 * Arguments:
	 *	char cmd[80] - the full command string given by the shell
	 *	int *quit - the quit flag required by main() 
	 */

	int idx; /* idx scans through the cmd in order to tokenize it */
	/* cmd is tokenized into the following 4 tokens - cmd_name, op1, op2, op3 */ 
	char cmd_name[80] = ""; 
	char op1[80] = "";
	char op2[80] = "";
	char op3[80] = "";
	int parsed_op; /* the numerical value parsed from the operand in char form */
	int address, value; /* for edit module */
	int fill_start, fill_end; /* for fill module */
	int valid=1;

	HASH_NODE *opnode;
	SYMBOL *symnode;

	
	/* parse the cmd of form (cmd_name, op1, op2, op3) */
	idx=0;
	get_token(cmd, cmd_name, &idx,80);
	get_token(cmd, op1, &idx,80);
	get_token(cmd, op2, &idx,80);
	get_token(cmd, op3, &idx,80);


	/* commands related to shell */	
	if(!strcmp(cmd_name,"h") || !strcmp(cmd_name,"help")) {
		help();
	} 
	else if(!strcmp(cmd_name,"d") || !strcmp(cmd_name,"dir")) {
		dir();
	} 
	else if(!strcmp(cmd_name,"q") || !strcmp(cmd_name,"quit")) {
		*quit = 1;
	}
	else if(!strcmp(cmd_name,"hi") || !strcmp(cmd_name,"history")) {
		/* push into history list in advance before printing the list */
		push_back(&head_history, cmd); 
		history(head_history);
	}
	else if(!strcmp(cmd_name,"type")) {
		type(op1);
	}


	/* commands related to memory */
	else if(!strcmp(cmd_name,"du") || !strcmp(cmd_name,"dump")) {
		/* get start */
		parsed_op = parse_op(op1); /* get hex */
		if(parsed_op==-1) start = end + 1; /* no operand */
		else if(parsed_op==-2 || strlen(op1)>5) valid = 0; /* invalid hex */
		else start = parsed_op; /* valid hex */
		if(start>0xFFFFF) start = 0; /* get back to 0 */

		/* get end */
		parsed_op = parse_op(op2); /* get hex */
		if(parsed_op==-1) end = (start+159 <= 0xFFFFF) ? start+159 : 0xFFFFF; /* no operand - boundary check */
		else if(parsed_op==-2 || strlen(op2)>5) valid = 0; /* invalid hex */
		else end = parsed_op; /* valid hex */
			
		/* assert 0 <= start <= end <= 0xFFFFF */
		if(!(0<=start && start<=0xFFFFF && start<=end && 0<=end && end <= 0xFFFFF)) valid =0;

		/* print error if op3 exists */
		if(strlen(op3)) {
			printf("too many operands!\n");
			return;
		}

		/* print error if invalid */
		if(!valid) {
			printf("invalid operands!\n");
			return;
		}

		dump(mem,start,end);
	}
	else if(!strcmp(cmd_name,"e") || !strcmp(cmd_name,"edit")) {
		/* get address */
		parsed_op = parse_op(op1); /* get hex */
		if(parsed_op<0 || strlen(op1)>5) valid = 0; /* invalid hex */
		else address = parsed_op; /* valid hex */
		
		/* get value */
		parsed_op = parse_op(op2); /* get hex */
		if(parsed_op<0 || strlen(op2)>2) valid = 0; /* invalid hex */
		else value = parsed_op; /* valid hex */

		/* print error if op3 exists */
		if(strlen(op3)) {
			printf("too many operands!\n");
			return;
		}

		/* print error if invalid */
		if(!valid) {
			printf("invalid operands!\n");
			return;
		}

		edit(mem, address, value);
	}
	else if(!strcmp(cmd_name,"f") || !strcmp(cmd_name,"fill")) {
		/* get start */
		parsed_op = parse_op(op1);
		if(parsed_op<0 || strlen(op1)>5) valid = 0; /* invalid hex */
		else fill_start = parsed_op; /* valid hex */

		/* get end */
		parsed_op = parse_op(op2);
		if(parsed_op<0 || strlen(op2)>5) valid = 0; /* invalid hex */
		else fill_end = parsed_op; /* valid hex */

		/* get value */
		parsed_op = parse_op(op3);
		if(parsed_op<0 || strlen(op3)>2) valid = 0; /* invalid hex */
		else value = parsed_op; /* valid hex */

		/* print error if invalid */
		if(!valid) {
			printf("invalid operands!\n");
			return;
		}

		fill(mem, fill_start, fill_end, value);

	}
	else if(!strcmp(cmd_name,"reset")) {
		reset(mem);
	}


	/* cmd related to optab */
	else if(!strcmp(cmd_name,"opcode")) {
		opnode = search_optab(optab, op1);
		if(opnode) { /* opcode found */
			printf("opcode is %.2X\n", opnode->opcode);
		} else {
			printf("can't find %s\n", op1);
		}
	}
	else if(!strcmp(cmd_name,"opcodelist")) {
		opcodelist(optab);
	}

	/* cmd related to assembler */
	else if(!strcmp(cmd_name,"assemble")) {
		assemble(op1,&sym_list,symtab,optab);		
	}
	else if(!strcmp(cmd_name,"symbol")) {
		//sort_sym(&sym_list, symtab);
		symnode = sym_list;
		while(symnode) {
			printf("        %-8s%.4X\n",symnode->label,symnode->address);
			symnode = symnode->link;
		}
		
	}

	/* cmd related to loader */
	else if(!strcmp(cmd_name,"progaddr")) {
		parsed_op = parse_op(op1);
		if(parsed_op>=0 && parsed_op <=0xFFFFF) PROGADDR = parsed_op;
		else printf("error! program address out of range!\n"); 	
	}
	else if(!strcmp(cmd_name,"loader")) {
		PROGLTH = loader(mem,estab,PROGADDR,&EXECADDR,op1,op2,op3);	
		for(int i=0; i<10; i++) reg[i]=0;
		reg[2] = PROGLTH;			
	}
	else if(!strcmp(cmd_name,"bp")) {
		if(!strcmp(op1,"clear")) {
			breakpoint.size=0;
			printf("\t\t[ok] clear all breakpoints\n");
		} else if(strlen(op1)) {
			parsed_op = parse_op(op1);
			if(PROGLTH<=0) {
				printf("error! program must be loaded..\n");
			} else if(parsed_op>=PROGLTH) {
				printf("breakpoint must be within the program range\n");
			} else bp(&breakpoint,parsed_op);
		} else {
			printf("breakpoint\n");
			printf("----------\n");
			for(int i=0; i<breakpoint.size; i++) printf("%X\n",breakpoint.arr[i]);
		}
	}
	else if(!strcmp(cmd_name,"run")) { 
		if(PROGLTH>0) {
			if(!breakpoint.size) reg[8] = EXECADDR;
			run(mem,PROGADDR,PROGLTH,reg,&breakpoint);
		} else {
			printf("error! program must be loaded..\n");
		}
	}	
	/* invalid command */
	else {
		if(strlen(cmd_name)) printf("command not found!\n");
		return;
	}


	/* push non-history cmd to the history list */
	if(strcmp(cmd_name,"hi") && strcmp(cmd_name,"history")) push_back(&head_history, cmd); 
}

void initialize() {
	/* initializes head_history and optab */
	head_history = NULL;
	for(int i=0; i<20; i++) {
		optab[i] = NULL;
		symtab[i] = NULL;
	}
	construct_optab(optab);
	breakpoint.size=0;
}
