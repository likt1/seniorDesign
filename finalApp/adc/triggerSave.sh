#!/bin/bash

# $1 represents raw file, $2 represents compression type

# create timestamp variable
tmp=$(date)
DATE=$(echo $tmp | sed 's/ //g') 
DATE=$(echo $DATE | sed 's/:/_/g') 

# convert file (may need to be done in circular buffer)
sox -r 44100 -e unsigned -b 16 -c 1 ~/$1 ~/$DATE-r.$2
#sox ~/$DATE-r.$2 ~/$DATE.$2 noisered ~/conf/noise.prof 0.1 silence 1 0.1 1%

# this line to cleans up noise
sox ~/$DATE-r.$2 ~/$DATE.$2 noisered ~/conf/noise.prof 0.1

# sync our wav with the cloud
python3 /root/seniorDesign/finalApp/dropBox/sendToDropbox.py $DATE.$2

#/root/seniorDesign/finalApp/adc/copyToSD /root/$1 # For development testing 
/root/seniorDesign/finalApp/adc/copyToSD /root/$DATE.$2

# TODO add in rm to remove from local memory if needed
