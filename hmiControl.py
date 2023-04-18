# import libraries
import sys
import os
import socket
import pickle
from hmiEthernet import EthHandler

#check and import if the system uses PySide2 or PyQt5
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

    #update motor speed to clearcore and return the motor speed to the QML script
    @Slot(result=int)
    def motorSpeedGet(self):
        #attempt to retrieve the motor speed from speed.pick and update the commanded motor speed
        try:
            p = open('speed.pick', 'rb')
            EthHandler.cmd_motor_speed = pickle.load(p)
            p.close
        except:
            print("load failed")
        
        #send the tag "<m[value]>" to the clearcore and return the motor speed to the QML
        EthHandler.attemptEthSend(b'<m' + chr(EthHandler.cmd_motor_speed).encode() + b'>')
        return int(EthHandler.cmd_motor_speed)

    #tell clearcore what to set the motor speed to
    @Slot(int)
    def motorSpeedSet(self, val):
        #send the motor speed in a format it can read "<m[value]>"
        EthHandler.attemptEthSend(b'<m' + chr(val).encode() + b'>')

        #attempt to save the value in a file to be accessed later
        try:
            p = open('speed.pick', 'wb')
            pickle.dump(EthHandler.cmd_motor_speed, p)
            p.close
        except:
            print("save failed")

        #set commanded motor speed to incoming value
        EthHandler.cmd_motor_speed = val

    #tell clearcore to stop main loop
    @Slot()
    def ccStart(self):
        #commanded run value set to 1 and send tag "<l1>" to the clearcore
        EthHandler.cmd_running = 1
        EthHandler.attemptEthSend(b'<l' + chr(EthHandler.cmd_running).encode() + b'>')

    #tell clearcore to start main loop
    @Slot()
    def ccStop(self):
        #commanded run value set to 0 and send tag "<l0>" to the clearcore
        EthHandler.cmd_running = 0
        EthHandler.attemptEthSend(b'<l' + chr(EthHandler.cmd_running).encode() + b'>')

    #send reset signal to clearcore and sets was_estopped to false
    @Slot()
    def ccReset(self):
        #send tag <r> to the clearcore
        h = EthHandler.attemptEthSend(b'<r'+b'>')
        if h == True:
            print("sent reset signal")

    #send command to put machine in automatic
    @Slot()
    def ccAuto(self):
        #set commanded in auto to true and send "<a1>" to the clearcore
        EthHandler.cmd_in_auto = 1
        EthHandler.attemptEthSend(b'<a' + chr(EthHandler.cmd_in_auto).encode() + b'>')

    #send command to put machine in manaul    
    @Slot()
    def ccManual(self):
        #set commanded in auto to false and sent "<a0>" to the clearcore over tcp
        EthHandler.cmd_in_auto = 0
        EthHandler.attemptEthSend(b'<a' + chr(EthHandler.cmd_in_auto).encode() + b'>')

    #connected slot to send "<b2>" to the clearcore which is the foward button
    @Slot()
    def ccFwdButton(self):
        EthHandler.attemptEthSend(b'<b' + chr(2).encode() + b'>')

    #connected slot to send "<b1>" to the clearcore which is the reverse button
    @Slot()
    def ccRevButton(self):
        EthHandler.attemptEthSend(b'<b' + chr(1).encode() + b'>')

    #connected slot to send "<b0>" to the clearcore which is the pause/stop button
    @Slot()
    def ccStopButton(self):
        EthHandler.attemptEthSend(b'<b' + chr(0).encode() + b'>')

    #returns the value of commanded in auto
    @Slot(result=bool)
    def getAutoState(self):
         return EthHandler.cmd_in_auto
    
    #returns the value of running
    @Slot(result=bool)
    def getRunState(self):
         return bool(EthHandler.running)
    
    #get the current estop state
    @Slot(result=bool)
    def getEstopState(self):
         return bool(EthHandler.estopped)

    #runs socket connection function in EthHandler and sets the ethernet fault based on the results
    @Slot(result=bool)
    def ethReset(self):
         if EthHandler.eth_fault:
            EthHandler.sock = EthHandler.attemptEthConnect()
            if EthHandler.sock is not None:
                 return True
            else:
                 EthHandler.eth_fault = True
                 return False

    # closes program and exits to desktop
    @Slot()
    def closeWindow(self):
        EthHandler.sock.close()
        sys.exit()
