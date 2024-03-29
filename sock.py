import socket
from time import sleep

ip = "127.0.0.1"
port = 1234

socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.connect((ip, port))
print("Connection on {}".format(port))

#test envoi de paquets
for i in range(10):
    b = bytes(str(i)*10, 'utf-8')
    socket.send(b)
    sleep(2)

print("Close")
socket.close()


