#include "assembler.h"

void assemble(char filename[80], SYMBOL **sym_list, SYMBOL *symtab[20], HASH_NODE *optab[20]) {
	/* 
	 * the wrapper of pass1 and pass2
	 * Argugments:
	 * 	char filename[] - the .asm file
	 * 	SYMBOL **sym_list
	 * 	SYMBOL *symtab[]
	 * 	HASH_NODE *optab[]
	 */
	int prog_len=0;
	int idx=0;
	char prefix[80]; /* prefix of the filename */

	/* parse filename */
	while(filename[idx]!='.') {
		if(filename[idx]=='\0') {
			printf("error! invalid filetype\n");
			return;
		}
		idx++;
	}
	if(strcmp(&filename[idx],".asm")) {
		printf("error! invalid filetype\n");
		return;
	}

	/* init symbtab */
	free_symtab(symtab);
	add_symtab(symtab,"A",0);
	add_symtab(symtab,"X",1);
	add_symtab(symtab,"L",2);
	add_symtab(symtab,"B",3);
	add_symtab(symtab,"S",4);
	add_symtab(symtab,"T",5);
	add_symtab(symtab,"F",6);
	add_symtab(symtab,"PC",8);
	add_symtab(symtab,"SW",9);

	if(pass1(filename,symtab,optab,&prog_len)) {
		/* pass1 failed */
		free_sym_list(sym_list);
		return;
	}
	
	filename[idx]='\0';
	strcpy(prefix,filename);
	filename[idx]='.';

	if(pass2(prefix,symtab,optab,prog_len)) {
		/* pass2 failed */
		free_sym_list(sym_list);
		return;
	}
	
	/* assemble successful */
	printf("Successfully assemble %s.\n",filename);
	free_sym_list(sym_list);
	sort_sym(sym_list, symtab);
}


