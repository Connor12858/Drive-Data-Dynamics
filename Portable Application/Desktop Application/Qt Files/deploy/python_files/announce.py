import time

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
