import socket

def kick_all(client_threads, client_sockets, remove_client):
    for addr in client_sockets:
        print(f"Kicking {addr}")
        try:
            client_sockets[addr].shutdown(socket.SHUT_RDWR)  # Prevent further sending/receiving
            client_sockets[addr].close()
            print(f"Socket closed for {addr}")
        except OSError as e:
            print(f"Error closing socket for {addr}: {e}")
        except Exception as e:
            print(f"Error kicking {addr}: {e}")

        remove_client(addr)

    client_threads.clear()  # Clear all threads
    print("All clients kicked.")


if __name__ == "__main__":
    kick_all()