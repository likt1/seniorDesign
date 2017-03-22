#!/bin/bash

# $1 represents raw file, $2 represents compression type

# create timestamp variable
tmp=$(date)
DATE=$(echo $tmp | sed 's/ //g') 

# convert file (may need to be done in circular buffer)
sox -r 44100 -e unsigned -b 16 -c 1 ~/$1 ~/$DATE.$2

# sync our wav with the cloud
python3 /root/seniorDesign/finalApp/sendToDropbox.py $DATE.$2

#./copyToSD /root/$1 # For development testing 
./copyToSD /root/$DATE.$2
