#include "shell.h"



void help() {
	/* prints the help string */
	const char help_str[200] = \
"h[elp]\n\
d[ir]\n\
q[uit]\n\
hi[story]\n\
du[mp] [start, end]\n\
e[dit] address, value\n\
f[ill] start, end, value\n\
reset\n\
opcode mnemonic\n\
opcodelist\n\
assemble filename\n\
type filename\n\
symbol\n";

	printf("%s",help_str);
}

void dir() {
	/*
	 * lists all the files contained in the current directory
	 * [ref] en.wikibooks.org/wiki/C_Programming/POSIX_Reference/dirent.h
	 * [ref] en.wikibooks.org/wiki/C_Programming/POSIX_Reference/sys/stat.h
	 */
	struct dirent *entry;
	DIR *dp;
	struct stat fstat;


	dp = opendir(".");
	while((entry = readdir(dp))) {
		lstat(entry->d_name,&fstat); /* get file mode */
		printf("%s", entry->d_name);

		if(S_ISDIR(fstat.st_mode)) printf("/"); /* directory */
		else if(fstat.st_mode & S_IXUSR) printf("*"); /* executable */

		printf("\n");
	}
	closedir(dp);
}

void type(char filename[]) {
	/*
	 * print out the content of the file
	 * Arguments:
	 * 	char filename[]
	 */
	struct stat fstat;
	FILE *fp = fopen(filename,"r");
	char line[200];

	if(!fp) {
		fprintf(stderr,"%s not found!\n",filename);
		return;
	}	

	lstat(filename,&fstat); /* get file mode */
	
	if(S_ISREG(fstat.st_mode)) 
		while(fscanf(fp,"%[^\n]s",line)!=EOF) {
			fgetc(fp);
			printf("%s\n",line);
			line[0]='\0';
		}

	fclose(fp);
}

void history(NODE *head) {
	/* traverse the list and print */
	NODE *curr = head;
	int i=1;
	while(curr) {
		printf("%d\t%s\n",i++,curr->cmd);
		curr = curr->link;
	}
}
