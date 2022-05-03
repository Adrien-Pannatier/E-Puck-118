import numpy as np
from matplotlib import pyplot as plt
from matplotlib.widgets import Slider, Button
plt.rcParams["figure.figsize"] = [7.50, 3.50]
plt.rcParams["figure.autolayout"] = True
fig, ax = plt.subplots()
map_matrix = np.zeros((3,3))
map_matrix[1,1] = 4
image = map_matrix.reshape(3, 3)
img = ax.imshow(image)
axcolor = 'yellow'
ax_slider = plt.axes([0.20, 0.01, 0.65, 0.03], facecolor=axcolor)
slider = Slider(ax_slider, 'Slide->', 0.1, 30.0, valinit=2)
clearAx             = plt.axes([0.7,0.025,0.1,0.04])
clearbut                = Button(clearAx, 'clear', color='red',hovercolor = '0.975')
def update(val):
    map_matrix[0,0] = 10
    ax.imshow(y.reshape(3,3))

slider.on_changed(update)
clearbut.on_clicked(update)
plt.show()