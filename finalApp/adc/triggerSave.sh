#!/bin/bash

# convert file (may need to be done in circular buffer)
sox -r 44100 -e unsigned -b 16 -c 1 ~/testOut.raw ~/testOut-44k1.wav

# sync our wav with the cloud
python3 ../dropBox.py testOut-44k1.wav

# check if mounted / available the copy audio files over
if [ -b "/dev/mmcblk0p1" ]
then
  ~/mount.sh
  cp ~/*.raw /media/store/sd-card
  mv ~/*.wav /media/store/sd-card
  #mv ~/*.mp3 /media/store/sd-card
  #mv ~/*.flac /media/store/sd-card
  #mv ~/*.ogg /media/store/sd-card
  ~/umount.sh
else
  echo "sdcard not found"
fi
