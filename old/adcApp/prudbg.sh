#!/bin/bash

./build.sh
./devmem/devmem2 0x4a300004 w 0x12341234
./circularBuffer

