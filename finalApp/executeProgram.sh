#!/bin/bash

# in production, our services need to be delayed on boot in order to wait for all service configs to lock down

sleep 30s

$1

# example usage:
#"/root/seniorDesign/finalApp/executeProgram.sh /root/seniorDesign/finalApp/gpio/EnactPWM.py"
