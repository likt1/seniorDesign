#!/bin/bash

gcc -o circularBuffer circularBuffer.c -lpthread -lprussdrv
gcc -o mem2file mem2file.c
pasm -b adcSample.p
