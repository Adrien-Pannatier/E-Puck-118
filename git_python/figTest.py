import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button, RadioButtons
import serial
import struct
import sys
import signal
import time
from threading import Thread

#Can be converted into a portable package by using the PyInstaller module
# pip install pyinstaller (need to be used with Python3)
# cf. https://pyinstaller.readthedocs.io/en/v3.3.1/usage.html

goodbye = """
          |\      _,,,---,,_
          /,`.-'`'    -.  ;-;;,_
         |,4-  ) )-,_..;\ (  `'-'
 _______'---''(_/--'__`-'\_)______   ______            _______  _
(  ____ \(  ___  )(  ___  )(  __  \ (  ___ \ |\     /|(  ____ \| |
| (    \/| (   ) || (   ) || (  \  )| (   ) )( \   / )| (    \/| |
| |      | |   | || |   | || |   ) || (__/ /  \ (_) / | (__    | |
| | ____ | |   | || |   | || |   | ||  __ (    \   /  |  __)   | |
| | \_  )| |   | || |   | || |   ) || (  \ \    ) (   | (      |_|
| (___) || (___) || (___) || (__/  )| )___) )   | |   | (____/\ _ 
(_______)(_______)(_______)(______/ |______/    \_/   (_______/(_)                                         
"""

goodbye2 = """
                   /\_/\\
                 =( °w° )=
                   )   (  //
                  (__ __)//
 _____                 _ _                _ 
|  __ \               | | |              | |
| |  \/ ___   ___   __| | |__  _   _  ___| |
| | __ / _ \ / _ \ / _` | '_ \| | | |/ _ \ |
| |_\ \ (_) | (_) | (_| | |_) | |_| |  __/_|
 \____/\___/ \___/ \__,_|_.__/ \__, |\___(_)
                                __/ |       
                               |___/        
"""

#number of samples for one line from the camera
n = 640
#maximum value for an uint8
max_value = 255
#dimensions of the map in cm
max_X = 100
max_Y = 100
#creation of the map matrix
map_matrix = np.zeros((max_Y,max_X))
#colormap
colormap = matplotlib.colors.ListedColormap(['white','#c1c0bf', 'black', '#fe9200', '#0d48b1','red']) #in order: void,ground,walls,fire,e-pouck ext, e-pouck int
bounds = [0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5]
perso_norm = matplotlib.colors.BoundaryNorm(bounds, colormap.N)

#WINDOW STUFF
#handler when closing the window
def handle_close(evt):
    #we stop the serial thread
   # reader_thd.stop()
    print(goodbye)

#EPUCK POSITION
#distance moved
dist_moved = 0
#starting point
start_X = 50
start_Y = 90
#position offset of the e-puck (in the bottom middle)
posR_X = start_X
posR_Y = start_Y - dist_moved

#FIRE POSITIONS
#position of right fire from e-puck
if(fire_right):
    posF_X = posR_X + 11
    posF_Y = posR_Y
#position of left fire from e-puck
elif(fire_left):               #one firecheck at the time
    posF_X = posR_X - 11
    posF_Y = posR_Y

#figure config-------------------------------------------------------------------------
#BACKGROUND
#start background
map_matrix[start_Y:(start_Y+7),(start_X-5):(start_X+6)] = 2
#ground from e-puck position
map_matrix[posR_Y:(posR_Y+5),(posR_X-5):(posR_X+6)] = 2

#WALLS
#start walls
map_matrix[start_Y:(start_Y+7),start_X-6] = 3
map_matrix[start_Y:(start_Y+7),start_X+6] = 3
map_matrix[start_Y+6,(start_X-6):(start_X+7)] = 3

#HOLES
#from e-puck position
#if right hole
if(hole_right):
#background
    map_matrix[(posR_Y-10):(posR_Y+1),(posR_X+6):(posR_X+17)] = 2
#right hole walls
    map_matrix[(posR_Y-11),(posR_X+6):(posR_X+17)] = 3
    map_matrix[(posR_Y+1),(posR_X+6):(posR_X+17)] = 3
    map_matrix[(posR_Y-10):(posR_Y+1),(posR_X+16)] = 3
#if no right hole
else:
#normal wall
    map_matrix[posR_Y,posR_X+6] = 3
    
#if left hole
if(hole_right):
#background
    map_matrix[(posR_Y-10):(posR_Y+1),(posR_X-16):(posR_X-5)] = 2
