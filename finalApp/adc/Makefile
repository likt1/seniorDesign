## Makefile for Inspirado (Circular Buffer) program(s)
ASM=pasm
CC=gcc
CFLAGS=-I. -lpthread -lprussdrv


## Targets
all: circularBuffer copyToSD adcSample.bin

circularBuffer: circularBuffer.o
	$(CC) -o circularBuffer circularBuffer.o $(CFLAGS)

copyToSD: portable-media-ops.o
	$(CC) -o copyToSD portable-media-ops.o

adcSample.bin: adcSample.p
	$(ASM) -b adcSample.p


## Object files
circularBuffer.o: circularBuffer.c circularBuffer.h #portable-media-ops.h
	$(CC) -c circularBuffer.c $(CFLAGS)

portable-media-ops.o: portable-media-ops.c portable-media-ops.h
	$(CC) -c portable-media-ops.c $(CFLAGS)


## Remove object and executable files
clean:
	rm *.o adcSample.bin circularBuffer copyToSD
