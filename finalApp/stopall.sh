#!/bin/bash
# kill all processes via PID files generated from runall.sh
pkill -F /root/cbufPID
pkill -F /root/btPID
pkill -F /root/dioPID
pkill -F /root/ledPID
