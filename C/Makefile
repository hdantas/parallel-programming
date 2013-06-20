all: compile run

compile: ass3-pthreads.c

	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

run: compile
	./$^.out

dbug: compile
	gdb ./$^.out

clean:
	trash-put -v compile.out
