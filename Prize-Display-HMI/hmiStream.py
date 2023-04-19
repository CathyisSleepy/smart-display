#import libraries we need
import sys
import os, time
import socket
from hmiEthernet import EthHandler

#check which Qt library we are using and import the right one
try:
        from PySide2.QtCore import *
except ImportError:
        from PyQt5.QtCore import *
if 'PyQt5' in sys.modules:
        from PyQt5.QtCore import pyqtSignal as Signal, pyqtSlot as Slot
else:
        from PySide2.QtCore import Signal, Slot


#Qthread class to stream variables to the QML
class Streaming(QThread):
    #set signals for all of the things we need to keep track of
    #estop, fault, and run are all tag inputs from the clearcore
    #EthSignal and MotorSignals are transmissions of ethfault and cmd_motor_speed respectively
    EstopSignal = Signal(bool)
    FaultSignal = Signal(bool)
    RunSignal = Signal(bool)
    EthSignal = Signal(bool)
    MotorSignal = Signal(int)

    #check that there is no connection fault and reconnect if so
    if EthHandler.eth_fault:
        EthHandler.sock = EthHandler.attemptEthConnect()
        #if connection was succesfull then turn off fault
        if EthHandler.sock is not None:
            EthHandler.eth_fault = False
        #else it is still faulted
        else:
            EthHandler.eth_fault = True

    #initiallize the thread
    def __init__(self):
        super().__init__()

    #Thread run loop
    def run(self):
        #initiallize empty container for incoming data
        data = None
        #start the tag update timer
        tag_timer_start = round(time.time() *1000)

        #repeating loop to update varibles over tags to and from the clearcore
        while True:
            #get current time on the tag update timer
            tag_timer_curr = round(time.time() * 1000) - tag_timer_start
                
            #try to update tags if it has been longer than 200 miliseconds
            if tag_timer_curr >= 1000:
                #send commanded in auto <a[val]>, motorspeed <m[val]>, and run state <l[val]>
                EthHandler.attemptEthSend(b'<a' + chr(EthHandler.cmd_in_auto).encode() + b'>')
                EthHandler.attemptEthSend(b'<m' + chr(EthHandler.cmd_motor_speed).encode() + b'>')
                EthHandler.attemptEthSend(b'<l' + chr(EthHandler.cmd_running).encode() + b'>')
                #restart tag timer
                tag_timer_start = round(time.time() * 1000)
                print("tags updated")

            #make sure we have a valid connection
            if (EthHandler.sock is not None) and not EthHandler.eth_fault:
                try:
                    #look for incoming data    
                    data = EthHandler.sock.recv(86).decode()
                except:
                    #there was no data in current recv
                    print("no data")

                #if there is incoming data
                if data:
                    #slit the data into an array using the "|" as a divider
                    datalist = data.split("|")
                    #take each item in the array
                    for msg in datalist:
                        #create a smaller list dividing items by ","
                        #this will get us our usable tags with the identifier
                        #in tag[0] and the attacted value in tag[1]
                        tag = msg.split(",")
                        #if the identifier is "estop" (estop signal from clearcore)
                        if tag[0] == "estop":
                            #try to cast it to an int to make sure it is usable
                            try:
                                estop = int(tag[1])
                            except:
                                #if it fails go to the next incoming tag
                                print("failed to cast estop to int")
                                continue

                            #if the input is valid then update EthHandler.estopped to be emited later
                            if estop == 1 or estop == 0:
                                EthHandler.estopped = estop
                                print("estop: " + str(estop))
                            else:
                                #if it is not valid go to the next tag
                                print("estop input invalid")
                                continue
                        
                        #else if the identifier is "fault" (motor fault signal from clearcore)
                        elif tag[0] == "fault":
                            #try to cast the value to an int
                            try:
                                fault = int(tag[1])
                            except:
                                #if this cast fails go to next tag
                                print("failed to cast fault to int")
                                print("fault invalid input: " + str(tag[1]))
                                continue
                            
                            #if the value is valid then update eth_fault to be emitted to QML
                            if fault == 1 or fault == 0:
                                EthHandler.eth_fault = fault
                                print("fault: " + str(fault))
                            else:
                                #if the value is invalid then go to next tag
                                print("fault input invalid")
                                continue

                        #if tag is "run" or "running" (the signal that the clearcore main loop is active)
                        elif tag[0] == "run" or tag[0] == "running":
                            #try to cast to int
                            try: 
                                run = int(tag[1])
                            except:
                                #if cast fails than go to next tag
                                print("failed to cast run to int")
                                print("run invalid input: " + str(tag[1]))
                                continue

                            # if the input is valid then update the running bit to be emitted  
                            if run == 1 or run == 0:
                                EthHandler.running = run
                                print("run: " + str(run))
                            else:
                                #if it is invalid go to the next tag
                                print("run input invalid: " + str(run))
                                continue
                            
                        #this was removed due to inconsistancy of feedback from motor
                        '''elif tag[0] == "speed":
                            try: 
                                speed = int(tag[1])
                            except:
                                print("failed to cast speed to int")
                                print("run invalid input: " + str(tag[1]))
                                continue
                                
                            if speed >= -255 and speed <= 255:
                                print("speed: " + str(speed))
                            else:
                                print("speed input invalid")
                                continue'''
            #if we do not have a good connection attempt to reconnect and update the
            #ethernet fault
            else:
                print("ethfaulted")
                EthHandler.sock = EthHandler.attemptEthConnect()
                if EthHandler.sock is not None:
                    EthHandler.eth_fault = False
                else:
                    EthHandler.eth_fault = True
            
            #emit all of the signals to the QML
            self.MotorSignal.emit(int(EthHandler.cmd_motor_speed))
            self.EthSignal.emit(bool(EthHandler.eth_fault))
            self.EstopSignal.emit(bool(EthHandler.estopped))
            self.FaultSignal.emit(bool(EthHandler.faulted))
            self.RunSignal.emit(bool(EthHandler.running))
            #for debug
            print("stream eth: " + str(bool(EthHandler.eth_fault)) + " time: " + str(time.time()))
