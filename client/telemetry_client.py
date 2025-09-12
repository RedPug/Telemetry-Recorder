from operator import is_
import socket
import struct
import threading
import subprocess
import csv
import datetime
import time
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, CheckButtons

ESP32_IP = "192.168.4.1"
TCP_PORT = 4210

client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

filename = datetime.datetime.now().strftime("telemetry_%Y_%m_%d_at_%H_%M_%S.csv")
csv_file = open(f'client/output/{filename}', 'w', newline='\n')
writer = csv.writer(csv_file)

writer.writerow(["Time", "AX", "AY", "AZ"])
csv_file.flush()

class TelemetryData:
    def __init__(self):
        self.t = []
        self.ax = []
        self.ay = []
        self.az = []

data = TelemetryData()
data_lock = threading.Lock()

NUM_POINTS = 50

def connect_to_server():
    global client_sock
    client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    while True:
        try:
            client_sock.connect((ESP32_IP, TCP_PORT))
            print("Connected!")
            break
        except Exception as _:
            # print(f"Connection failed: {e}")
            print("Connection failed...")
            time.sleep(5)

def get_wifi_strength():
    result = subprocess.run(['netsh', 'wlan', 'show', 'interfaces'], capture_output=True, text=True)
    for line in result.stdout.splitlines():
        if "Signal" in line:
            # Example line: "    Signal                 : 80%"
            strength = int(line.split(":")[1].strip().replace('%', ''))
            return strength
    return None

def is_client_connected():
    try:
        client_sock.getpeername()
        return True
    except socket.error:
        return False
    
def verifyClientConnected():
    if not is_client_connected():
        connect_to_server()

def receive_telemetry():
    while True:
        verifyClientConnected()

        try:
            #u8, u32, s16, s16, s16
            format = "<BIhhh"  # Little-endian
            num_bytes = struct.calcsize(format)

            buffer = b''

            while len(buffer) < num_bytes:
                chunk = client_sock.recv(num_bytes - len(buffer))
                if not chunk:
                    raise ConnectionError("Disconnected...")
                

                buffer += chunk

            if len(buffer) == num_bytes:
                (_, t, ax, ay, az) = struct.unpack(format, buffer)
                a_to_g = 1.0/16384.0
                ax *= a_to_g
                ay *= a_to_g
                az *= a_to_g

                
                # print(f"time:{t}, ax:{ax:+0.3f}, ay:{ay:+0.3f}, az:{az:+0.3f}")
                writer.writerow([t, f"{ax:0.5f}", f"{ay:0.5f}", f"{az:0.5f}"])
                csv_file.flush()

                with data_lock:
                    data.t.append(t)
                    data.t = data.t[-NUM_POINTS:]
                    data.ax.append(ax)
                    data.ax = data.ax[-NUM_POINTS:]
                    data.ay.append(ay)
                    data.ay = data.ay[-NUM_POINTS:]
                    data.az.append(az)
                    data.az = data.az[-NUM_POINTS:]
            else:
                print("invalid buffer size")
                break
        except Exception as e:
            print(f"Error receiving data: {e}")
            # Attempt to reconnect if disconnected
            if isinstance(e, ConnectionError):
                print("Attempting to reconnect...")
                connect_to_server()

is_logging = False

def send_command(cmd: str):
    global is_logging
    if not cmd.endswith("\n"):
        cmd += "\n"
    client_sock.sendall(cmd.encode("utf-8"))
    print(f"Sent command: {cmd.strip()}")

    if(cmd.strip() == 'start'):
        is_logging = True
    elif(cmd.strip() == 'stop'):
        is_logging = False

fig = plt.figure("Telemetry Client", figsize=(8, 6))

axes = fig.add_axes([0.2, 0.1, 0.75, 0.8])

def updatePlot():
    axes.clear()

    with data_lock:
        t = data.t
        ax = data.ax
        ay = data.ay
        az = data.az

        btns = btn.get_status()
        if(btns[0]):
            axes.plot(t, ax, 'ro-')
        if(btns[1]):
            axes.plot(t, ay, 'go-')
        if(btns[2]):
            axes.plot(t, az, 'bo-')

    plt.draw()

ax_btns = fig.add_axes([0.05, 0.7, 0.1, 0.2])
btn = CheckButtons(ax_btns, ['ax', 'ay', 'az'])
btn.on_clicked(lambda _: updatePlot())

ax_btn = fig.add_axes([0.05, 0.6, 0.1, 0.05])
start_button = Button(ax_btn, 'Start')
start_button.on_clicked(lambda event: send_command('start'))

ax_btn = fig.add_axes([0.05, 0.5, 0.1, 0.05])
end_button = Button(ax_btn, "Stop")
end_button.on_clicked(lambda event: send_command('stop'))

def open_app():

    updatePlot()

    plt.show(block = False)

def main():
    open_app()
    # return
    
    connect_to_server()
    # Start receiver thread
    recv_thread = threading.Thread(target=receive_telemetry, daemon=True)
    recv_thread.start()

    # send_command('start')

    while True:
        if is_logging:
            updatePlot()
        plt.pause(0.2)

if __name__ == "__main__":
    main()