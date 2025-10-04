import matplotlib.pyplot as plt
from matplotlib.widgets import Button, CheckButtons
import sys
from gps import GlobalPosition
from message_handler import send_message

fig = plt.figure("Telemetry Client", figsize=(8, 6))

axes = fig.add_axes((0.2, 0.1, 0.75, 0.8))

def updatePlot():
    axes.clear()
    axes.set_aspect('equal', adjustable="box")
    coords: list[GlobalPosition] = [
        GlobalPosition(33.6413898, -117.8467565),
        GlobalPosition(33.6413898, -117.8467565),
        GlobalPosition(33.6423187, -117.8421217),
        GlobalPosition(33.6427832, -117.836371),
        GlobalPosition(33.6477492, -117.835341),
        GlobalPosition(33.6486244, -117.8388816),
        GlobalPosition(33.651018, -117.8417998),
        GlobalPosition(33.6493925, -117.8434091),
        GlobalPosition(33.6487137, -117.8466278),
        GlobalPosition(33.6464095, -117.8488379),
        GlobalPosition(33.645159, -117.8490096),
        GlobalPosition(33.6433727, -117.8492671),
        GlobalPosition(33.6416042, -117.8483444),
    ]

    center = coords[0]

    x = []
    y = []

    for coord in coords:
        pos = coord.toXY(center)
        x.append(pos[0])
        y.append(pos[1])

    axes.plot(x,y, 'ro-')
    # data: DataPacket = DataPacket(
    #     0,0,0,0,0,0
    # )
    # # with data_lock:
    # t = data.timestamp
    # ax = data.acc_x
    # ay = data.acc_y
    # az = data.acc_z

    # btns = btn.get_status()
    # if(btns[0]):
    #     axes.plot(t, ax, 'ro-')
    # if(btns[1]):
    #     axes.plot(t, ay, 'go-')
    # if(btns[2]):
    #     axes.plot(t, az, 'bo-')

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