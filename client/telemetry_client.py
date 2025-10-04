import threading
import time
import os
import sys

# Change to the client directory to ensure imports work
client_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(client_dir)

import wifi_handler
import message_handler
from message_handler import DataPacket
import plot

# data : list[DataPacket] = []
# data_lock = threading.Lock()

# NUM_POINTS = 50

is_logging = False

def send_command(cmd: str):
    global is_logging
    cmd = cmd.strip()

    if cmd == 'start':
        is_logging = True
        message_handler.send_message('start')
    elif cmd == 'stop':
        is_logging = False
        message_handler.send_message('stop')


def send_heartbeat():
    while True:
        time.sleep(1)
        wifi_handler.verifyClientConnected()

        try:
            message_handler.send_message('HB\n')
        except Exception as e:
            print(f"Error sending heartbeat: {e}")

def main():
    print("Opening app...")
    plot.begin()
    time.sleep(0.5)
    # return
    
    # Start receiver thread
    print("Starting threads...")
    connect_thread = threading.Thread(target=wifi_handler.connect_to_server, daemon=True)
    connect_thread.start()
    
    # recv_thread = threading.Thread(target=message_handler.listen_for_messages, daemon=True)
    # recv_thread.start()

    heart_thread = threading.Thread(target=send_heartbeat, daemon=True)
    heart_thread.start()

    

    # print("Connecting to server...")
    # connect_to_server()
    
    
    while True:
        plot.onFrame()
        # time.sleep(0.2)

    
        

if __name__ == "__main__":
    main()