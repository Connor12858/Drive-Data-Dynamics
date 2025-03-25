import socket
import time
import threading
import os
from announce import announce

# Constants
client_socket = SERVER_IP = SERVER_PORT = REQUEST_MESSAGE = None

CONFIG_FILE = "config.ini"   


def setup():
    global SERVER_IP, SERVER_PORT, REQUEST_MESSAGE

    # Load configuration from file
    config_path = os.path.join(os.path.dirname(__file__), CONFIG_FILE)
    if os.path.exists(config_path):
        with open(config_path, 'r') as f:
            lines = f.readlines()
            for line in lines:
                key, value = line.strip().split('=')
                if key == 'HOST':
                    SERVER_IP = value
                elif key == 'PORT':
                    SERVER_PORT = int(value)
                elif key == 'REQUEST_MESSAGE':
                    REQUEST_MESSAGE = value
    else:
        SERVER_IP = 'localhost'
        SERVER_PORT = 12345
        REQUEST_MESSAGE = "REQUEST"


def connect():
    global client_socket

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((SERVER_IP, SERVER_PORT))
    client_socket.sendall(REQUEST_MESSAGE.encode('utf-8'))
    return client_socket.recv(1024).decode('utf-8')

if __name__ == "__main__":
    setup()

    message = connect()

    announce_thread = threading.Thread(target=announce, args=(client_socket, message))
    announce_thread.start()