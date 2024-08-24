#!/usr/bin/python3
import socket
import time
import sys

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0" , int(sys.argv[1])))

while 1:
    data, addr = sock.recvfrom(65535)  # Buffer size 65535 bytes

    timestamp = time.time()
    timestamp = f"{timestamp:.6f}".ljust(16, '0')

    octets = ' '.join(f"{byte:02x}" for byte in data)

    print(f"{timestamp} {octets}")