#walls
    map_matrix[(posR_Y-11),(posR_X-16):(posR_X-5)] = 3
    map_matrix[(posR_Y+1),(posR_X-16):(posR_X-5)] = 3
    map_matrix[(posR_Y-10):(posR_Y+1),(posR_X-16)] = 3
#if no left hole
else:
#normal wall
    map_matrix[posR_Y,posR_X-6] = 3


#ROBOT MODEL
#plot a circle of diameter 7 at robot position
map_matrix[(posR_Y-3),(posR_X-1):(posR_X+2)] = 5
map_matrix[(posR_Y-2),(posR_X-2)] = 5
map_matrix[(posR_Y-2),(posR_X+2)] = 5
map_matrix[(posR_Y-1):(posR_Y+2),(posR_X-3)] = 5
map_matrix[(posR_Y-1):(posR_Y+2),(posR_X+3)] = 5
map_matrix[(posR_Y+2),(posR_X-2)] = 5
map_matrix[(posR_Y+2),(posR_X+2)] = 5
map_matrix[(posR_Y+3),(posR_X-1):(posR_X+2)] = 5
map_matrix[(posR_Y-2):(posR_Y+3),(posR_X-1):(posR_X+2)] = 6
map_matrix[(posR_Y-1):(posR_Y+2),(posR_X-2):(posR_X+3)] = 6

#FIRE MODEL
if(fire_left or fire_right):
    map_matrix[(posF_Y-1):(posF_Y+2),(posF_X-1):(posF_X+2)] = 4
    
def add_T_cross_up():
    map_matrix[(posR_Y+6),(posR_X-12):posR_X+13] = 3
    map_matrix[(posR_Y-6),(posR_X-12):(posR_X-5)] = 3
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+13)] = 3
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X-6)] = 3
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X+6)] = 3
    
def clearmap():
    map_matrix[0:max_Y,0:max_X] = 0

#figure config-------------------------------------------------------------------------------------------------------------------------
fig, ax = plt.subplots(num=None, figsize=(10, 8), dpi=80)
fig.canvas.set_window_title('Map of the house')
plt.subplots_adjust(left=0.1, bottom=0.25)
fig.canvas.mpl_connect('close_event', handle_close) #to detect when the window is closed and if we do a ctrl-c

#cam graph config with initial plot
#graph_cam = plt.subplot(111)
#graph_cam.set_ylim([0, max_value])
map_reshape = map_matrix.reshape(max_X, max_Y)#uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
map_plot = plt.imshow(map_reshape, cmap = colormap, norm = perso_norm)
#plt.plot(np.arange(0,n,1), np.linspace(max_value, max_value, n),lw=1, color='red')

#positions of the buttons, sliders and radio buttons
colorAx             = 'lightgoldenrodyellow'
receiveAx           = plt.axes([0.4, 0.025, 0.1, 0.04])
stopAx              = plt.axes([0.5, 0.025, 0.1, 0.04])
testAx              = plt.axes([0.6,0.025,0.1,0.04])
clearAx             = plt.axes([0.7,0.025,0.1,0.04])

#config of the buttons, sliders and radio buttons
receiveButton           = Button(receiveAx, 'Start reading', color=colorAx, hovercolor='0.975')
stop                    = Button(stopAx, 'Stop reading', color=colorAx, hovercolor='0.975')
testBut                 = Button(testAx, 'salut', color=colorAx, hovercolor = '0.975')
clearbut                = Button(clearAx, 'clear', color='red',hovercolor = '0.975')

def recievefunc():
    print("recieve button")
def stopfunc():
    print("stop button")
    
class Test:
    def testfunc(self, event):
        print("salut")
    def clear(self, event):
        clearmap()
        
def clear(val):
    clearmap()
    add_T_cross_up()
    print("tprint")
    ax.imshow(map_matrix.reshape(max_Y,max_X), cmap = colormap, norm = perso_norm)
    fig.canvas.draw_idle()
    #map_plot.draw()
        
testi = Test()
        
#callback config of the buttons, sliders and radio buttons
receiveButton.on_clicked(stopfunc())
stop.on_clicked(stopfunc())
testBut.on_clicked(testi.testfunc)
clearbut.on_clicked(clear)

#starts the matplotlib main
plt.show()