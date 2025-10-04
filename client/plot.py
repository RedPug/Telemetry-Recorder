import matplotlib.pyplot as plt
from matplotlib.widgets import Button, CheckButtons
import sys
from message_handler import send_message, DataPacket

fig = plt.figure("Telemetry Client", figsize=(8, 6))

axes = fig.add_axes((0.2, 0.1, 0.75, 0.8))

def updatePlot():
    axes.clear()
    data: DataPacket = DataPacket(
        0,0,0,0,0,0
    )
    # with data_lock:
    t = data.timestamp
    ax = data.acc_x
    ay = data.acc_y
    az = data.acc_z

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
start_button.on_clicked(lambda event: send_message('start'))

ax_btn = fig.add_axes((0.05, 0.5, 0.1, 0.05))
end_button = Button(ax_btn, "Stop")
end_button.on_clicked(lambda event: send_message('stop'))


def begin():
    updatePlot()

    plt.show(block = False)

def handle_close(event):
    print("Plot window closed. Exiting program.")
    sys.exit(0)

fig.canvas.mpl_connect('close_event', handle_close)


def onFrame():
    updatePlot()
    plt.pause(0.2)