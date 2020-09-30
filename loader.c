#include "loader.h"



int loader(unsigned char mem[65536][16], ES_NODE *estab[20], int PROGADDR, int *EXECADDR, char filename1[50], char filename2[50], char filename3[50]) {
	int PROGLTH=0;
	int idx=0;

	/* file error handling - filetype */
	if(strlen(filename1)) {
		idx=0;
		while(filename1[idx]!='.') {
			if(filename1[idx]=='\0') {
				printf("error! invalid filetype\n");
				return -1;
			}
			idx++;
		}
		if(strcmp(&filename1[idx],".obj")) {
			printf("error! invalid filetype\n");
			return -1;
		}
	}
	if(strlen(filename2)) {
		idx=0;
		while(filename2[idx]!='.') {
			if(filename2[idx]=='\0') {
				printf("error! invalid filetype\n");
				return -1;
			}
			idx++;
		}
		if(strcmp(&filename2[idx],".obj")) {
			printf("error! invalid filetype\n");
			return -1;
		}
	}
	if(strlen(filename3)) {
		idx=0;
		while(filename3[idx]!='.') {
			if(filename3[idx]=='\0') {
				printf("error! invalid filetype\n");
				return -1;
			}
			idx++;
		}
		if(strcmp(&filename3[idx],".obj")) {
			printf("error! invalid filetype\n");
			return -1;
		}
	}

	free_estab(estab);

	if(loader1(estab,PROGADDR,&PROGLTH,filename1,filename2,filename3)==-1) return -1;
	if((*EXECADDR=loader2(mem,estab,PROGADDR,filename1,filename2,filename3))==-1) return -1;

	return PROGLTH;
}


int loader1(ES_NODE *estab[20], int PROGADDR, int *PROGLTH, char filename1[50], char filename2[50], char filename3[50]) {
	/* pass1 */
	int CSADDR,CSLTH;
	char CSNAME[10];
	FILE *fp[3];
	char line[100];
	int idx;
	char tok[50], tok2[50];
	int b;


	fp[0] = fopen(filename1,"r");
	fp[1] = fopen(filename2,"r");
	fp[2] = fopen(filename3,"r");

	/* file error handling - no such file */
	if(strlen(filename1) && !fp[0]) {
		printf("file doens't exist!\n");
		return -1;
	}
	if(strlen(filename2) && !fp[1]) {
		printf("file doens't exist!\n");
		return -1;
	}
	if(strlen(filename3) && !fp[2]) {
		printf("file doens't exist!\n");
		return -1;
	}

	printf("control  symbol  address  length\n");
	printf("section  name\n");
	printf("--------------------------------\n");



	CSADDR = PROGADDR;
	for(int i=0; i<3; i++) { /* for each file */
		if(!fp[i]) continue;
	
		/* get CSNAME, CSLTH from  header */
		fscanf(fp[i],"%[^\n]s",line); fgetc(fp[i]); /* read header line */
		idx=1; get_token(line,CSNAME,&idx,8); /* get CSNAME */
		
		tok[6]='\0'; for(idx=13;idx<=18;idx++)tok[idx-13]=line[idx]; /* get CSLTH */
		CSLTH = parse_op(tok);
		
		/* insert CSNAME to estab */
		printf("%-9s        %-9X%.4X\n",CSNAME,CSADDR,CSLTH);
		add_estab(estab,CSNAME,CSADDR,CSLTH);

		/* read each D record */
		for(int j=0; j<100; j++) line[j]='\0';
		while(fscanf(fp[i],"%[^\n]s",line)!=EOF) {	
			fgetc(fp[i]);
			if(line[0]=='D') {
				idx=1;
				do {
					/* symbol */
					get_substr(line,tok,idx,idx+5);		
					idx += 6;
					/* loc */
					tok2[6]='\0'; for(int j=idx;j<idx+6;j++) tok2[j-idx]=line[j];
					b = parse_op(tok2);
					idx += 6;
					/* insert the symbol to estab */
					printf("         %-8s%-8X\n",tok,CSADDR+b);
					add_estab(estab,tok,CSADDR+b,-1);
				} while(line[idx]!='\0');	
			}
			for(int j=0; j<100; j++) line[j]='\0';
		}
		CSADDR += CSLTH;
	}
	*PROGLTH = CSADDR - PROGADDR;

	printf("--------------------------------\n");
	printf("\t\ttotal length %.4X\n",*PROGLTH);

	if(fp[0]) fclose(fp[0]);
	if(fp[1]) fclose(fp[1]);
	if(fp[2]) fclose(fp[2]);
	return 0;

// takes PROGADDR
// init CSADDR = PROGADDR

// for each control section
	// set CSLTH from H record
	// insert CS name to ESTAB if not found
	// for each D record
		// for each symbol at loc, insert (symbol, CSADDR+loc) into ESTAB if not found
	// CSADDR += CSLTH
}

