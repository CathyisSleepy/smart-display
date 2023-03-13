import socket, time

HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        while True:
            data = conn.recv(32)
            if data:
                print(data.decode())
            conn.sendall(b'|||estop,1|run,0|fault,1|sfllsfj,9|')
            time.sleep(5)
            conn.sendall(b'|||estop,0|run,1|')
            conn.sendall(b'|fault,0|sfllsfj,9|estop,run|')
            time.sleep(5)