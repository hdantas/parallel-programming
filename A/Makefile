all: run

template: ass1-pthreads-template.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

serial: ass1.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

runserial: serial
	./$^.out

compile: ass1-pthreads.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

run: template
	./$^.out

dbug: template
	gdb ./$^.out

clean:
	trash-put -v *.out
