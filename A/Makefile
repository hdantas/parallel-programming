all: run

serial: ass1.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

runserial: serial
	./$^.out

compile: ass1-pthreads.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

run: compile
	./$^.out

dbug: compile
	gdb ./$^.out

clean:
	trash-put -v *.out
