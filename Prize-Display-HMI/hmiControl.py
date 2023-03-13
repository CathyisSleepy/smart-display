# import libraries
import sys
import os
import socket
import pickle
from hmiEthernet import EthHandler

try:
        from PySide2.QtCore import *
except ImportError:
        from PyQt5.QtCore import *
if 'PyQt5' in sys.modules:
        from PyQt5.QtCore import pyqtSignal as Signal, pyqtSlot as Slot
else:
        from PySide2.QtCore import Signal, Slot

# class to handle button controls
class Setting(QObject):

    #ask politely for motor_speed from clearcore
    @Slot(result=int)
    def motorSpeedGet(self):
        try:
            p = open('store.data', 'rb')
            EthHandler.motor_speed = pickle.load(p)
            p.close
        except:
             print("load failed")
        if not EthHandler.eth_fault:
            try:
                EthHandler.sock.send(b'<m' + chr(EthHandler.motor_speed).encode() + b'>')
            except:
                EthHandler.eth_fault = True
        return int(EthHandler.motor_speed)

    #tell clearcore what to set the motor speed to
    @Slot(int)
    def motorSpeedSet(self, val):
        if not EthHandler.eth_fault:
            try:
                EthHandler.sock.send(b'<m' + chr(val).encode() + b'>')
            except:
                EthHandler.eth_fault = True
            try:
                p = open('store.data', 'wb')
                pickle.dump(EthHandler.motor_speed, p)
                p.close
            except:
                print("save failed")
        EthHandler.motor_speed = val

    #tell clearcore to stop main loop
    @Slot()
    def ccStart(self):
        if not EthHandler.eth_fault:
            try:
                EthHandler.sock.send(b'<l' + chr(1).encode() + b'>')
            except:
                EthHandler.eth_fault = True

    #tell clearcore to start main loop
    @Slot()
    def ccStop(self):
        if not EthHandler.eth_fault:
            try:
                EthHandler.sock.send(b'<l' + chr(0).encode() + b'>')
            except:
                EthHandler.eth_fault = True

    @Slot()
    def ccReset(self):
        if EthHandler.eth_fault:
            EthHandler.sock = EthHandler.attemptEthConnect()
        if EthHandler.sock is not None:
            EthHandler.eth_fault = False
            try:
                EthHandler.sock.send(b'<r' + b'>')
            except:
                EthHandler.sock = None
                EthHandler.eth_fault = False
        else:
            EthHandler.eth_fault = True

    # close
    @Slot()
    def closeWindow(self):
        sys.exit()