#!/usr/bin/python3
# script to automate wifi setup via connmanctl
from subprocess import call
import subprocess
import os

# start fresh
if(os.path.isfile("log")):
	os.remove("log")
if(os.path.isfile("networks")):
	os.remove("networks")

networks = open("networks","wb")
log = open("log","wb")

command = ["connmanctl","tether","wifi","disable"]
subprocess.call(command,stdout=log,stderr=log)

command = ["connmanctl","agent","on"]
subprocess.call(command,stdout=log,stderr=log)

command = ["connmanctl","scan","wifi"]
subprocess.call(command,stdout=log,stderr=log)

command = ["connmanctl","services"]
subprocess.call(command,stdout=networks,stderr=log)

# must close before reading
networks.close()
networks_file = open("networks","r")
available_networks = networks_file.read()
print("available networks: \n",available_networks)

networks_file.close()
log.close()

#config wifi_****_****_managed_psk ipv4 dhcp
#connect wifi_****_****_managed_psk
#<enter_passphrase_here>
#quit
#ping google.com
