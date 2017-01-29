#!/usr/bin/python3
# script to automate wifi setup via connmanctl
import os
import subprocess
import re

# start fresh
if(os.path.isfile("log")):
    os.remove("log")
if(os.path.isfile("networks")):
    os.remove("networks")

networks = open("networks","wb")
log = open("log","wb")

print("untethering wifi...")
command = ["connmanctl","tether","wifi","disable"]
subprocess.call(command,stdout=log,stderr=log)

command = ["connmanctl","agent","on"]
subprocess.call(command,stdout=log,stderr=log)

print("scanning for available networks...")
command = ["connmanctl","scan","wifi"]
subprocess.call(command,stdout=log,stderr=log)

command = ["connmanctl","services"]
subprocess.call(command,stdout=networks,stderr=log)

# must close before reading
networks.close()
log.close()
networks_file = open("networks","r")
log_file = open("log","w")

available_networks = networks_file.readlines()

class NetworkItem(object):
    def __init__(self, status, ssid, serviceKey, id):
        self.status = status
        self.ssid = ssid
        self.serviceKey = serviceKey
        self.id = id

network_items = []

i = 0
for network_line in available_networks:
    # splice & clean the list of values
    network_item = network_line.strip().split(" ")
    network_item = list(filter(('').__ne__, network_item))
    
    # fill object accordingly
    if (len(network_item) >= 3):
        # detect if connected via first string (i.e. "*AO" means connected)
        status = network_item[0]
        if('O' in status):
            status = 'connected'
        else:
            status = 'available'
        network_items.append(NetworkItem(status,network_item[1],network_item[2],i))
    elif(len(network_item) >= 2):
        network_items.append(NetworkItem("not connected", network_item[0], network_item[1],i))
    else:
        i -= 1
    i += 1

print(str(len(network_items)) + " networks are available: ")
for item in network_items:
    print("(" + str(item.id) + ") " + item.ssid + " - " + item.status)
print("")

networks_file.close()

selection = input("please select a network you wish to connect to: ");
selected_network = network_items[0]
success = False

if(selection.isdigit()):
    if(int(selection) < len(network_items) and int(selection) >= 0):
        selected_network = network_items[int(selection)]
        success = True
    else:
        log_file.write("error, network selection out of bounds\n")
else:
    log_file.write("error, network selection invalid\n")

if(success == True):
    # selection valid.. configure network here...
    print("valid input!")
else:
    print("invalid input...")

#config wifi_****_****_managed_psk ipv4 dhcp
#connect wifi_****_****_managed_psk
#<enter_passphrase_here>
#quit
#ping google.com
