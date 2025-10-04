import datetime
import os
import csv
from data_handler import DataPacket

writer:csv.writer = None

def create_log_file():
    global writer

    print("making new log file...")

    filename = datetime.datetime.now().strftime("telemetry_%Y_%m_%d_at_%H_%M_%S.csv")

    os.makedirs(os.path.join(os.path.dirname(__file__), 'output'), exist_ok=True) # create output directory if it doesn't exist
    csv_file = open(os.path.join(os.path.dirname(__file__), f'output/{filename}'), 'w', newline='\n') # output log file
    writer = csv.writer(csv_file)

    writer.writerow(["Time", "AX", "AY", "AZ", "GPSX", "GPSY"])
    csv_file.flush()

def close_log_file(csv_file):
    csv_file.close()

def log_data(packet: DataPacket):
    global writer
    if(writer == None): create_log_file()
    print("logging...")

    writer.writerow([
        packet.timestamp,
        f"{packet.acc_x:0.5f}",
        f"{packet.acc_y:0.5f}",
        f"{packet.acc_z:0.5f}",
        f"{packet.gps_long:0.5f}",
        f"{packet.gps_lat:0.5f}"
        ])