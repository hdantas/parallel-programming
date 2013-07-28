all: pthreads-run

openmp: ass1-openmp.c
	gcc $^ -ggdb3 -fopenmp -lm -Wall -o $@.out

pthreads: ass1-pthreads-template.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

serial: ass1.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

# compile: ass3-pthreads.c
# 	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

serial-run: serial
	./$^.out

pthreads-run: pthreads
	./$^.out

openmp-run: openmp
	./$^.out

pthreads-dbug: pthreads
	gdb ./$^.out

openmp-dbug: openmp
	gdb ./$^.out

clean:
	trash-put -v *.out
