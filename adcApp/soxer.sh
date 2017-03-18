#!/bin/bash

sox -r 200000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-200k.wav
sox -r 150000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-150k.wav
sox -r 100000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-100k.wav
sox -r 90000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-90k.wav
sox -r 60000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-60k.wav
sox -r 44100 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-44k1.wav
sox -r 30000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-30k.wav
sox -r 10000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-10k.wav
sox -r 5000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-5k.wav
sox -r 3000 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-3k.wav

if [ -b "/dev/mmcblk0p1" ]
then
  ~/mount.sh
  cp ~/*.raw /media/store/sd-card
  mv ~/*.wav /media/store/sd-card
  ~/umount.sh
else
  echo "sdcard not found"
fi
