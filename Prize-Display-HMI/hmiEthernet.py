import socket
import sys, os, time

#For testing
#HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
#PORT = 65000  # Port to listen on (non-privileged ports are > 1023)

#for the connection to clearcore server
HOST = '169.254.207.250'    # The remote host
PORT = 8888              # The same port as used by the server

#class to do all the ethernet stuff and hold python side tag bits/vars
class EthHandler():
    #declare all of the tag related bits/vars
    #cmd for hmi side commanded bits
    #everything else for incoming or internal bits
    cmd_motor_speed = 0 #motor speed commanded

    eth_fault = True #ethernet fault bit

    sock = None #socket for connection to clearcore

    cmd_in_auto = True #commanded automatic/manual state
    
    cmd_wait_time = 10

    cmd_running = False #commanded running state

    running = True #incoming running state

    faulted = False #motor/clearcore fault incoming bit

    estopped = False #estop incoming bit

    #function to cycle through possible ethernet connections and attempt
    #to establish a connection
    def attemptEthConnect():
        #for debug
        print ("attempting to connect")
        #get possible res's on set ip and port
        for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            #try to set socke to current res
            try:
                s = socket.socket(af, socktype, proto)
            except OSError as msg:
                #if it does not connect set it to the socket var to none and
                #go to next res
                s = None
                continue
            #try to connect on current res
            try:
                s.connect(sa)
            except OSError as msg:
                #if this fails close the connection, set s to none, 
                #and go to next res
                s.close()
                s = None
                continue
            #exit for loop if you get this far because it is connected or
            #gone through all possible res's
            break
        #return socket s
        return s

    #set golbal sock to s
    sock = attemptEthConnect()

    #if the connection was not successful raise the eth faul bit
    if sock is None:
        print('could not open socket')
        eth_fault = True
        print("current eth = " + str(eth_fault))
    else:
        #otherwise lower the eth fault bit
        eth_fault = False

    #function for sending messages to the clearcore
    def attemptEthSend(msg):
        #if we have a valid connection
        if not EthHandler.eth_fault and (EthHandler.sock is not None):
            #try to send the message
            try:
                EthHandler.sock.send(msg)
                print("eth send complete")
            except:
                #if this fails set the connection to invalid
                EthHandler.eth_fault = True
                if EthHandler.sock is not None:
                    EthHandler.sock.close()
                EthHandler.sock = None
                #return that it was unsuccessful
                return False
            if EthHandler.sock is not None:
                #otherwise return that it was successful
                return True
        else:
            #if we do not have a valid connection raise the eth fault
            #and close the socket
            EthHandler.eth_fault = True
            if EthHandler.sock is not None:
                EthHandler.sock.close()
            EthHandler.sock = None
        
#for debug on initial startup
print(EthHandler.eth_fault)
