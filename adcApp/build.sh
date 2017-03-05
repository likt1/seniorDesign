#!/bin/bash

gcc -o circularBuffer circularBuffer.c -lpthread -lprussdrv
pasm -b adcSample.p
