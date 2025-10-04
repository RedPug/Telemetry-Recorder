from dataclasses import dataclass
import threading
import message_handler

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

def get_all_data() -> list[DataPacket]:
    with data_lock:
        return all_data.copy()


def handle_packet(data: dict):
    global all_data

    # print("got a packet!")

    packet = DataPacket(
        timestamp = data.get('ts', None),
        acc_x =     data.get('ax', None),
        acc_y =     data.get('ay', None),
        acc_z =     data.get('az', None),
        gps_long =  data.get('lon', None),
        gps_lat =   data.get('lat', None)
    )
    
    import file_manager  # Import here to avoid circular import
    file_manager.log_data(packet)
    
    with data_lock:
        all_data.append(packet)

message_handler.add_message_callback(handle_packet)