import socket
import time
import threading
import os
from announce import announce
import sys


# Constants
client_socket = SERVER_IP = SERVER_PORT = REQUEST_MESSAGE = None

CONFIG_FILE = os.path.join(os.path.dirname(__file__), "../config/config.ini")  

# Function to handle commands from Qt
def handle_commands():
    while True:
        try:
            command = sys.stdin.readline().strip()  # Read input from Qt
            if command == "disconnect":
                client_socket.close()
                sys.exit(0)
            else:
                pass
                #print(f"Unknown command: {command}", flush=True)
        except Exception as e:
            pass
            #print(f"Error: {e}", flush=True)

# Function to set up the server connection
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

def main():
    message = connect()

    announce_thread = threading.Thread(target=announce, args=(client_socket, message))
    announce_thread.start()

if __name__ == "__main__":
    setup()

    threading.Thread(target=main, daemon=True).start()
    handle_commands()
    
