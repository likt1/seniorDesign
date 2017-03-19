#!/bin/bash

# simple program to prove RGB writing via digital io

# enable gpio pins
#echo 67 > /sys/class/gpio/export
#echo 68 > /sys/class/gpio/export
#echo 44 > /sys/class/gpio/export

echo low > /sys/class/gpio/gpio67/direction
echo low > /sys/class/gpio/gpio68/direction
echo low > /sys/class/gpio/gpio44/direction

sleep 1

echo high > /sys/class/gpio/gpio67/direction
echo low > /sys/class/gpio/gpio68/direction
echo low > /sys/class/gpio/gpio44/direction

sleep 1

echo low > /sys/class/gpio/gpio67/direction
echo high > /sys/class/gpio/gpio68/direction
echo low > /sys/class/gpio/gpio44/direction

sleep 1

echo low > /sys/class/gpio/gpio67/direction
echo low > /sys/class/gpio/gpio68/direction
echo high > /sys/class/gpio/gpio44/direction

sleep 1

echo high > /sys/class/gpio/gpio67/direction
echo high > /sys/class/gpio/gpio68/direction
echo high > /sys/class/gpio/gpio44/direction

sleep 1

echo low > /sys/class/gpio/gpio67/direction
echo low > /sys/class/gpio/gpio68/direction
echo low > /sys/class/gpio/gpio44/direction

