CC=gcc
CFLAGS=-I.

pom-example: portable-media-ops.o
	$(CC) -o pom-example portable-media-ops.o $(CFLAGS)

portable-media-ops.o: portable-media-ops.c portable-media-ops.h
	$(CC) -c portable-media-ops.c $(CFLAGS)

clean:
	rm *.o pom-example
