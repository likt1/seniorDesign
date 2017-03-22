#!/bin/bash

# create timestamp variable
tmp=$(date)
DATE=$(echo $tmp | sed 's/ //g') 

# convert file (may need to be done in circular buffer)
sox -r 44100 -e unsigned -b 16 -c 1 ~/$DATE.raw ~/$DATE.wav

# sync our wav with the cloud
python3 /root/seniorDesign/finalApp/sendToDropbox.py $DATE.wav

./copyToSD /root/$DATE.raw 
./copyToSD /root/$DATE.wav 
