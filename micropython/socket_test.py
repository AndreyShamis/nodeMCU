import socket
import os

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(["192.168.1.1", 80])
except Exception as ex:
    print("Issue found :" + str(ex))
s.close()