int loader2(unsigned char mem[65536][16], ES_NODE *estab[20], int PROGADDR, char filename1[50], char filename2[50], char filename3[50]) {
	/* pass 2 */
	int CSADDR,CSLTH,EXECADDR;
	char CSNAME[10];
	FILE *fp[3];
	char line[100];
	int idx;
	char tok[50], tok2[50];
	int a,b;
	int reftab[100]; /* for the static indexing of R records */
	int loc;
	ES_NODE *es0;

	fp[0] = fopen(filename1,"r");
	fp[1] = fopen(filename2,"r");
	fp[2] = fopen(filename3,"r");




	CSADDR = PROGADDR;
	EXECADDR = PROGADDR;
	for(int i=0; i<3; i++) { /* for each file */
		if(!fp[i]) continue;
		
		/* get CSNAME, CSLTH from  header */
		fscanf(fp[i],"%[^\n]s",line); fgetc(fp[i]); /* read header line */
		idx=1; get_token(line,CSNAME,&idx,8); /* get CSNAME */
		reftab[1] = CSADDR;
		tok[6]='\0'; for(idx=13;idx<=18;idx++)tok[idx-13]=line[idx]; /* get CSLTH */
		CSLTH = parse_op(tok);

		for(int j=0; j<100; j++) line[j]='\0';
		while(fscanf(fp[i],"%[^\n]s",line)!=EOF) {
			fgetc(fp[i]);
			if(line[0]=='T') {
				tok[6]='\0'; for(idx=1;idx<=6;idx++)tok[idx-1]=line[idx]; /* get loc */
				loc = parse_op(tok) + CSADDR;
				/* get length */
				tok[2]='\0'; for(idx=7;idx<=8;idx++)tok[idx-7]=line[idx];
				a = parse_op(tok); /* record length */
				for(int j=0; j<a; j++) {
					/* read byte */
					tok[2]='\0'; for(int k=idx;k<idx+2;k++) tok[k-idx]=line[k];
					b = parse_op(tok);
					edit(mem,loc,parse_op(tok));
					idx += 2; loc += 1;
				}
			} 
			else if(line[0]=='M') {
				tok[6]='\0'; for(idx=1;idx<=6;idx++)tok[idx-1]=line[idx]; /* get loc */
				loc = parse_op(tok) + CSADDR;
				/* get length */
				tok[2]='\0'; for(idx=7;idx<=8;idx++)tok[idx-7]=line[idx];
				a = parse_op(tok); /* content length */
				/* get operand */
				tok[2]='\0'; for(idx=10;idx<=11;idx++)tok[idx-10]=line[idx];
				b = reftab[parse_op(tok)];
				if(line[9]=='+') {
					modify_hb(mem,loc,a,1,b);
				} else if(line[9]=='-') {
					modify_hb(mem,loc,a,0,b);
				}
			}
			else if(line[0]=='R') {
				idx=1;
				do {
					/* reference number */
					tok2[2]='\0'; for(int j=idx;j<idx+2;j++) tok2[j-idx]=line[j];
					b = parse_op(tok2);
					idx += 2;
					/* symbol */
					get_substr(line,tok,idx,idx+5);	
					idx += 6;
					/* insert the symbol to reftab */
					if((es0=search_estab(estab,tok))) reftab[b] = es0->address;
					else {
						printf("%s : undefined symbol\n",tok);
						if(fp[0])fclose(fp[0]);if(fp[1])fclose(fp[1]);if(fp[2])fclose(fp[2]);
						return -1;
					}
				} while(line[idx]!='\0');	
			}
			else if(line[0]=='E'&&line[1]=='0') {
				tok[6]='\0'; for(idx=1;idx<=6;idx++)tok[idx-1]=line[idx]; /* get loc */
				EXECADDR = parse_op(tok) + CSADDR;	
			}
			for(int j=0; j<100; j++) line[j]='\0';
		}
		CSADDR += CSLTH;
	}


	
	if(fp[0]) fclose(fp[0]);
	if(fp[1]) fclose(fp[1]);
	if(fp[2]) fclose(fp[2]);


	return EXECADDR;
// CSADDR = PROGADDR
// EXECADDR = PROGADDR

// for each control section
	// set CSLTH from H record
	// for each line
		// if T then place content to CSADDR+loc
		// if M then modify content at CSADDR+loc
	// EXECADDR = (CSADDR+entry)
	// CSADDR += CSLTH

// jump to EXECADDR		

}

