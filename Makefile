main: 20181643.o shell.o memory.o optab.o linked_list.o utils.o assembler.o symtab.o loader.o estab.o exec.o
	gcc -Wall -o 20181643.out 20181643.o shell.o memory.o optab.o linked_list.o utils.o assembler.o symtab.o loader.o estab.o exec.o

20181643.o: 20181643.c
	gcc -Wall -c 20181643.c

shell.o: shell.c
	gcc -Wall -c shell.c

memory.o: memory.c
	gcc -Wall -c memory.c

optab.o: optab.c
	gcc -Wall -c optab.c

linked_list.o: linked_list.c
	gcc -Wall -c linked_list.c

utils.o: utils.c
	gcc -Wall -c utils.c

assembler.o: assembler.c
	gcc -Wall -c assembler.c

symtab.o: symtab.c
	gcc -Wall -c symtab.c

loader.o: loader.c
	gcc -Wall -c loader.c

estab.o: estab.c
	gcc -Wall -c estab.c

exec.o: exec.c
	gcc -Wall -c exec.c

clean: 
	rm *.o *.out
