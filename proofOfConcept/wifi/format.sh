#!/bin/bash

# file to auto replace all tabs with spaces in a file
# usage:
#        ./format.sh wifi.py

if [[ -z "$1" ]] 
then
	echo "please enter a file name"
else
	sed -i 's/\t/    /g' $1
fi

