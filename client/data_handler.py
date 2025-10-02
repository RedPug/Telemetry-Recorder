import json
from wifi_handler import client_sock
import threading
import file_manager
from dataclasses import dataclass

@dataclass
class DataPacket:
    timestamp: float
    acc_x: float
    acc_y: float
    acc_z: float
    gps_long: float
    gps_lat: float

data_lock = threading.Lock()
all_data : list[DataPacket] = []

buffer:str = "" # Buffer to hold partial data

def begin():
    """
    Begin listening for and logging data.
    """
    recv_thread = threading.Thread(target=receive_telemetry, daemon=True)
    recv_thread.start()
    pass

def receive_telemetry():
    while True:
        messages:list[dict] = receive_json_messages()
        for msg in messages:
            if msg['type'] == 'data':
                packet = DataPacket(
                    timestamp = msg['ts'],
                    acc_x =     msg['ax'],
                    acc_y =     msg['ay'],
                    acc_z =     msg['az'],
                    gps_long =  msg['lon'],
                    gps_lat =   msg['lat']
                )
                handle_packet(packet)
            else:
                print(f"Unknown message type: {msg['type']}")
    # while True:
    #     verifyClientConnected()

    #     try:
    #         # integers:
    #         # b = 8, h = 16, i = 32, q = 64
    #         # capital is unsigned, lower is signed.
    #         # float:
    #         # f = 32, d = 64

    #         #time, ax, ay, az, longitude, latitude
    #         format = "<Ifffff"  # Little-endian is default for ESP32
    #         num_bytes = struct.calcsize(format)

    #         buffer = b''
            
    #         while len(buffer) < num_bytes:
    #             try:
    #                 client_sock.setblocking(False) # don't block if no data available
    #                 chunk = client_sock.recv(num_bytes - len(buffer))
    #             except:
    #                 time.sleep(0.2) # wait before trying again
    #                 continue
    #             if not chunk:
    #                 raise ConnectionError("Disconnected...")

    #             buffer += chunk

    #         if len(buffer) == num_bytes:
    #             (t, ax, ay, az, gps_long, gps_lat) = struct.unpack(format, buffer)
    #             packet = DataPacket(t, ax, ay, az, gps_long, gps_lat)
    #             handle_packet(packet)
                
    #         else:
    #             print("invalid buffer size")
    #             break
    #     except Exception as e:
    #         print(f"Error receiving data: {e}")
    #         # Attempt to reconnect if disconnected
    #         if isinstance(e, ConnectionError):
    #             print("Attempting to reconnect...")
    #             connect_to_server()

def receive_json_messages() -> list[dict]:
        """Receive and parse JSON messages, handling partial/multiple messages"""
        messages:list[dict] = []
        
        try:
            # Receive up to 1024 chars from network buffer
            data:str = client_sock.recv(1024).decode('utf-8')
            if not data:
                return messages  # Connection closed
            buffer += data
            # Split by null characters (assuming each JSON is separated by a null character)
            lines:list[str] = buffer.split('\0')
            # Keep the last line in buffer (might be partial)
            buffer = lines[-1]
            
            # Process complete lines
            for line in lines[:-1]:
                line = line.strip()
                if line:  # Skip empty lines
                    try:
                        json_obj = json.loads(line)
                        messages.append(json_obj)
                    except json.JSONDecodeError as e:
                        print(f"Invalid JSON line: {line[:50]}... Error: {e}")
        except socket.timeout:
            print("Socket timeout - no data received")
        except ConnectionResetError:
            print("Connection lost")
            buffer = ""  # Clear buffer on connection loss
        except Exception as e:
            print(f"Error receiving data: {e}")
        
        return messages

def handle_packet(packet: DataPacket):
    global all_data

    file_manager.log_data(packet)
    
    with data_lock:
        all_data.append(packet)