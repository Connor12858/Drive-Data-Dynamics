import socket
import time
import threading

SERVER_IP = "localhost"  # Replace with the server's IP address
SERVER_PORT = 12345  # Replace with the server's port

def announce(client_socket, message):
    try:
        while True:
            time.sleep(0.5)  # Send every 500 milliseconds
            client_socket.sendall(message.encode('utf-8'))

    except ConnectionRefusedError:
        print("Connection refused. Server might not be running.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if client_socket:
            client_socket.close()
            print("Connection closed.")

if __name__ == "__main__":
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((SERVER_IP, SERVER_PORT))
    print(f"Connected to {SERVER_IP}:{SERVER_PORT}")
    client_socket.sendall("request".encode('utf-8'))
    message = client_socket.recv(1024).decode('utf-8')
    print(message)

    announce_thread = threading.Thread(target=announce, args=(client_socket, message))
    announce_thread.start()