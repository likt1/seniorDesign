#!/usr/bin/python3
import bluetooth
import json
import time

import wifiScript as w 

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

def handleCommand(cmd):
    switch={"ListWifi":listNetworks,"default_value":defaultReply}
    try:
        msg = switch[cmd]()
    except KeyError:
        msg = switch["default_value"]()

    client_sock.send(msg.encode('utf-8'))
      
def listNetworks():
    #Get json object representing available networks from wifi script
    network_object = w.scanNetworks()
    json_string = json.dumps([element.__dict__ for element in network_object])
    prepend = '{"type":"listNetworks","networks":'
    append = '}'
    
    networks = prepend + json_string + append
    return networks

def defaultReply():
    return "Not a valid command"

while True:
    print("Waiting for a connection...")
    client_sock,address = server_sock.accept()
    
    try: 
        print("Accepted connection from ",address)
        greeting = b"\x41\x43\x4B"                  #Send ACK to acknowledge connection
        client_sock.send(greeting)
        handleCommand('ListWifi')
        
        while True:
            data = client_sock.recv(1024)
            data = data.decode('UTF-8')
            print("received [%s]" % data)
            if data:
                try:
                    test = json.loads(data)
                    print(test['network']) 
                except:
                    pass
                message = handleCommand(data)
            else:
                break
    except Exception as e:
        print(e)
        #client_sock.close()
    