int pass1(char filename[80], SYMBOL *symtab[20], HASH_NODE *optab[20], int *prog_len) {
	/* 
	 * pass1
	 * Argugments:
	 * 	char filename[] - the .asm file
	 * 	SYMBOL *symtab[]
	 * 	HASH_NODE *optab[]
	 * 	int *prog_len - the ptr to program length
	 * Returns:
	 * 	0 if successful, 1 otherwise
	 */
	int locctr=0;
	int idx; // for parsing asm cmd
	int is_format4;
	int tmp;
	char line[80];
	char label[80];
	char opcode[80];
	char op1[80];
	char op2[80];
	FILE *fp = fopen(filename,"r");
	FILE *mid = fopen("mid","w");
	HASH_NODE *opnode;
	int print_loc;
	int d_loc;
	int prog_start=0;
	int line_num=5;
	int parsed_op;
	/* file error handling */
	if(!fp) {
		printf("%s not found!\n",filename);
		fclose(fp); fclose(mid);
		return 1;
	}
	line[0]='\0';
	while(fscanf(fp,"%[^\n]s",line)!=EOF) {
		print_loc=1;
		d_loc=0;
		fgetc(fp);

		/* parse the cmd of form (label, opcode, op1, op2) */
		idx=0; label[0]='\0';
		if(line[0]!=' ' && line[0]!='\t') get_token(line, label, &idx, 80);
		get_token(line, opcode, &idx, 80);
		get_token(line, op1, &idx, 80);
		get_token(line, op2, &idx, 80);	

		if(locctr>0xFFFFF) {
			printf("line %d: address out of range\n",line_num);
			fclose(fp); fclose(mid);
			return 1;
		}

		if(!strcmp(opcode,"START")) {
			locctr = parse_op(op1);
			if(locctr<0) { // TODO test!
				printf("line %d: invalid start operand\n",line_num);
				fclose(fp); fclose(mid);
				return 1;
			}
			prog_start = locctr;
			fprintf(mid,"%.4X    %s\n",locctr,line); /* with locctr */
		} else if(!strcmp(opcode,"END")) {
			fprintf(mid,"        %s\n",line);
			*prog_len = locctr - prog_start; /* store prog_len */
			fclose(fp); fclose(mid);
			return 0;
		} else if (line[0]=='\0') {
			fprintf(mid,"\n");
			continue;
		} else if(label[0]!='.') { /* not a comment */
			/* process label token */
			if(strlen(label)) {
				if(search_symtab(symtab, label)) { /* if found */
					printf("line %d: duplicate symbols\n",line_num); /* error */ // TODO Test it!
					fclose(fp); fclose(mid);
					return 1;
				} else { /* if not found */
					add_symtab(symtab, label, locctr);
				}
			}
			/* preprocess opcode for format 4 */
			is_format4 = 0;
			if(opcode[0]=='+') {
				is_format4 = 1;
				idx = 1;
				get_token(opcode, opcode, &idx, 50);
			}
			/* process opcode token */
			if((opnode = search_optab(optab,opcode))) {
				/* increment locctr */
				if(is_format4) d_loc++;
				d_loc += opnode->opcode_len;
			} else if(!strcmp(opcode,"WORD")) {
				d_loc += 3;
			} else if(!strcmp(opcode,"RESW")) {
				parsed_op = parse_dec(op1);
				if(parsed_op>=0) d_loc += 3 * parse_dec(op1);
				else {
					printf("line %d: invalid operand\n",line_num); /* error */ // TODO Test it!
					fclose(fp); fclose(mid);
					return 1;
				}
			} else if(!strcmp(opcode,"RESB")) {
				parsed_op = parse_dec(op1);
				if(parsed_op>=0) d_loc += parse_dec(op1);
				else {
					printf("line %d: invalid operand\n",line_num); /* error */ // TODO Test it!
					fclose(fp); fclose(mid);
					return 1;
				}
			} else if(!strcmp(opcode,"BYTE")) {
				idx=2; tmp=0;
				while(op1[idx] != '\'') {
					if(op1[idx]=='\0') {
						printf("line %d: invalid operand\n",line_num); /* error */ // TODO Test it!
						fclose(fp); fclose(mid);
						return 1;
					}
					tmp++;
					idx++;
				}	
				if(op1[0]=='X'&&op1[1]=='\'') tmp  = tmp/2;
				else if(op1[0]!='C'||op1[1]!='\'') {
					printf("line %d: invalid operand\n",line_num); /* error */ // TODO Test it!
					fclose(fp); fclose(mid);
					return 1;
				}
				d_loc += tmp;
			} else if(!strcmp(opcode,"BASE")||!strcmp(opcode,"NOBASE")) {
				print_loc=0;				
			} else {
				// TODO Test it!
				printf("line %d: opcode not found\n",line_num); /* error */
				fclose(fp); fclose(mid);
				return 1;
			}
			
			if(print_loc) fprintf(mid,"%.4X    %s\n",locctr,line); /* with locctr */
			else fprintf(mid,"        %s\n",line); /* without locctr */
		
			locctr += d_loc;

		} else { /* is a comment */
			fprintf(mid,"        %s\n",line);
		}
		line_num += 5;
		line[0]='\0';
	}
	//*prog_len = locctr - prog_start; /* store prog_len */
	printf("error: the asm file should contain END directive\n");
	fclose(fp); fclose(mid);
	return 1;
}


