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
            # print(f"Connection failed: {e}")
            print("Connection failed...")
            time.sleep(1)

# def get_wifi_strength():
#     result = subprocess.run(['netsh', 'wlan', 'show', 'interfaces'], capture_output=True, text=True)
#     for line in result.stdout.splitlines():
#         if "Signal" in line:
#             # Example line: "    Signal                 : 80%"
#             strength = int(line.split(":")[1].strip().replace('%', ''))
#             return strength
#     return None

def is_client_connected():
    try:
        client_sock.send(b'')  # Sending empty bytes is safe and triggers error if disconnected
        return True
    except (OSError, ConnectionResetError, BrokenPipeError):
        return False
    
def verifyClientConnected():
    if not is_client_connected():
        connect_to_server()