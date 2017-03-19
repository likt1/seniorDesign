#!/bin/bash

mount /dev/mmcblk0p1 /mnt
cp ../../testOut.raw /mnt/
umount /mnt

