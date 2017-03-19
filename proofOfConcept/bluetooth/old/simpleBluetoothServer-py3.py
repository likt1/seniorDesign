#!/usr/bin/python3
from bluetooth import *
import socket

server_sock=BluetoothSocket( RFCOMM )

port = 0
server_sock.bind(("",port))
server_sock.listen(1)
print("listening on port " + str(port))

uuid = "00001101-0000-1000-8000-00805F9B34FB"
advertise_service( server_sock, "Inspirado Service", service_id = uuid, service_classes = [ uuid, SERIAL_PORT_CLASS ], profiles = [ SERIAL_PORT_PROFILE ] )

while socketOpen:
    print("Waiting for a connection...")
    client_sock,address = server_sock.accept()
    try:
        print("Accepted connection from " + str(address))
        while True:
            data = client_sock.recv(1024)
            print("received [" + data.decode("utf-8") + "]")
            toSend = "generic ack to phone..."

            if "hi" in data.decode("utf-8"):
                toSend = "hi there"

            # NOTE: disconnecting the server without disconnecting the client
            # may cause server->client message issues, connecting a couple times
            # in the phone app should resolve this for now...
            if "quit" in data.decode("utf-8"):
                client_sock.close()
                print("quitting server...")
                socketOpen = False
                break;

            print('sending ' + toSend + ' now...')
            client_sock.send(toSend.encode('utf-8'))

    except Exception as e:
        print('exception raised\n', e)
        client_sock.close()
        print("quitting server...")
        socketOpen = False
