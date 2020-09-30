#ifndef UTILS
#define UTILS

#include <string.h>


void get_token(char cmd[], char token[], int *idx, int size);
int hex_to_int(char digit);
int parse_op(char op[]);
int parse_dec(char dec[]);
void get_substr(char str[], char tok[], int lb, int ub);

#endif
