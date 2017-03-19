## Makefile for Inspirado (Circular Buffer) program(s)
ASM=pasm
CC=gcc
CFLAGS=-I. -lpthread -lprussdrv


## Targets
circularBuffer: circularBuffer.o
	$(CC) -o circularBuffer $(CFLAGS)

copyToSD: portable-media-ops.o
	$(CC) -o copyToSD portable-media-ops.o

adcSample: adcSample.p
	$(ASM) -b adcSample.p

mem2file: mem2file.o
	$(CC) -o mem2file mem2file.o


## Object files
circularBuffer.o: circularBuffer.c circularBuffer.h portable-media-ops.h
	$(CC) -o circularBuffer circularBuffer.c $(CFLAGS)

mem2file.o: mem2file.c
	$(CC) -o mem2file mem2file.c

portable-media-ops.o: portable-media-ops.c portable-media-ops.h
	$(CC) -c portable-media-ops.c $(CFLAGS)


## Remove object and executable files
clean:
	rm *.o pom-example circularBuffer mem2file
