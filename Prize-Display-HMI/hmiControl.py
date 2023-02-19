# import libraries
import sys
import os
import socket, time

try:
        from PySide2.QtCore import *
except ImportError:
        from PyQt5.QtCore import *
if 'PyQt5' in sys.modules:
        from PyQt5.QtCore import pyqtSignal as Signal, pyqtSlot as Slot
else:
        from PySide2.QtCore import Signal, Slot

'''
#For testing
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)
'''

HOST = '169.254.207.250'    # The remote host
PORT = 8888              # The same port as used by the server

class EthHandler():

    motor_speed = 0
    eth_fault = True
    sock = None

    def attemptEthConnect():
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
        return s

    sock = attemptEthConnect()

    if sock is None:
        print('could not open socket')
        eth_fault = True
        print("current eth = " + str(eth_fault))
    else:
        eth_fault = False

print(EthHandler.eth_fault)

# class to handle button controls
class Setting(QObject):

    #ask politely for motor_speed from clearcore
    @Slot(result=int)
    def motorSpeedGet(self):
        return int(EthHandler.motor_speed)

    #tell clearcore what to set the motor speed to
    @Slot(int)
    def motorSpeedSet(self, val):
        if not EthHandler.eth_fault:
            EthHandler.sock.send(b'<m' + chr(val).encode() + b'>')

        EthHandler.motor_speed = val

    #tell clearcore to stop main loop
    @Slot()
    def ccStart(self):
        if not EthHandler.eth_fault:
            EthHandler.sock.send(b'<l' + chr(1).encode() + b'>') 

    #tell clearcore to start main loop
    @Slot()
    def ccStop(self):
        if not EthHandler.eth_fault:
            EthHandler.sock.send(b'<l' + chr(0).encode() + b'>')

    @Slot()
    def ccReset(self):
        if EthHandler.eth_fault:
            EthHandler.sock = EthHandler.attemptEthConnect()
        if EthHandler.sock is not None:
            EthHandler.eth_fault = 0
            EthHandler.sock.send(b'<r' + b'>')
        else:
            EthHandler.eth_fault = 1

    # close
    @Slot()
    def closeWindow(self):
        sys.exit()

    @Slot(result=bool)
    def getEthFault(self):
        return EthHandler.eth_fault

class Streaming(QThread):
    EstopSignal = Signal(bool)
    FaultSignal = Signal(bool)
    RunSignal = Signal(bool)

    if EthHandler.eth_fault:
        EthHandler.sock = EthHandler.attemptEthConnect()
        if EthHandler.sock is not None:
            EthHandler.eth_fault = 0
        else:
            EthHandler.eth_fault = 1

    def __init__(self):
        super().__init__()

    def run(self):
        data = None
        while True:
            if EthHandler.sock is not None and EthHandler.eth_fault == 0:
                try:
                    data = EthHandler.sock.recv(86).decode()
                    EthHandler.sock.send(b'-')
                except:
                    EthHandler.eth_fault = 1
                    break
                if data:
                    datalist = data.split("|")
                    for msg in datalist:
                        tag = msg.split(",")
                        print("tag is:" + str(tag[0]))
                        if tag[0] == "estop":
                            try:
                                estop = int(tag[1])
                            except:
                                print("failed to cast estop to int")
                                continue

                            if estop == 1 or estop == 0:
                                self.EstopSignal.emit(bool(estop))
                                print("estop: " + str(estop))
                            else:
                                print("estop input invalid")
                        elif tag[0] == "fault":
                            try:
                                fault = int(tag[1])
                            except:
                                print("failed to cast fault to int")
                                print("fault invalid input: " + str(tag[1]))
                                continue
                            
                            if fault == 1 or fault == 0:
                                self.FaultSignal.emit(bool(fault))
                                print("fault: " + str(fault))
                            else:
                                print("fault input invalid")
                        elif tag[0] == "run":
                            try: 
                                run = int(tag[1])
                            except:
                                print("failed to cast run to int")
                                print("run invalid input: " + str(tag[1]))
                                continue
                                
                            if run == 1 or run == 0:
                                self.RunSignal.emit(not bool(run))
                                print("run: " + str(run))
                            else:
                                print("run input invalid")
            else:
                EthHandler.sock = EthHandler.attemptEthConnect()
                if EthHandler.sock is not None:
                    EthHandler.eth_fault = 0
                else:
                    EthHandler.eth_fault = 1