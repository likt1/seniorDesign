import bluetooth

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

def handle_command(cmd):
    switch={"ListWifi":list_networks,"default_value":default_reply}
    try:
        msg = switch[cmd]()
    except KeyError:
        print("Send default")
        msg = switch["default_value"]()

    client_sock.send(msg.encode('utf-8'))
      
def list_networks():
    #Hardcoded for now
    return "SecureWireless,FBISurveillanceVan,PrettyFlyForAWiFi"

def default_reply():
    return "Not a valid command"

while True:
    print("Waiting for a connection...")
    client_sock,address = server_sock.accept()
    try: 
        print("Accepted connection from ",address)
        greeting = b"\x41\x43\x4B"
        client_sock.send(greeting)
        while True:
            data = client_sock.recv(1024)
            print("received [%s]" % data.decode('UTF-8'))
            if data:               
                message = handle_command(data.decode('utf-8'))
            else:
                break
    except Exception as e:
        print(e)
        #client_sock.close()
    

