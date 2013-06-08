all: compile run

compile: ass1-pthreads.c
	gcc $^ -pthread -o $@

run: compiled
	./$^

clean:
	trash-put -v compile