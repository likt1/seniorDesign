#!/bin/bash

# convert file (may need to be done in circular buffer)
sox -r 44100 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-44k1.wav

# sync our wav with the cloud
#python3 ../dropBox.py testOut-44k1.wav

./copyToSD /root/testOut.raw 
./copyToSD /root/testOut-44k1.wav 
