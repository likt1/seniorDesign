#!/usr/bin/python3
import subprocess
import sys
from subprocess import Popen, PIPE, STDOUT
from time import sleep

log1 = open("log1","wb")
log2 = open("log2","wb")

#passPhrase = input("please enter your passphrase: ");
passPhrase = 'A9E544E10F'
key = 'wifi_506583d9499c_436973636f3231323831_managed_wep'

def readProcOut(p):
    for line in iter(p.stdout.readline, b''):
        if not line: break
        print('{}'.format(line.rstrip()))
        sys.stdout.flush()
    for line in iter(p.stderr.readline, b''):
        if not line: break
        print('{}'.format(line.rstrip()))
        sys.stdout.flush()

# note: sleeps are necessary for proper buffering and flushing...
p = Popen(['connmanctl'], bufsize=1024, stdout=log1, stdin=PIPE, stderr=log2)
p.stdin.write(bytes('agent on\n','utf-8'))
p.stdin.flush()
sleep(1)
p.stdin.write(bytes('scan wifi\n','utf-8'))
p.stdin.flush()
sleep(1)
p.stdin.write(bytes('services\n','utf-8'))
p.stdin.flush()
sleep(1)
p.stdin.write(bytes('config ' + key + ' ipv4 dhcp\n','utf-8'))
p.stdin.flush()
sleep(1)
p.stdin.write(bytes('connect ' + key + '\n','utf-8'))
p.stdin.flush()
sleep(1)
p.stdin.write(bytes(passPhrase+'\n','utf-8'))
p.stdin.flush()
sleep(1)
p.stdin.write(bytes('exit\n','utf-8'))
p.stdin.flush()
