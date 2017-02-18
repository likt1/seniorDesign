import bluetooth

server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )

port = bluetooth.bluez._get_available_port( bluetooth.RFCOMM )
server_sock.bind(("",port))
server_sock.listen(1)
print "listening on port %d" % port

uuid = "00001101-0000-1000-8000-00805F9B34FB"
bluetooth.advertise_service( server_sock, "Inspirado Service", uuid )

while True:
    print "Waiting for a connection..."
    client_sock,address = server_sock.accept()
    try: 
        print "Accepted connection from ",address
        while True:
            data = client_sock.recv(1024)
            print "received [%s]" % data
            if data:
                client_sock.send(data)
            else:
                break
    except:
        client_sock.close()
        


