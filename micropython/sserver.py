#!/usr/bin/env python
# -*- coding: utf-8 -*-

import socket

SERVER_PORT = 9090

sock = socket.socket()
sock.bind(('', SERVER_PORT))
sock.listen(1)
EXIT = False
conn = None
try:
    while not EXIT:
        conn, addr = sock.accept()

        print 'connected:', addr
        while True:
            data = conn.recv(1024)

            if not data:
                EXIT = True
                break
            data = data.strip()
            if data == "EXIT":
                EXIT = True
                break
            print "Received data:|" + str(data) + "|"
            conn.send(data.upper())
except Exception as ex:
    print "Issue found: " + str(ex)

finally:
    print ("")
    print ("Closing connection")
    if conn:
        conn.close()
    print ("Connection closed")

import sys

sys._clear_type_cache()