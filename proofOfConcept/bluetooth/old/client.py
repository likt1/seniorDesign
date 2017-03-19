
import bluetooth

bd_addr = "00:19:86:00:05:C0"
port = 1

sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )
sock.connect((bd_addr, port))

sock.send("hello!!")

data = sock.recv(1024)
data = data.decode('UTF-8')
print("received [%s]" % data)


sock.close()
