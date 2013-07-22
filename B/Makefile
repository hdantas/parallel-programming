all: run

template: ass2-pthreads-template.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

serial: ass2.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

thread: ass2-pthreads.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

runserial: serial
	./$^.out

runthread: thread
	./$^.out

run: template
	./$^.out

dbug: template
	gdb ./$^.out

clean:
	trash-put -v *.out
