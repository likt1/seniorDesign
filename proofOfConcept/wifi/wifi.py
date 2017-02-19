#!/usr/bin/python3
# script to automate wifi setup via connmanctl
import os
import subprocess
import re

from subprocess import Popen, PIPE, STDOUT
from time import sleep

# object to store key network data...
# todo detect type (wifi, managed, etc.)
class NetworkItem(object):
    def __init__(self, status, ssid, serviceKey, id):
        self.status = status
        self.ssid = ssid
        self.serviceKey = serviceKey
        self.id = id

# start fresh
if(os.path.isfile("log")):
    os.remove("log")
if(os.path.isfile("networks")):
    os.remove("networks")

peap_template = "[global]\nName = <SSID>\nDescription = wifi.py autogen config PEAP\n\n[service_<SERVICEKEY>]\nType = wifi\nName = <SSID>\nEAP = peap\nPhase2 = MSCHAPV2\nIdentity = <NETID>\nPassphrase = <PASSPHRASE>\n"

single_auth_template = "[service_<SERVICEKEY>]\nType = wifi\nName = <SSID>\nPassphrase = <PASSPHRASE>\n"

def setupWifi():
    networks = open("networks","wb")
    log = open("log","wb")
    
    print("untethering wifi...")
    command = ["connmanctl","tether","wifi","disable"]
    subprocess.call(command,stdout=log,stderr=log)

    print("enabling wifi...")
    command = ["connmanctl","enable","wifi"]
    subprocess.call(command,stdout=log,stderr=log)

    networks.close()
    log.close()

def scanNetworks(log_file):
    networks = open("networks","wb")
    log = open("log","ab")
    network_items = []

    print("scanning for available networks...")
    command = ["connmanctl","scan","wifi"]
    subprocess.call(command,stdout=log,stderr=log)

    command = ["connmanctl","services"]
    subprocess.call(command,stdout=networks,stderr=log)

    # must close before reading
    networks.close()
    log.close()
    networks_file = open("networks","r")

    available_networks = networks_file.readlines()

    i = 0
    for network_line in available_networks:
        # splice & clean the list of values
        network_item = network_line.strip().split(" ")
        network_item = list(filter(('').__ne__, network_item))
        
        # this script only cares about wifi items...
        if "wifi" not in network_item[-1]:
            continue
    
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
    networks_file.close()
    return network_items

def showNetworks(log_file, network_items):
    print(str(len(network_items)) + " networks are available: ")
    for item in network_items:
        print("(" + str(item.id) + ") " + item.ssid + " - " + item.status)
    print("")

def promptNetworks(log_file):
    selection = input("please select a network you wish to connect to: ");
    selected_network = ""
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
    return selected_network

def configureNetwork(log_file, selected_network):
    log_file.close()    
    log = open("log","wb")
    
    networkId = input("please enter your network identity (dont care for single_auth networks): ");
    passPhrase = input("please enter your passphrase: ");
    
    # start session so we can utilize agent utility
    p = Popen(['connmanctl'], bufsize=64, stdout=log, stdin=PIPE, stderr=log)
    p.stdin.write(bytes('agent on\n','utf-8'))
    p.stdin.flush()
    sleep(0.25)
    p.stdin.write(bytes('config ' + selected_network.serviceKey + ' ipv4 dhcp\n','utf-8'))
    p.stdin.flush()
    sleep(0.25)
    p.stdin.write(bytes('connect ' + selected_network.serviceKey + '\n','utf-8'));
    p.stdin.flush()
    sleep(0.75)
    p.stdin.write(bytes(passPhrase+'\n','utf-8'))
    p.stdin.flush()
    sleep(0.5)
    p.stdin.write(bytes('exit\n','utf-8'))
    p.stdin.flush()
    sleep(0.5)
    
    config_file_name = "/var/lib/connman/" + selected_network.ssid.lower() + ".config"
    #config_file_name = "/var/lib/connman/wifi.config"
    config_file = open(config_file_name,"w")

    # create config file to config peap
    skey = selected_network.serviceKey
    config_content = peap_template
    if "wep" in skey or "wpa" in skey or "psk" in skey:
        config_content = single_auth_template

    # link config file to connman service
    #settings_file_name = "/var/lib/connman/" + selected_network.serviceKey + "/settings"
    #settings_old = open(settings_file_name,'r').read()
    #if "Config.file" in settings_old
    #settings_new = settings_old
    
    # Config.file=<SSIDLOWER>
    # Config.ident=service_<SERVICEKEY>

    config_content = config_content.replace("<NETID>",networkId)
    config_content = config_content.replace("<PASSPHRASE>",passPhrase)
    config_content = config_content.replace("<SERVICEKEY>",selected_network.serviceKey)
    config_content = config_content.replace("<SSID>",selected_network.ssid)
    
    config_file.write(config_content)

    config_file.close()

    print("rescanning networks...")
    command = ["connmanctl", "scan","wifi"]
    subprocess.call(command,stdout=log,stderr=log)
    command = ["connmanctl","services"]
    subprocess.call(command,stdout=log,stderr=log)
    
    print("Attempting connection...")
    command = ["connmanctl","connect",selected_network.serviceKey]
    subprocess.call(command,stdout=log,stderr=log)

    log.close()
    log_file = open("log","a")

# main...
setupWifi()
log_file = open("log","a")
network_items = scanNetworks(log_file)
showNetworks(log_file, network_items)
chosen_network = promptNetworks(log_file)
if chosen_network != "":
    configureNetwork(log_file,chosen_network)

log_file.close()
