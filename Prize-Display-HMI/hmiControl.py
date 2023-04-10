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

    #ask politely for motor_speed
    @Slot(result=int)
    def motorSpeedGet(self):
        try:
            p = open('speed.pick', 'rb')
            EthHandler.cmd_motor_speed = pickle.load(p)
            p.close
        except:
            print("load failed")
        EthHandler.attemptEthSend(b'<m' + chr(EthHandler.cmd_motor_speed).encode() + b'>')
        return int(EthHandler.cmd_motor_speed)

    #tell clearcore what to set the motor speed to
    @Slot(int)
    def motorSpeedSet(self, val):
        EthHandler.attemptEthSend(b'<m' + chr(val).encode() + b'>')
        try:
            p = open('speed.pick', 'wb')
            pickle.dump(EthHandler.cmd_motor_speed, p)
            p.close
        except:
            print("save failed")
        EthHandler.cmd_motor_speed = val

    #tell clearcore to stop main loop
    @Slot()
    def ccStart(self):
        EthHandler.cmd_running = 1
        EthHandler.attemptEthSend(b'<l' + chr(EthHandler.cmd_running).encode() + b'>')

    #tell clearcore to start main loop
    @Slot()
    def ccStop(self):
        EthHandler.cmd_running = 0
        EthHandler.attemptEthSend(b'<l' + chr(EthHandler.cmd_running).encode() + b'>')

    @Slot()
    def ccReset(self):
        EthHandler.attemptEthSend(b'<r' + b'>')

    @Slot()
    def ccAuto(self):
        EthHandler.cmd_in_auto = 1
        EthHandler.attemptEthSend(b'<a' + chr(EthHandler.cmd_in_auto).encode() + b'>')
        
    @Slot()
    def ccManual(self):
        EthHandler.cmd_in_auto = 0
        EthHandler.attemptEthSend(b'<a' + chr(EthHandler.cmd_in_auto).encode() + b'>')

    @Slot()
    def ccFwdButton(self):
        EthHandler.attemptEthSend(b'<b' + chr(2).encode() + b'>')

    @Slot()
    def ccRevButton(self):
        EthHandler.attemptEthSend(b'<b' + chr(1).encode() + b'>')

    @Slot()
    def ccStopButton(self):
        EthHandler.attemptEthSend(b'<b' + chr(0).encode() + b'>')

    @Slot(result=bool)
    def getAutoState(self):
         return EthHandler.cmd_in_auto

    @Slot(result=bool)
    def ethReset(self):
         if EthHandler.eth_fault:
            EthHandler.sock = EthHandler.attemptEthConnect()
            if EthHandler.sock is not None:
                 return True
            else:
                 EthHandler.eth_fault = True
                 return False

    # close
    @Slot()
    def closeWindow(self):
        EthHandler.sock.close()
        sys.exit()