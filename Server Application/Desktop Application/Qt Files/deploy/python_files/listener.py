import socket
import threading
import time
import traceback
import random
import string
import os
import kick_all
import sys

# Constants
INACTIVITY_TIMEOUT = HOST = PORT = REQUEST_MESSAGE = None
ANNOUNCE_MESSAGE = ''.join(random.choices(string.ascii_letters + string.digits, k=24))
CONNECTIONS_FILE = os.path.join(os.path.dirname(__file__), "../config/connections.ini")
CONFIG_FILE = os.path.join(os.path.dirname(__file__), "../config/config.ini")  
LOG_FILE = os.path.join(os.path.dirname(__file__), "log.txt")

# Global dictionary to store client information (address: last_activity_time)
client_activity = {} # Store client activity for inactivity check
client_sockets = {}  # Store client sockets for proper closing
client_threads = {}  # Store client threads for proper closing

def log(message):
    with open(LOG_FILE, 'a') as f:
        f.write(f"{time.ctime()}: {message}\n")

# Function to handle commands from Qt
def handle_commands():
    while True:
        try:
            command = sys.stdin.readline().strip()  # Read input from Qt
            if command == "kick all":
                kick_all.kick_all(client_threads, client_sockets, remove_client)
            else:
                pass
                #print(f"Unknown command: {command}", flush=True)
        except Exception as e:
            pass
            #print(f"Error: {e}", flush=True)

# Function to load configuration from file
def setup():
    global INACTIVITY_TIMEOUT, HOST, PORT, REQUEST_MESSAGE

    # Load configuration from file
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, 'r') as f:
            lines = f.readlines()
            for line in lines:
                if "=" in line:
                    key, value = line.strip().split('=')
                    if key == 'INACTIVITY_TIMEOUT':
                        INACTIVITY_TIMEOUT = int(value)
                    elif key == 'HOST':
                        HOST = value
                    elif key == 'PORT':
                        PORT = int(value)
                    elif key == 'REQUEST_MESSAGE':
                        REQUEST_MESSAGE = value
    # Set default values if not loaded
    else:
        INACTIVITY_TIMEOUT = 1
        HOST = 'localhost'
        PORT = 12345
        REQUEST_MESSAGE = "REQUEST"

# Function to handle incoming connections
def handle_client(client_socket, address):
    log(f"Connection established with {address}")
    client_activity[address] = time.time()  # Record initial activity
    client_sockets[address] = client_socket  # Store the socket

    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break

            message = data.decode('utf-8')
            if message == ANNOUNCE_MESSAGE:
                client_activity[address] = time.time()  # Update activity time
            elif message == REQUEST_MESSAGE:
                client_socket.sendall(ANNOUNCE_MESSAGE.encode('utf-8'))
                log(f"Announcement message requested from {address}")
            else:
                log(f"Received from {address}: {message}")

    except ConnectionResetError:
        log(f"Connection reset by {address}")
    except Exception as e:
        log(f"Error with {address}: {e}")
    finally:
        log(f"Connection closed with {address}")
        del client_sockets[address]  # Remove the socket
        remove_client(address)  # Clean up client from activity list
        client_socket.close() # Close the socket

# Function to remove inactive clients
def remove_client(address):
    if address in client_activity:
        del client_activity[address]
        log(f"Removed inactive client: {address}")

# Function to check for inactive clients
def check_inactive_clients():
    while True:
        time.sleep(1)  # Check every 1 second
        current_time = time.time()
        # Find inactive clients
        inactive_clients = [
            addr for addr, last_activity in client_activity.items()
            if current_time - last_activity > INACTIVITY_TIMEOUT
        ]

        for addr in inactive_clients:
            if addr in client_sockets:
                try:
                    client_sockets[addr].shutdown(socket.SHUT_RDWR)  # Prevent further sending/receiving
                    client_sockets[addr].close() # Close the socket
                except OSError as e:
                    pass
                    #print(f"Error closing socket for {addr}: {e}")
                del client_sockets[addr]  # Remove the socket
            remove_client(addr)

        # Write active clients to file
        with open(CONNECTIONS_FILE, 'w') as f:
            for client in client_sockets:
                f.write(f"{client}\n")

# Main function to open a port and listen for connections
def main():
    try:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind((HOST, PORT))
        server.listen(5)
    except Exception as e:
        #print(f"Error creating or binding socket: {e}")
        traceback.log_exc()
        return

    # Start a thread to check for inactive clients
    threading.Thread(target=check_inactive_clients, daemon=True).start()

    while True:
        try:
            client_socket, addr = server.accept()
            client_handler = threading.Thread(
                target=handle_client, args=(client_socket, addr), daemon=True
            )
            client_threads[addr] = client_handler
            client_handler.start()
        except Exception as e:
            #print(f"Error accepting connection: {e}")
            traceback.log_exc()

if __name__ == "__main__":
    setup()

    threading.Thread(target=main, daemon=True).start()
    handle_commands()