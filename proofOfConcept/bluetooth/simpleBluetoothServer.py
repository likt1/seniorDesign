#!/usr/bin/python3
import bluetooth
import json
import time
import dropbox
import threading
import configparser as ConfigParser

import pdb

import wifiScript as w 
import dioScript as d

server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )

port = 0
server_sock.bind(("",port))
server_sock.listen(1)
print("listening on port %d" % port)

uuid = "00001101-0000-1000-8000-00805F9B34FB"
bluetooth.advertise_service( server_sock, "Inspirado Service", uuid )

app_key = "1mqt8n70964mn8n"
app_secret = "3p9t99oz4i9cgg3"

"""
Network Objects:
ID                  [int]
SSID                [string]
SecurityType        [string]
ServiceKey          [string] <- specific to conmanctl
ConnectionStatus    [string]
"""

def handleCommand(cmd, parameters = ""):
    # Switch for handling various commands with associated parameters
    switch={"listNetworks":listNetworks,
        "connectToNetwork":connectToNetwork,
        "getAccessKey":getAccessKey,
        "listDIOValues":listDIOValues,
        "accessKeyToServer":accessKeyToServer,
        "control":handleControl,
        "default_value":defaultReply}
    try:
        msg = switch[cmd](parameters)
    except KeyError:
        msg = switch["default_value"]()

    #pdb.set_trace()

    client_sock.send(msg.encode('utf-8'))
      
def listNetworks(parameters):
    # Get json object representing available networks from wifi script
    network_object = w.scanNetworks()
    json_string = json.dumps([element.__dict__ for element in network_object])
    prepend = '{"type":"listNetworks","networks":'
    append = '}'
    
    networks_cmd = prepend + json_string + append
    return networks_cmd

def connectToNetwork(parameters):
    # Run configure Network command with appropriate credentials from app

    network_item = w.NetworkItem(parameters['id'],parameters['status'], parameters['ssid'], parameters['serviceKey'], parameters['security'])
    log_file = open("log","a")
    item = w.configureNetwork(log_file, network_item, parameters['username'],parameters['password'])

    #pdb.set_trace()
    print(item[0].__dict__)

    return "[!] CONNECTION ATTEMPTED"

def getAccessKey(parameters):
    # Prompt client to send its dropbox access key
    access_key_cmd = '{"type":"getAccessKey"}'
    return access_key_cmd

def accessKeyToServer(parameters):
    # Retrieve access key from the client
    access_key = parameters['access_key']    

    Config = ConfigParser.ConfigParser()
    cfgfile = open("/root/conf/config.ini",'w')
    Config.add_section("Dropbox")
    Config.set('Dropbox','app_key',app_key)
    Config.set('Dropbox','app_secret',app_secret)
    Config.set('Dropbox','access_key',access_key)

    Config.write(cfgfile)
    cfgfile.close()

    return "[!] Access key sent to server"

def handleControl(parameters):
    d.setValue(parameters['property'],parameters['value'])
    return "[!] Handled control command"

def listDIOValues(parameters):
    # Get json object representing DIO values
    dio_object = d.getValues() 
    # Ex: '{"format":"wav","time":"active","toggle"="True"}'
    json_string = dio_object.serialize()
    prepend = '{"type":"listDIO","settings":'
    append = '}'
    
    #pdb.set_trace()

    dio_cmd = prepend + json_string + append
    return dio_cmd 

def listDIOValuesOnInterval(interval, client_sock, parameters):
    while True:
        msg = listDIOValues("")
        client_sock.send(msg.encode('utf-8'))
        time.sleep(interval)    

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
        time.sleep(.1)
        handleCommand('listDIOValues')  
        time.sleep(.1)
        handleCommand('getAccessKey')
            
        t = threading.Thread(target=listDIOValuesOnInterval, args=(.3,client_sock,""))
        t.daemon = True
        t.start()

        while True:
            #handleCommand('listDIOValues') 
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

