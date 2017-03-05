import bluetooth
import json

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
    # Goal: Run the wifi script to scan for networks
    # Script returns list of network objects
    # Serialize into json string, return the json string
    networks = '{"type":"listNetworks","networks":[{"name":"SecureWireless"},{"name":"ThyNeighborsWifi"}]}'  #Hardcoded example
    return networks

def defaultReply():
    return "Not a valid command"

while True:
    try: 
        print("Waiting for a connection...")
        client_sock,address = server_sock.accept()
        print("Accepted connection from ",address)
        greeting = b"\x41\x43\x4B"                  #Send ACK to acknowledge connection
        client_sock.send(greeting)
        
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
    except KeyboardInterrupt:
        print("Stopping...")
        bluetooth.stop_advertising(server_sock)
        client_sock.close()
        server_sock.close()
        break     

