import socket
import sys, os

#For testing
#HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
#PORT = 65000  # Port to listen on (non-privileged ports are > 1023)

HOST = '169.254.207.250'    # The remote host
PORT = 8888              # The same port as used by the server

class EthHandler():

    motor_speed = 0
    eth_fault = True
    sock = None
    in_auto = True
    running = 0
    faulted = 0

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

    def attemptEthSend(msg):
        if not EthHandler.eth_fault:
            try:
                EthHandler.sock.send(msg)
            except:
                EthHandler.eth_fault = True
                EthHandler.sock.close()
                EthHandler.sock = None
                return False
            if EthHandler.sock is not None:
                return True

print(EthHandler.eth_fault)