#include "utils.h"

void get_token(char cmd[], char token[], int *idx, int size) {
	/*
	 * generates the next token contained in cmd appearing after the idx
	 * Arguments:
	 *	char cmd[] - the cmd string
	 *   	char token[] - the resultant token
	 *	int *idx - pointer to the starting index in cmd
	 * 	int size - the max size of the cmd & token
	 */
	int i;
	int comma=0; /* flag for detecting the first comma */

	/* skip valid blanks after idx */
	while(*idx<size-1 && ((!comma&&cmd[*idx]==',') || cmd[*idx]==' ' || cmd[*idx]=='\t') && cmd[*idx]!='\0') {
		(*idx)++;
		comma = 1;
	}

	/*
	 at this point, idx indicates one of the following:
	   i) first character of token (typical case)
	   ii) null char (end of the string)
	   iii) max size (end of the array)
	*/

	/* construct the token until the first delimiter is recognized */
	i=0;
	while(*idx<size-1 && i<size-1 && cmd[*idx]!=' ' && cmd[*idx]!='\t' && cmd[*idx]!='\0' && cmd[*idx]!=',') {
		token[i] = cmd[*idx];
		(*idx)++;
		i++;
	}
	token[i] = '\0'; /* make token into a valid string */
}

void get_substr(char str[], char tok[], int lb, int ub) {
	int i; /* scans tok */
	int comma=0; /* flag for detecting the first comma */
	int idx = lb; /* scans str */

	/* skip valid blanks after idx */
	while(idx<=ub && ((!comma&&str[idx]==',') || str[idx]==' ' || str[idx]=='\t') && str[idx]!='\0') {
		idx++;
		comma = 1;
	}

	/*
	 at this point, idx indicates one of the following:
	   i) first character of token (typical case)
	   ii) null char (end of the string)
	   iii) max size (end of the array)
	*/

	/* construct the token until the first delimiter is recognized */
	i=0;
	while(idx<=ub && str[idx]!=' ' && str[idx]!='\t' && str[idx]!='\0' && str[idx]!=',') {
		tok[i] = str[idx];
		idx++;
		i++;
	}
	tok[i] = '\0'; /* make token into a valid string */

}

int hex_to_int(char digit) {
	/*
	 * converts the hex char into int
	 * Arguments:
	 *	char digit - hex in char
	 * Returns:
	 * 	the corresponding int if digit is a valid hex, and -1 otherwise
	 */
	if('0' <= digit && digit <= '9') return (int)(digit-'0');
	else if('A' <= digit && digit <= 'F') return 10+(int)(digit-'A');
	else if('a' <= digit && digit <= 'f') return 10+(int)(digit-'a');
	else return -1;
}

int parse_op(char op[]) {
	/*
	 * converts the hex string into int, if valid
	 * Arguments:
	 * 	char op[] - string consisting of hex digits
	 * Returns:
	 * 	the corresponding int if op is not null, -1 if null, and -2 if invalid
	 */
	int basis,idx,ret,valid=1;
	const int len = strlen(op);

	ret = 0;
	basis = 1;
	idx = len - 1; /* last idx of op */
	while(idx>=0) {
		/* check validity */
		valid *= (('0'<=op[idx] && op[idx]<='9') || ('a'<=op[idx] && op[idx]<='f') || ('A'<=op[idx] && op[idx]<='F')) ? 1 : 0;
		/* construct start address from op */
		ret += hex_to_int(op[idx]) * basis;
		basis *= 16;
		idx--;
	}
	
	if(!valid) return -2;
	else if(!len) return -1;
	else return ret;
}

int parse_dec(char dec[]) {
	// TODO returns -1 if invalid
	int basis, sum, idx;
	const int len = strlen(dec);
	sum = 0;
	basis = 1;
	idx = len - 1;
	while(idx>=0) {
		if(dec[idx]<'0' || dec[idx]>'9') return -1;
		sum += basis * (int)(dec[idx] - '0');
		basis *= 10;
		idx--;
	}
	return sum;
}
