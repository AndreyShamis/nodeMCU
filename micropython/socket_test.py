import socket
import os

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(["192.168.1.1", 80])
s.close()
