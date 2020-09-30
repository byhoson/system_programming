#ifndef LL
#define LL

#include <stdlib.h>
#include <string.h>

typedef struct _NODE {
	char cmd[80];
	struct _NODE *link;		
} NODE;

void push_back(NODE **head_history, char cmd[80]);


#endif