int pass2(char prefix[80], SYMBOL *symtab[20], HASH_NODE *optab[20], int prog_len) {
	/* 
	 * pass2
	 * Argugments:
	 * 	char prefix[] - prefix of .asm file
	 * 	SYMBOL *symtab[]
	 * 	HASH_NODE *optab[]
	 * 	int prog_len - program length given by pass1
	 * Returns:
	 * 	0 if successful, 1 otherwise
	 */
	int locctr;
	int idx;
	int parsed_op;
	char line[100];
	char label[80];
	char opcode[80];
	int tmp;
	unsigned char byte;

	char op1[80]; int op1_int;
	char op2[80]; int op2_int;
	char tok[80];

	FILE *mid; FILE *lst; FILE *obj;
	HASH_NODE *opnode; 
	SYMBOL *symnode;
	int opcode_len; /* in byte */
	unsigned int opcode_int; /* to store 32-bit instruction */
	int base=-1; /* content of base register if BASE, -1 if NOBASE */
	int direct=0; /* direct addressing mode */
	unsigned char *literal; /* buffer for data when printing to obj */
	int is_data=0; /* 0: instruction, 1: data, 2: reserve data */
	int line_num; /* line number */

	int is_modif=0;
	unsigned char obj_line[50];
	int obj_idx=0;
	int obj_loc=0;
	int obj_br=0;

	M_REC *m_recs=NULL; /* linked list of m records */
	M_REC *m_item, *m_curr;

	/* open files */
	mid = fopen("mid","r");
	lst = fopen(strcat(prefix,".lst"),"w");
	prefix[strlen(prefix)-4]='\0';
	obj = fopen(strcat(prefix,".obj"),"w");

	/* file error handling */
	if(!mid||!lst||!obj) {
		printf("file not found\n");
		fclose(mid); fclose(lst); fclose(obj);
		return 1;
	}

	locctr=0;
	line_num=5;
	
	line[0]='\0';
	while(fscanf(mid,"%[^\n]s",line)!=EOF) {
		fgetc(mid);
		/* parse the cmd of form (locctr, label, opcode, op1, op2) */
		if(line[0]!='\0') {
			idx=0; label[0] = '0';
			get_token(line, tok, &idx, 80); 
			if(idx>4) {idx=4; locctr=-1;} // locctr is empty
			else locctr = parse_op(tok);
			if(line[8]!=' ' && line[8]!='\t') get_token(line, label, &idx, 80); 	
			get_token(line, opcode, &idx, 80);
			get_token(line, op1, &idx, 80);
			get_token(line, op2, &idx, 80);
		}

		is_data=0;
		direct=0;

		if(!strcmp(opcode,"START")) {
			parsed_op = parse_op(op1);
			if(parsed_op>=0) {
				fprintf(lst,"%-8d%s\n",line_num,line);
				fprintf(obj,"H%-6s%.6X%.6X\n",label,parsed_op,prog_len);
			}
			else {
				printf("line %d: invalid start operand\n",line_num); /* error */ // TODO test!
				fclose(mid); fclose(lst); fclose(obj);
				return 1;
			}	
			obj_loc = locctr;		
		} else if(!strcmp(opcode,"END")) {
			fprintf(lst,"%-8d%s\n",line_num,line);
			/* flush obj_line */
			if(obj_idx) {
				fprintf(obj,"T%.6X%.2X",obj_loc,obj_idx);
				for(int i=0; i<obj_idx; i++) fprintf(obj,"%.2X",obj_line[i]);
				fprintf(obj,"\n");
			}
			
			/* add M records to obj */	
			m_curr = m_recs;
			while(m_curr) {
				fprintf(obj,"M%.6X%.2X\n",m_curr->address,m_curr->length);
				m_curr = m_curr->link;
			}

			/* free m_recs */
			m_curr = m_recs;
			while(m_curr) {
				m_item = m_curr;
				m_curr = m_curr->link;
				free(m_item);
			}
			m_recs = NULL;

			/* add E record to obj */
			if((symnode = search_symtab(symtab, op1))) { // if found
				parsed_op = symnode->address;
			} else if((parsed_op=parse_op(op1))>=0) { // if not found
				op1_int = parsed_op;
			} else {
				printf("line %d: invalid end operand\n",line_num); /* error */
				fclose(mid); fclose(lst); fclose(obj);
				return 1;
			}

			fprintf(obj,"E%.6X\n",parsed_op);
			fclose(mid); fclose(lst); fclose(obj);
			return 0;		
		} else if(line[0]=='\0') {
			fprintf(lst,"%d\n",line_num);
			line_num += 5;
			continue;
		} else if(label[0]!='.') { /* not a comment */
			opcode_int=0;
			opcode_len=0;
			/* preprocess format 4 */
			if(opcode[0]=='+') {
				opcode_len = 4;
				is_modif=1;
				idx = 1;
				get_token(opcode, opcode, &idx, 80);
			}
			if((opnode = search_optab(optab,opcode))) {
				/* fill in the opcode part of opcode_int */
				if(!opcode_len) opcode_len = opnode->opcode_len;
				opcode_int = opnode->opcode;
				opcode_int <<= 8*(opcode_len-1);
				
				/* simple addressing by default i=n=1 */
				if(opcode_len==3) opcode_int |= 0x030000;
				if(opcode_len==4) opcode_int |= 0x03000000;

				/* parse op1 */
				if(strlen(op1)) {
					/* preprocess */
					if(op1[0]=='#') {  /* immediate addressing */
						/* n=0 */
						if(opcode_len==3) opcode_int &= 0xFDFFFF;
						if(opcode_len==4) opcode_int &= 0xFDFFFFFF;
						idx=1;
						get_token(op1,op1,&idx,50);
					} else if(op1[0]=='@') { /* indirect addressing */
						/* i=0 */
						if(opcode_len==3) opcode_int &= 0xFEFFFF;
						if(opcode_len==4) opcode_int &= 0xFEFFFFFF;
						idx=1;
						get_token(op1,op1,&idx,50);
					} 

					/* search symtab */
					if((symnode = search_symtab(symtab, op1))) { /* if found */
						op1_int = symnode->address;
					} else if((parsed_op=parse_dec(op1))>=0) { /* constant operand */
						op1_int = parsed_op;
						direct=1; /* direct addressing */
						is_modif=0;		
					} else {
						printf("line %d: invalid operand\n",line_num); /* error */
						fclose(mid); fclose(lst); fclose(obj);
						return 1;
					}


				} else op1_int = 0;

				if(strlen(op2)) {
					/* 
					 there are 2 cases where op2 is used   
						(i) one of the 2 registers in format 2
						(ii) base addressing mode 
					*/

					/* case (ii) */
					if(opcode_len>=3 && op2[0]=='X') opcode_int |= 0x8000;
					
					/* case (i) */
					else if((symnode = search_symtab(symtab, op2))) { /* if found */
						op2_int = symnode->address;
					} else {
						printf("line %d: invalid operand\n",line_num); /* error */
						fclose(mid); fclose(lst); fclose(obj);
						return 1;
					}
				} else op2_int =0;


				/* generate full instruction */

				/* format 2 */
				if(opcode_len==2) {
					if(0<=op1_int&&op1_int<=9&&0<=op2_int&&op2_int<=9) {
						opcode_int |= (op1_int << 4);
						opcode_int |= op2_int;
					} else {
						printf("line %d: no such register\n",line_num); /* error */ // TODO test!
						fclose(mid); fclose(lst); fclose(obj);
						return 1;
					}
				}
	
				/* format3 */
				else if(strlen(op1) && opcode_len==3) {
					if(direct && -2048<=op1_int && op1_int<=2047) { /* direct addressing */
						opcode_int |= op1_int;	
					} else if(-2048<=(tmp=op1_int-locctr-3) && tmp<=2047) { /* pc relative */
						tmp &= 0x000FFF; /* bit masking for the negative offset case */
						opcode_int |= tmp;
						opcode_int |= 0x002000; /* p=1 */
					} else if(base>=0 && -2048<=(tmp=op1_int-base) && tmp<=2047) { /* base */
						tmp &= 0x000FFF; /* bit masking for the negative offset case */
						opcode_int |= tmp;
						opcode_int |= 0x004000; /* b=1 */
					} else {
						printf("line %d: address not reachable\n",line_num); /* error */ // TODO test!
						fclose(mid); fclose(lst); fclose(obj);
						return 1;
					}
					
				}

				/* format4 */
				else if(opcode_len==4) {
					opcode_int |= op1_int;
					opcode_int |= 0x100000; /* e=1 */
					/* push back m_rec item to m_recs */
					if(is_modif) {
						m_item = (M_REC*)malloc(sizeof(M_REC));
						m_item->address = locctr+1;
						m_item->length = 5;
						m_item->link = NULL;

						m_curr = m_recs;
					
						if(!m_curr) m_recs = m_item;
						else {	
							while(m_curr->link) m_curr = m_curr->link;
							m_curr->link = m_item;
						}
					}
				}

			} else if(!strcmp(opcode,"BASE")) {
				if((symnode = search_symtab(symtab, op1))) { /* if found */ 
					base = symnode->address;
				} else {
					printf("line %d: invalid base operand\n",line_num); /* error */ // TODO test!
					fclose(mid); fclose(lst); fclose(obj);
					return 1;
				} 
			} else if(!strcmp(opcode,"NOBASE")) {
				base = -1;
			} else if(!strcmp(opcode,"WORD")) {
				/* for convenience, regard is_data=0 */	
				opcode_int = parse_dec(op1);	// TODO parse_dec unsigned??
				opcode_len = 3;
				if(opcode_int<0) {
					printf("line %d: invalid word operand\n",line_num); /* error */ // TODO test!
					fclose(mid); fclose(lst); fclose(obj);
					return 1;
				} 
			} else if(!strcmp(opcode,"BYTE")) {
				is_data=1;
				idx=2; tmp=0; /* tmp= length of the string */
				while(op1[idx] != '\'') {
					tmp++;
					idx++;
				}
				if(op1[0]=='C') {
					literal = (unsigned char*)malloc(tmp+1); 
					idx=2;
					while(op1[idx] != '\'') {
						literal[idx-2] = op1[idx];
						idx++;
					}
					literal[tmp]='\0';
					opcode_len=tmp;
				} else if (op1[0]=='X') {
					/* assert tmp even */
					if(tmp%2) {
						printf("line %d: hex constant must be of even size\n",line_num); /* error */ // TODO test!
						fclose(mid); fclose(lst); fclose(obj);
						return 1;
					}
					literal = (unsigned char*)malloc(tmp/2+1);
					idx=2;		
					while(op1[idx] != '\'') {
						byte=(unsigned char)hex_to_int(op1[idx]);
						byte<<=4;
						byte|=(unsigned char)hex_to_int(op1[idx+1]);
						literal[(idx/2)-1]=byte;	
						idx+=2;
					}
					
					literal[tmp/2]='\0';
					opcode_len=tmp/2;
				}
			} else if(!strcmp(opcode,"RESW") || !strcmp(opcode,"RESB")) {
				obj_br=1; is_data=2;
				fprintf(lst,"%-8d%s\n",line_num,line);
			} else {
				printf("line %d: opcode not found\n",line_num); // TODO test!
				fclose(mid); fclose(lst); fclose(obj);
				return 1;
			}
			
		
			if(!is_data) {
				/* write line to lst */
				if(locctr!=-1) {
					if(opcode_len==1) fprintf(lst,"%-8d%-40s%.2X\n",line_num,line,opcode_int);
					if(opcode_len==2) fprintf(lst,"%-8d%-40s%.4X\n",line_num,line,opcode_int);
					if(opcode_len==3) fprintf(lst,"%-8d%-40s%.6X\n",line_num,line,opcode_int);
					if(opcode_len==4) fprintf(lst,"%-8d%-40s%.8X\n",line_num,line,opcode_int);
				}
				else fprintf(lst,"%-8d%-40s\n",line_num,line);

			
				/* write line to obj */	

				/* break line */
				if(obj_br || obj_idx+opcode_len>30) {
					fprintf(obj,"T%.6X%.2X",obj_loc,obj_idx); 
					for(int i=0; i<obj_idx; i++) fprintf(obj,"%.2X",obj_line[i]);
					fprintf(obj,"\n");
					obj_idx=0;
					obj_loc=locctr;
				}
							
				/* add to buffer(obj_line) */
				for(int i=opcode_len-1; i>=0; i--) {
					obj_line[obj_idx+i] = opcode_int & 0xFF; /* last byte of opcode_int */
					opcode_int >>= 8; /* right shift by 8 */
				}
				obj_idx += opcode_len;
			

				obj_br=0;
				
			} else if(is_data==1) {
				/* write line to lst */
				fprintf(lst,"%-8d%-40s", line_num,line);
				idx=0;
				while(literal[idx]!='\0') {
					fprintf(lst,"%.2X",(int)literal[idx++]);	
				}
			
				fprintf(lst,"\n");		
			
				/* write line to obj */
				/* break line */
				if(obj_br || obj_idx+opcode_len>30) {
					fprintf(obj,"T%.6X%.2X",obj_loc,obj_idx);
					for(int i=0; i<obj_idx; i++) fprintf(obj,"%.2X",obj_line[i]);
					fprintf(obj,"\n");
					obj_idx=0;
					obj_loc=locctr;
				}
							
				/* add to buffer(obj_line) */
				for(int i=0; i<opcode_len; i++) {
					obj_line[obj_idx+i] = literal[i];
				}
				obj_idx += opcode_len;	

				obj_br=0;

				free(literal);
			}
		} else { /* is a comment */
			fprintf(lst,"%-8d%s\n",line_num,line);
		}
		line_num+=5;
		line[0]='\0';
	}


	/* free m_recs */
	m_curr = m_recs;
	while(m_curr) {
		m_item = m_curr;
		m_curr = m_curr->link;
		free(m_item);
	}
	m_recs = NULL;

	printf("error: the asm file should contain END directive\n");
	fclose(mid); fclose(lst); fclose(obj);
	return 1;
}


