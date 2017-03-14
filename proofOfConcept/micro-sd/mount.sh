#!/bin/bash
file="/media/store/sd-card"
if [ -d "$file" ]
then
	echo "$file found."
else
	mkdir -p /media/store/sd-card
fi

mloc="/dev/mmcblk0p1"
if [ -b "$mloc" ]
then
	mount /dev/mmcblk0p1 /media/store/sd-card
	echo "mounted sdcard"
	echo "to unmount: umount /media/store/sd-card"
else
	echo "sd-card not available for mount"
fi


