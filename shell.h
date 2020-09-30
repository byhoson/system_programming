#ifndef SHELL
#define SHELL

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#include "linked_list.h"

void help();
void dir();
void history(NODE *head);
void type(char filename[]);

#endif
