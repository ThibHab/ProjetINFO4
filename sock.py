import socket
from time import sleep

ip = "127.0.0.1"
port = 1234

socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.connect((ip, port))
print("Connection on {}".format(port))
def checkCable(data):
    inp=(5,7,9)
    out=(1,2,3,4,6,8)
    error=False
    cables=data.split("/")
    for c in cables:
        if "10" in c:
            error=True
            break
        part=c.split(":")
        l,r=part[0],part[1]
        if((l in inp and r in inp) or(l in out and r in out)):
            error=True
            print("CABLAGE"+l+"---"+r+" IMPOSSIBLE")


#test envoi de paquets
a=str(input(">"))
while(a!=""):
    b = bytes(a, 'utf-8')
    socket.send(b)
    a=str(input(">"))
print("Close")
socket.close()


