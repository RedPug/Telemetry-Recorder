
import sys
import threading
import csv
import datetime
import time
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, CheckButtons
import os
from data import DataPacket

data : list[DataPacket] = []
data_lock = threading.Lock()

NUM_POINTS = 50

is_logging = False

def send_command(cmd: str):

    global is_logging

    if not cmd.endswith("\n"):
        cmd += "\n"
    try:
        client_sock.sendall(cmd.encode("utf-8"))
    except Exception as e:
        print(f"Error sending command: {e}")
    print(f"Sent command: {cmd.strip()}")

    if(cmd.strip() == 'start'):
        is_logging = True
    elif(cmd.strip() == 'stop'):
        is_logging = False

fig = plt.figure("Telemetry Client", figsize=(8, 6))

axes = fig.add_axes((0.2, 0.1, 0.75, 0.8))

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

ax_btns = fig.add_axes((0.05, 0.7, 0.1, 0.2))
btn = CheckButtons(ax_btns, ['ax', 'ay', 'az'])
btn.on_clicked(lambda _: updatePlot())

ax_btn = fig.add_axes((0.05, 0.6, 0.1, 0.05))
start_button = Button(ax_btn, 'Start')
start_button.on_clicked(lambda event: send_command('start'))

ax_btn = fig.add_axes((0.05, 0.5, 0.1, 0.05))
end_button = Button(ax_btn, "Stop")
end_button.on_clicked(lambda event: send_command('stop'))

def open_app():
    updatePlot()

    plt.show(block = False)

def handle_close(event):
    print("Plot window closed. Exiting program.")
    sys.exit(0)

fig.canvas.mpl_connect('close_event', handle_close)

def send_heartbeat():
    while True:
        time.sleep(1)
        verifyClientConnected()

        try:
            client_sock.sendall(b'HB\n')
        except Exception as e:
            print(f"Error sending heartbeat: {e}")

def main():
    print("Opening app...")
    open_app()
    time.sleep(0.5)
    # return
    
    # Start receiver thread
    print("Starting threads...")
    recv_thread = threading.Thread(target=receive_telemetry, daemon=True)
    recv_thread.start()

    heart_thread = threading.Thread(target=send_heartbeat, daemon=True)
    heart_thread.start()

    connect_thread = threading.Thread(target=connect_to_server, daemon=True)
    connect_thread.start()

    # print("Connecting to server...")
    # connect_to_server()
    
    
    while True:
        updatePlot()
        plt.pause(0.2)

    
        

if __name__ == "__main__":
    main()