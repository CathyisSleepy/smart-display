# import libraries
import sys
import os
import socket
from PySide2.QtCore import *

HOST = '169.254.207.250'    # The remote host
PORT = 8888              # The same port as used by the server
s = None
for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM):
    af, socktype, proto, canonname, sa = res
    try:
        s = socket.socket(af, socktype, proto)
    except OSError as msg:
        s = None
        continue
    try:
        s.connect(sa)
    except OSError as msg:
        s.close()
        s = None
        continue
    break
'''if s is None:
    print('could not open socket')
    sys.exit(1)'''

# class to handle button controls
class Setting(QObject):
    motor_speed = 100

    #ask politely for motor_speed from clearcore
    @Slot(result=int)
    def motorSpeedGet(self):
        return int(self.motor_speed)

    #tell clearcore what to set the motor speed to
    @Slot(int)
    def motorSpeedSet(self, val):
        s.send(b'<m' + chr(val).encode('utf8') + b'>')
        self.motor_speed = val

    #tell clearcore to stop main loop
    @Slot()
    def ccStart(self):
        s.send(b'<l' + chr(1).encode('utf8') + b'>') 

    #tell clearcore to start main loop
    @Slot()
    def ccStop(self):
        s.send(b'<l' + chr(0).encode('utf8') + b'>') 

    # close
    @Slot()
    def closeWindow(self):
        sys.exit()