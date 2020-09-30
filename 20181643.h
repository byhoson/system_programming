#ifndef MAIN
#define MAIN

#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "memory.h"
#include "optab.h"
#include "linked_list.h"
#include "utils.h"
#include "assembler.h"
#include "symtab.h"
#include "estab.h"
#include "loader.h"
#include "exec.h"

void process_cmd(char cmd[], int *quit);
void initialize();

#endif
