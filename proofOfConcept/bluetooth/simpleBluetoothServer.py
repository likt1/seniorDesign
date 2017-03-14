#!/usr/bin/python3
import bluetooth
import json
import time
import dropbox

import pdb

import wifiScript as w 
import encrypt as e

server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )

port = 0
server_sock.bind(("",port))
server_sock.listen(1)
print("listening on port %d" % port)

uuid = "00001101-0000-1000-8000-00805F9B34FB"
bluetooth.advertise_service( server_sock, "Inspirado Service", uuid )

"""
Network Objects:
ID                  [int]
SSID                [string]
SecurityType        [string]
ServiceKey          [string] <- specific to conmanctl
ConnectionStatus    [string]
"""

def handleCommand(cmd, parameters = ""):
    switch={"listNetworks":listNetworks,"connectToNetwork":connectToNetwork,"default_value":defaultReply}
    try:
        msg = switch[cmd](parameters)
    except KeyError:
        msg = switch["default_value"]()

    client_sock.send(msg.encode('utf-8'))
      
def listNetworks(parameters):
    #Get json object representing available networks from wifi script
    network_object = w.scanNetworks()
    json_string = json.dumps([element.__dict__ for element in network_object])
    prepend = '{"type":"listNetworks","networks":'
    append = '}'
    
    networks = prepend + json_string + append
    return networks

def connectToNetwork(parameters):
    # Goal: Run configure Network command with appropriate credentials from app
    # App should send json object with type connectToNetwork as well as ssid, username if necessary, and pwd 
    # w.configureNetwork(log_file, selected_network)

    # Hardcoded example  
    # network_item = w.NetworkItem("2","not connected","ZyXEL24474","wifi_88532ee764bd_5a7958454c3234343734_managed_psk","wpa-psk")
    
    print("[!] PARAMETERS [%s]" % parameters)
    network_item = w.NetworkItem(parameters['id'],parameters['status'], parameters['ssid'], parameters['serviceKey'], parameters['security'], parameters['username'])
    log_file = open("log","a")
    w.configureNetwork(log_file, network_item)

    return "[!] CONNECTION ATTEMPTED"

def defaultReply():
    return "[!] Not a valid command"

while True:
    print("Waiting for a connection...")
    
    try: 
        client_sock,address = server_sock.accept()
        print("[!] Accepted connection from ",address)
        greeting = b"\x41\x43\x4B"                  #Send ACK to acknowledge connection
        client_sock.send(greeting)
        handleCommand('listNetworks')
        
        while True:
            data = client_sock.recv(1024)
            data = data.decode('UTF-8')
            print("[!] RECEIVED: [%s]" % data)
            if data:
                parameters = ""
                try:
                    command = json.loads(data)
                    command_type = command['type']                    
                    parameters = command['parameters']
                    print("[!] COMMAND TYPE [%s]" % command['type'])
                except:
                    pass
                message = handleCommand(command_type,parameters)
            else:
                break
    except Exception as e:
        print(e)
    except KeyboardInterrupt:
        print("[E] Stopping...")
        bluetooth.stop_advertising(server_sock)
        client_sock.close()
        server_sock.close()
        break     

