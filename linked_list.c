#include "linked_list.h"
#include <stdio.h> // TODO
void push_back(NODE **head_history, char cmd[80]) {
	/*
	 * pushes the NODE containg cmd into the history
	 * Arguments:
	 *	NODE **head_history - the pointer of the head of the linked list
	 * 	char cmd[100] - the shell command to add into history
	 */

	/* get tail (i.e. the last element in the list, if any) */
	NODE *tail = *head_history;
	if(tail) while(tail->link) tail = tail->link;

	/* make node */
	NODE *item = (NODE*)malloc(sizeof(NODE));
	strcpy(item->cmd, cmd);
	item->link = NULL;

	/* paste the item to the tail, if any */
	if(tail) tail->link = item;
	else *head_history = item;
}


