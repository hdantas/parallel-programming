all: compile run

compile: ass1-pthreads.c
	gcc $^ -pthread -lm -o $@

run: compile
	./$^

clean:
	trash-put -v compile
