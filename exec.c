#include "exec.h"

void bp(BP_NODE *breakpoint, int address) {
	breakpoint->arr[breakpoint->size] = address;
	(breakpoint->size)++;
	printf("\t\t[ok] create breakpoint %X\n",address);
}


void run(unsigned char mem[65536][16], int PROGADDR, int PROGLTH, int reg[10], BP_NODE *breakpoint) {
	/* A:0 , X:1 , L:2 , B:3 , S:4 , T:5 , F:6 , PC:8 , SW:9 */
	unsigned int buffer,inst,opcode,reg1,reg2,addr;
	int n,i,x,b,p,e; /* bit flags */
	int quit=0;
	

	for(int j=0; j<breakpoint->size; j++) {
		if(reg[8] == PROGADDR + breakpoint->arr[j]) {
			quit=1;
			printf("-------------------------\n");
			printf("A : %.6X  X : %.6X\n",reg[0],reg[1]);
			printf("L : %.6X PC : %.6X\n",reg[2],reg[8]);
			printf("B : %.6X  S : %.6X\n",reg[3],reg[4]);
			printf("T : %.6X\n",reg[5]);
			printf("\t\tStop at checkpoint[%X]\n",breakpoint->arr[j]);
			breakpoint->arr[j] = -1;
		}
	}
	if(reg[8]>=PROGADDR+PROGLTH) {
		quit=1;
		printf("-------------------------\n");
		printf("A : %.6X  X : %.6X\n",reg[0],reg[1]);
		printf("L : %.6X PC : %.6X\n",reg[2],reg[8]);
		printf("B : %.6X  S : %.6X\n",reg[3],reg[4]);
		printf("T : %.6X\n",reg[5]);
		printf("\t\tEnd Program\n");
		breakpoint->size=0;
	}

	while (!quit) {
		inst = read_hb(mem,reg[8],8);
		opcode = (inst & 0xFC000000) >> 24;
		reg1 = (inst & 0x00F00000) >> 20;
		reg2 = (inst & 0x000F0000) >> 16;

		n = (inst & 0x02000000) >> 25;
		i = (inst & 0x01000000) >> 24;
		x = (inst & 0x00800000) >> 23;
		b = (inst & 0x00400000) >> 22;
		p = (inst & 0x00200000) >> 21;
		e = (inst & 0x00100000) >> 20;

		/* relative addressing for format 3 */
		addr = (inst & 0x000FFF00) >> 8; /* get disp */
		if(addr>>11==1) addr |= 0xFFFFF000; /* sign extend disp */
		if(b) addr += (PROGADDR + reg[3]); /* base relative */
		else if(p) addr += (PROGADDR + 3 + reg[8]); /* pc relative */
		if(x) addr += reg[1]; /* index mode */

		/* direct addressing for format 4 */
		if(e) {
			addr = inst & 0x000FFFFF;
			addr += PROGADDR;
		}

		/* FORMAT 2 */
		if(opcode==0xB4) { /* CLEAR */
			reg[8] += 2;
			reg[reg1] = reg[reg2] = 0;
			//dpc += 2;
		} 
		else if(opcode==0xA0) { /* COMPR */
			reg[8] += 2; /* increment PC */
			reg[9] = (reg[reg1] - reg[reg2]); /* set CC */
		} 
		else if(opcode==0xB8) { /* TIXR */
			reg[8] += 2; /* increment PC */
			reg[1]++; /* X <- X + 1 */
			reg[9] = (reg[1] - reg[reg1]); /* (X) : (r1) */
		}

		/* FORMAT 3 */
		else if(opcode==0x28) { /* COMP */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[9] = reg[0] - read_hb(mem,addr,6); /* (A) : (m..m+2) */
			if(!n) reg[9] = reg[0] - addr; /* if immediate */
			//printf("i: %d, comp %X and %X => CC = %X\n",i,reg[0],addr,reg[9]);
		}
		else if(opcode==0x3C) { /* J */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[8] = addr; /* PC <- m */
			if(!i) reg[8] = read_hb(mem,addr,5); /* if indirect */
		}
		else if(opcode==0x30) { /* JEQ */
			reg[8] += (e ? 4 : 3); /* increment PC */
			if(!reg[9]) reg[8] = addr; /* PC <- m if CC set to = */
		}
		else if(opcode==0x38) { /* JLT */
			reg[8] += (e ? 4 : 3); /* increment PC */
			if(reg[9]<0) reg[8] = addr; /* PC <- m if CC set to < */
			//printf("(jlt) addr:%X\n",addr);
		}
		else if(opcode==0x48) { /* JSUB */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[2] = reg[8]; /* L <- (PC) */
			reg[8] = addr; /* PC <- m */	
		}
		else if(opcode==0x00) { /* LDA */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[0] = !n ? addr : (buffer=read_hb(mem,addr,6)); /* A <- (m..m+2) */
		//	printf("read: %X, i: %d, addr: %X, reg[0]:%X\n",buffer,i,addr,reg[0]);
		//	printf("inst: %X\n",inst);
		}
		else if(opcode==0x68) { /* LDB */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[3] = !n ? addr : read_hb(mem,addr,6); /* B <- (m..m+2) */
		}
		else if(opcode==0x50) { /* LDCH */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[0] = read_hb(mem,addr,2); /* A <- (m) */
			reg[0] &= 0xFF; /* A <- A[rightmost byte] */
		}
		else if(opcode==0x74) { /* LDT */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[5] = !n ? addr : read_hb(mem,addr,6); /* T <- (m..m+2) */
			//printf("(ldt) addr: %X\n",addr);
		}
		else if(opcode==0xD8) { /* RD */
			reg[8] += (e ? 4 : 3); /* increment PC */
		}
		else if(opcode==0x4C) { /* RSUB */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[8] = reg[2]; /* PC <- (L) */
		}
		else if(opcode==0x0C) { /* STA */
			reg[8] += (e ? 4 : 3); /* increment PC */
			buffer = reg[0] & 0xFFFFFF; /* buffer <- (A) */
			write_hb(mem,addr,6,buffer); /* m <- (A) */
		}
		else if(opcode==0x54) { /* STCH */
			reg[8] += (e ? 4 : 3); /* increment PC */
			buffer = reg[0] & 0xFF; /* rightmost byte of (A) */	
			write_hb(mem,addr,2,buffer); /* m <- (A) rightmost byte */
		}
		else if(opcode==0x14) { /* STL */
			reg[8] += (e ? 4 : 3); /* increment PC */
			buffer = reg[2] & 0xFFFFFF; /* buffer <- (L) */
			//printf("(stl) addr: %X, buffer: %X\n",addr,buffer);
			write_hb(mem,addr,6,buffer); /* m <- (L) */
		}
		else if(opcode==0x10) { /* STX */
			reg[8] += (e ? 4 : 3); /* increment PC */
			buffer = reg[1] & 0xFFFFFF; /* buffer <- (X) */
			write_hb(mem,addr,6,buffer); /* m..m+2 <- buffer */
		}
		else if(opcode==0xE0) { /* TD */
			reg[8] += (e ? 4 : 3); /* increment PC */
			reg[9] = -1; /* CC set to < */
		}
		else if(opcode==0xDC) { /* WD */
			reg[8] += (e ? 4 : 3); /* increment PC */
		}

		for(int j=0; j<breakpoint->size; j++) {
			if(reg[8] == PROGADDR + breakpoint->arr[j]) {
				quit=1;
				printf("-------------------------\n");
				printf("A : %.6X  X : %.6X\n",reg[0],reg[1]);
				printf("L : %.6X PC : %.6X\n",reg[2],reg[8]);
				printf("B : %.6X  S : %.6X\n",reg[3],reg[4]);
				printf("T : %.6X\n",reg[5]);
				printf("\t\tStop at checkpoint[%X]\n",breakpoint->arr[j]);
				breakpoint->arr[j] = -1;
			}
		}
		if(reg[8]>=PROGADDR+PROGLTH) {
			quit=1;
			printf("-------------------------\n");
			printf("A : %.6X  X : %.6X\n",reg[0],reg[1]);
			printf("L : %.6X PC : %.6X\n",reg[2],reg[8]);
			printf("B : %.6X  S : %.6X\n",reg[3],reg[4]);
			printf("T : %.6X\n",reg[5]);
			printf("\t\tEnd Program\n");
			breakpoint->size=0;
		}
	}
}
