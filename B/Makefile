all: compile run

compile: ass2-pthreads.c

	gcc $^ -ggdb3 -pthread -lm -Wall -o $@

run: compile
	./$^

dbug: compile
	gdb ./$^

clean:
	trash-put -v compile
