import socket
import time
import threading
import os
from announce import announce
from file_handler import send_file
import sys
import select


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
            elif command == "file":
                print("File command received", flush=True)
                # Handle file command
                filename = sys.stdin.readline().strip()
                print(f"Filename: {filename}", flush=True)
                client_name = sys.stdin.readline().strip()
                print(f"Client name: {client_name}", flush=True)
                event_name = sys.stdin.readline().strip()
                print(f"Event name: {event_name}", flush=True)
                file_size = int(sys.stdin.readline().strip())
                print(f"File size: {file_size}", flush=True)
                file_data = sys.stdin.buffer.read(file_size)
                print(f"File data length: {len(file_data)}", flush=True)
                send_file(client_socket, filename, client_name, event_name, file_data)
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

    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((SERVER_IP, SERVER_PORT))
        client_socket.sendall(REQUEST_MESSAGE.encode('utf-8'))
        print("CMD-SET-GOOD", flush=True, end='')
        return client_socket.recv(1024).decode('utf-8')
    except Exception as e:
        print(f"Error: {e}")
        os._exit(0)

def main():
    message = connect()

    announce(client_socket, message)

if __name__ == "__main__":
    setup()

    threading.Thread(target=main, daemon=True).start()
    handle_commands()
    
