import socket
import time


ESP32_IP = "192.168.4.1"
TCP_PORT = 4210

client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def connect_to_server():
    global client_sock

    #close existing socket if open
    try:
        client_sock.close()
    except Exception:
        pass

    client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    while not is_client_connected():
        try:
            client_sock.connect((ESP32_IP, TCP_PORT))
            print("Connected!")
            break
        except Exception as _:
            print("Connection failed...")
            time.sleep(1)

def is_client_connected():
    global client_sock
    try:
        client_sock.sendall(b'')  # Sending empty bytes is safe and triggers error if disconnected
        return True
    except (OSError, ConnectionResetError, BrokenPipeError):
        return False
    
def send_data(data: bytes):
    client_sock.sendall(data)

def receive_data(num_bytes: int) -> bytes:
    return client_sock.recv(num_bytes)
    
def verifyClientConnected():
    if not is_client_connected():
        connect_to_server()

