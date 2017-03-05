#!/usr/bin/python3
# script to automate wifi setup via connmanctl
import os
import subprocess
import re

from subprocess import Popen, PIPE, STDOUT
from time import sleep

# object to store key network data...
class NetworkItem(object):
    def __init__(self, id, status, ssid, serviceKey, security):
        self.id = id
        self.status = status
        self.ssid = ssid
        self.serviceKey = serviceKey
        self.security = security


# start fresh
if(os.path.isfile("log")):
    os.remove("log")
if(os.path.isfile("networks")):
    os.remove("networks")

# config file templates to be filled out based on selected network
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

        # detect security type of scanned networks
        security = 'unknown'
        if "none" in network_item[-1]:
            security = 'open'
        elif "wep" in network_item[-1]:
            security = 'wep'
        elif "psk" in network_item[-1]:
            security = 'wpa-psk'
        elif "ieee8021x" in network_item[-1]:
            security = 'peap'
    
        # fill object accordingly
        if (len(network_item) >= 3):
            # detect if connected via first string (i.e. "*AO" means connected)
            status = network_item[0]
            if('O' in status):
                status = 'connected'
            else:
                status = 'available'
            network_items.append(NetworkItem(i,status,network_item[1],network_item[2],security))
        elif(len(network_item) >= 2):
            network_items.append(NetworkItem(i,"not connected", network_item[0], network_item[1],security))
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

def promptNetworks(log_file, selected_network_id=-1):
    selection = ""
    if selected_network_id >= 0:
        selection = str(selected_network_id)
    else: 
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

def configSettings(log_file, ssid, skey):
    settings_file_name = "/var/lib/connman/" + skey + "/settings"

    if os.path.exists(settings_file_name):
        config_file_line = "Config.file=" + ssid.lower() + "\n"
        config_ident_line = "Config.ident=service_" + skey + "\n"

        settings_file = open(settings_file_name,'r')
        settings_old = settings_file.read()
        settings_file.close()
        settings_file = open(settings_file_name,'w')

        lines = settings_old.split('\n')

        for line in lines:
            if "Config.file" in line:
                settings_file.write(config_file_line)
            elif "Config.ident" in line:
                settings_file.write(config_ident_line)
            elif line:
                settings_file.write(line + "\n")

        if "Config.file" not in settings_old:
            settings_file.write(config_file_line)
        if "Config.ident" not in settings_old:
            settings_file.write(config_ident_line)

        settings_file.close()
    else:
        log_file.write("error, could not find " + settings_file_name + " for " + ssid + "\n")

def configureNetwork(log_file, selected_network, network_id="", passphrase=""):
    log_file.close()    
    log = open("log","ab")
    
    # based on selected security, (local only) prompt for username / password...
    if not passphrase and not network_id:
        if "peap" in selected_network.security:
            networkId = input("please enter your network identity (dont care for single_auth networks): ");
        if "open" not in selected_network.security:
            passPhrase = input("please enter your passphrase: ");
    
    # start session so we can utilize agent utility
    p = Popen(['connmanctl'], bufsize=64, stdout=log, stdin=PIPE, stderr=log)
    p.stdin.write(bytes('agent on\n','utf-8'))
    p.stdin.flush()
    sleep(0.25)
    p.stdin.writ    e(bytes('config ' + selected_network.serviceKey + ' ipv4 dhcp\n','utf-8'))
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
    
    # create config file to config networks with passphrases
    if "open" not in selected_network.security:
        config_file_name = "/var/lib/connman/" + selected_network.ssid.lower() + ".config"
        config_file = open(config_file_name,"w")
        skey = selected_network.serviceKey
        config_content = peap_template
        if "peap" not in selected_network.security:
            config_content = single_auth_template
        config_content = config_content.replace("<NETID>",networkId)
        config_content = config_content.replace("<PASSPHRASE>",passPhrase)
        config_content = config_content.replace("<SERVICEKEY>",selected_network.serviceKey)
        config_content = config_content.replace("<SSID>",selected_network.ssid)
        config_file.write(config_content)
        config_file.close()

    # link config file to connman service
    log.close()
    log_file = open("log","a")
    configSettings(log_file, selected_network.ssid, selected_network.serviceKey)
    log_file.close()
    log = open("log","wb")

    # validate it worked / re-attempt connection
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
