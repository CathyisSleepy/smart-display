import sys
import os
import socket
from hmiEthernet import EthHandler

try:
        from PySide2.QtCore import *
except ImportError:
        from PyQt5.QtCore import *
if 'PyQt5' in sys.modules:
        from PyQt5.QtCore import pyqtSignal as Signal, pyqtSlot as Slot
else:
        from PySide2.QtCore import Signal, Slot


class Streaming(QThread):
    EstopSignal = Signal(bool)
    FaultSignal = Signal(bool)
    RunSignal = Signal(bool)
    EthSignal = Signal(bool)
    MotorSignal = Signal(int)

    if EthHandler.eth_fault:
        EthHandler.sock = EthHandler.attemptEthConnect()
        if EthHandler.sock is not None:
            EthHandler.eth_fault = False
        else:
            EthHandler.eth_fault = True

    def __init__(self):
        super().__init__()

    def run(self):
        data = None
        while True:
            if EthHandler.sock is not None and EthHandler.eth_fault != True:
                try:
                    data = EthHandler.sock.recv(86).decode()
                    EthHandler.sock.send(b'')
                except:
                    EthHandler.sock.close()
                    EthHandler.eth_fault = True
                if data:
                    datalist = data.split("|")
                    for msg in datalist:
                        tag = msg.split(",")
                        #print("tag is:" + str(tag[0]))
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
                                continue

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
                                continue
                                
                        elif tag[0] == "run" or tag[0] == "running":
                            try: 
                                run = int(tag[1])
                            except:
                                print("failed to cast run to int")
                                print("run invalid input: " + str(tag[1]))
                                continue
                                
                            if run == 1 or run == 0:
                                self.RunSignal.emit(bool(run))
                                print("run: " + str(run))
                            else:
                                print("run input invalid: " + str(run))
                                continue

                        elif tag[0] == "speed":
                            try: 
                                speed = int(tag[1])
                            except:
                                print("failed to cast speed to int")
                                print("run invalid input: " + str(tag[1]))
                                continue
                                
                            if speed >= -255 and speed <= 255:
                                adj_speed = speed / 2550
                                self.MotorSignal.emit(int(adj_speed))
                                print("speed: " + str(speed))
                            else:
                                print("speed input invalid")
                                continue
            else:
                EthHandler.sock = EthHandler.attemptEthConnect()
                if EthHandler.sock is not None:
                    EthHandler.eth_fault = False
                else:
                    EthHandler.eth_fault = True
            
            self.EthSignal.emit(bool(EthHandler.eth_fault))
            print(EthHandler.eth_fault)
            self.MotorSignal.emit(int(EthHandler.motor_speed))