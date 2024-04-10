import socket
from time import sleep

ip = "127.0.0.1"
port = 1234

socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.connect((ip, port))
print("Connection on {}".format(port))

#test envoi de paquets
a=str(input(">"))
while(a!=""):
    b = bytes(a, 'utf-8')
    socket.send(b)
    a=str(input(">"))
print("Close")
socket.close()


