mpiio: mpiio.c
	mpicc -std=c99 -g -Wall -o mpiio mpiio.c

clean:
	rm mpiio
