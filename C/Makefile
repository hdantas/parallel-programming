all: run

template: ass3-pthreads-template.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

serial: ass3.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

runserial: serial
	./$^.out

compile: ass3-pthreads.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

run: template
	./$^.out

dbug: template
	gdb ./$^.out

clean:
	trash-put -v *.out
