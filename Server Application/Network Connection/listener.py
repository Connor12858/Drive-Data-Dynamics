import socket
import threading
import time
import traceback
import random
import string
import os

# Constants
HOST = "localhost"
PORT = 12345
ANNOUNCE_MESSAGE = ''.join(random.choices(string.ascii_letters + string.digits, k=24))
INACTIVITY_TIMEOUT = 3
REQUEST_MESSAGE = "request"
CONNECTIONS_FILE = "connections.list"

# Global dictionary to store client information (address: last_activity_time)
client_activity = {}
client_sockets = {}  # Store client sockets for proper closing

# Function to handle incoming connections
def handle_client(client_socket, address):
    print(f"Connection established with {address}")
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
                print(f"Received announcement from {address}")
            elif message == REQUEST_MESSAGE:
                client_socket.sendall(ANNOUNCE_MESSAGE.encode('utf-8'))
                print(f"Announcement message requested from {address}")
            else:
                print(f"Received from {address}: {message}")

    except ConnectionResetError:
        print(f"Connection reset by {address}")
    except Exception as e:
        print(f"Error with {address}: {e}")
    finally:
        print(f"Connection closed with {address}")
        del client_sockets[address]  # Remove the socket
        remove_client(address)  # Clean up client from activity list
        client_socket.close()

def remove_client(address):
    if address in client_activity:
        del client_activity[address]
        print(f"Removed inactive client: {address}")

# Function to check for inactive clients
def check_inactive_clients():
    while True:
        time.sleep(1)  # Check every 1 second
        current_time = time.time()
        inactive_clients = [
            addr for addr, last_activity in client_activity.items()
            if current_time - last_activity > INACTIVITY_TIMEOUT
        ]

        for addr in inactive_clients:
            print(f"Client {addr} timed out.")
            if addr in client_sockets:
                try:
                    client_sockets[addr].shutdown(socket.SHUT_RDWR)  # Prevent further sending/receiving
                    client_sockets[addr].close()
                except OSError as e:
                    print(f"Error closing socket for {addr}: {e}")
                del client_sockets[addr]  # Remove the socket
            remove_client(addr)

        with open("D:\\Drive-Data-Dynamics\\Server Application\\Network Connection\\" + CONNECTIONS_FILE, 'w') as f:
            for client in client_sockets:
                f.write(f"{client}\n")

# Main function to open a port and listen for connections
def main():
    try:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind((HOST, PORT))
        server.listen(5)
        print(f"Server listening on {HOST}:{PORT}")
    except Exception as e:
        print(f"Error creating or binding socket: {e}")
        traceback.print_exc()
        return

    # Start a thread to check for inactive clients
    threading.Thread(target=check_inactive_clients, daemon=True).start()

    while True:
        try:
            client_socket, addr = server.accept()
            client_handler = threading.Thread(
                target=handle_client, args=(client_socket, addr)
            )
            client_handler.start()
        except Exception as e:
            print(f"Error accepting connection: {e}")
            traceback.print_exc()

if __name__ == "__main__":
    main()