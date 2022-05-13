import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button, RadioButtons
from matplotlib.offsetbox import TextArea, DrawingArea, OffsetImage, AnnotationBbox
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
  
         xxxxxxxxxxxxxxxxxxxxxx
           /_/  | x__[__]____    
          | |   |[___________]     
 ______   '-'(_)=    (_(_)(_)  ___   ______            _______  _
(  ____ \(  ___  )(  ___  )(  __  \ (  ___ \ |\     /|(  ____ \| |
| (    \/| (   ) || (   ) || (  \  )| (   ) )( \   / )| (    \/| |
| |      | |   | || |   | || |   ) || (__/ /  \ (_) / | (__    | |
| | ____ | |   | || |   | || |   | ||  __ (    \   /  |  __)   | |
| | \_  )| |   | || |   | || |   ) || (  \ \    ) (   | (      |_|
| (___) || (___) || (___) || (__/  )| )___) )   | |   | (____/\ _ 
(_______)(_______)(_______)(______/ |______/    \_/   (_______/(_)                                         
"""

#number of samples for one line from the camera
n = 640
#maximum value for an uint8
max_value = 255
#dimensions of the map in cm
max_X = 200
max_Y = 200
#creation of the map matrix
map_matrix = np.zeros((max_Y,max_X))

#colormap
colormap = matplotlib.colors.ListedColormap(['#dadadb','#c1c0bf', 'black', '#fe9200', 'red','red','yellow','red','yellow']) #in order: void,ground,walls,fire1,e-pouck ext, e-pouck int, arrow, fire2, fire3
bounds = [0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5]
perso_norm = matplotlib.colors.BoundaryNorm(bounds, colormap.N)

#COLOR CONSTANTS
C_VOID = 1
C_GROUND = 2
C_WALLS = 3
C_FIRE_ORANGE = 4
C_EPUCK_EXT = 5
C_EPUCK_INT = 6
C_ARROW = 7
C_FIRE_RED = 8
C_FIRE_YELLOW = 9

#CONSTANTS FOR TRANSMISSION
DO_NOTHING = 0
CORRIDOR = 1 
MOVING_IN_INTERSECTION = 2 #GO FORWARD WITHOUT DRAWING WALLS

#INTERSECTIONS
D_FIRE = 11
T_CROSS_RL = 12
T_CROSS_UR = 13
T_CROSS_UL = 14

L_LEFT_CROSS = 15
L_RIGHT_CROSS = 16

X_CROSS = 17

DEAD_END = 18

UNKNOWN = 19

#ORIENTATIONS
FACING_UP = 21
FACING_DOWN = 22
FACING_RIGHT = 23
FACING_LEFT = 24

#INTERNAL ORDERS
STAY = 0
GO = 1

#VARIABLES
orientation = FACING_UP

#EPUCK POSITION
#starting point
start_X = 100
start_Y = 190

posR_Y = 50
posR_X = 90

#WINDOW STUFF
#handler when closing the window
def handle_close(evt):
    #we stop the serial thread
    reader_thd.stop()
    print(goodbye)

def draw_epuck():
    global posF_X
    global posF_Y
    global map_matrix
    #ground from e-puck position
    map_matrix[(posR_Y-5):(posR_Y+6),(posR_X-5):(posR_X+6)] =     C_GROUND

    #ROBOT MODEL
    #plot a circle of diameter 7 at robot position
    map_matrix[(posR_Y-3),(posR_X-1):(posR_X+2)] =                C_EPUCK_EXT
    map_matrix[(posR_Y-2),(posR_X-2)] =                           C_EPUCK_EXT
    map_matrix[(posR_Y-2),(posR_X+2)] =                           C_EPUCK_EXT
    map_matrix[(posR_Y-1):(posR_Y+2),(posR_X-3)] =                C_EPUCK_EXT
    map_matrix[(posR_Y-1):(posR_Y+2),(posR_X+3)] =                C_EPUCK_EXT
    map_matrix[(posR_Y+2),(posR_X-2)] =                           C_EPUCK_EXT
    map_matrix[(posR_Y+2),(posR_X+2)] =                           C_EPUCK_EXT
    map_matrix[(posR_Y+3),(posR_X-1):(posR_X+2)] =                C_EPUCK_EXT
    map_matrix[(posR_Y-2):(posR_Y+3),(posR_X-1):(posR_X+2)] =     C_EPUCK_INT
    map_matrix[(posR_Y-1):(posR_Y+2),(posR_X-2):(posR_X+3)] =     C_EPUCK_INT
    
def draw_fire():
    global posF_X
    global posF_Y
    global map_matrix

    #red layer
    map_matrix[(posF_Y+1):(posF_Y+4),(posF_X-4):(posF_X+5)] =     C_FIRE_RED
    map_matrix[(posF_Y-2):(posF_Y+1),(posF_X-2):(posF_X+4)] =     C_FIRE_RED
    map_matrix[posF_Y,(posF_X-3)] =                               C_FIRE_RED
    map_matrix[posF_Y,(posF_X+4)] =                               C_FIRE_RED
    map_matrix[(posF_Y-3),(posF_X-2):(posF_X+2)] =                C_FIRE_RED
    map_matrix[(posF_Y-3),(posF_X+3)] =                           C_FIRE_RED
    map_matrix[(posF_Y-4),(posF_X-2)] =                           C_FIRE_RED
    map_matrix[(posF_Y-4),(posF_X):(posF_X+2)] =                  C_FIRE_RED    
    map_matrix[(posF_Y-6):(posF_Y-3),(posF_X+1)] =                C_FIRE_RED
    
    #orange layer
    map_matrix[(posF_Y+2):(posF_Y+4),(posF_X-2):(posF_X+4)] =     C_FIRE_ORANGE
    map_matrix[(posF_Y):(posF_Y+2),(posF_X-1):(posF_X+2)] =       C_FIRE_ORANGE
    map_matrix[(posF_Y+1),(posF_X+2)] =                           C_FIRE_ORANGE
    map_matrix[(posF_Y-2):posF_Y,posF_X] =                        C_FIRE_ORANGE
    
    #yellow layer
    map_matrix[posF_Y:(posF_Y+2),posF_X:(posF_X+2)] =             C_FIRE_YELLOW
    map_matrix[(posF_Y+2):(posF_Y+4),(posF_X-1):(posF_X+3)] =     C_FIRE_YELLOW
    map_matrix[(posF_Y-1),posF_X] =                               C_FIRE_YELLOW
            
def update_epuck_position(STAY_OR_GO):
    global posR_Y
    global posR_X
    global map_matrix
    if(orientation == FACING_UP):
    #forward epuck in direction
        posR_Y = posR_Y - STAY_OR_GO
    #draw epuck
        draw_epuck()
    #arrow facing up
        map_matrix[(posR_Y-2):(posR_Y+3),posR_X] =                C_ARROW
        map_matrix[(posR_Y-1),(posR_X-1):(posR_X+2)] =            C_ARROW
        map_matrix[posR_Y,posR_X-2] =                             C_ARROW
        map_matrix[posR_Y,posR_X+2] =                             C_ARROW
    elif(orientation == FACING_DOWN):
    #forward epuck in direction
        posR_Y = posR_Y + STAY_OR_GO
    #draw epuck
        draw_epuck()
    #arrow facing down
        map_matrix[(posR_Y-2):(posR_Y+3),posR_X] =                C_ARROW
        map_matrix[(posR_Y+1),(posR_X-1):(posR_X+2)] =            C_ARROW
        map_matrix[posR_Y,posR_X-2] =                             C_ARROW
        map_matrix[posR_Y,posR_X+2] =                             C_ARROW
    elif(orientation == FACING_RIGHT):
    #forward epuck in direction
        posR_X = posR_X + STAY_OR_GO
    #draw epuck
        draw_epuck()
    #arrow facing right
        map_matrix[posR_Y,(posR_X-2):(posR_X+3)] =                C_ARROW
        map_matrix[(posR_Y-1):(posR_Y+2),(posR_X+1)] =            C_ARROW
        map_matrix[posR_Y-2,posR_X] =                             C_ARROW
        map_matrix[posR_Y+2,posR_X] =                             C_ARROW
    elif(orientation == FACING_LEFT):
    #forward epuck in direction
        posR_X = posR_X - STAY_OR_GO
    #draw epuck
        draw_epuck()
    #arrow facing right
        map_matrix[posR_Y,(posR_X-2):(posR_X+3)] =                C_ARROW
        map_matrix[(posR_Y-1):(posR_Y+2),(posR_X-1)] =            C_ARROW
        map_matrix[posR_Y-2,posR_X] =                             C_ARROW
        map_matrix[posR_Y+2,posR_X] =                             C_ARROW
        
def add_walls():
    global map_matrix
    if(orientation == FACING_UP or orientation == FACING_DOWN):
    #walls left and right
        map_matrix[posR_Y,posR_X-6] =                             C_WALLS
        map_matrix[posR_Y,posR_X+6] =                             C_WALLS
    elif(orientation == FACING_RIGHT or orientation == FACING_LEFT):
    #walls up and down
        map_matrix[posR_Y-6,posR_X] =                             C_WALLS
        map_matrix[posR_Y+6,posR_X] =                             C_WALLS
        
def add_fire_front():
    global map_matrix
    global posF_X
    global posF_Y
    #place fire
    if(orientation == FACING_UP):
        posF_X = posR_X 
        posF_Y = posR_Y - 11
    elif(orientation == FACING_DOWN):
        posF_X = posR_X 
        posF_Y = posR_Y + 11
    elif(orientation == FACING_RIGHT):
        posF_X = posR_X + 11
        posF_Y = posR_Y
    elif(orientation == FACING_LEFT):
        posF_X = posR_X - 11
        posF_Y = posR_Y
    draw_fire()
    
#INTERSECTION MODELS---------------------------------------------------------------------------

def add_T_cross_up():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-10):posR_X+11] =                C_WALLS
    map_matrix[(posR_Y-6),(posR_X-10):(posR_X-5)] =               C_WALLS
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y-10):(posR_Y-5),(posR_X-6)] =               C_WALLS
    map_matrix[(posR_Y-10):(posR_Y-5),(posR_X+6)] =               C_WALLS
def add_T_cross_down():
    global map_matrix
    map_matrix[(posR_Y-6),(posR_X-10):posR_X+11] =                C_WALLS
    map_matrix[(posR_Y+6),(posR_X-10):(posR_X-5)] =               C_WALLS
    map_matrix[(posR_Y+6),(posR_X+6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y+6):(posR_Y+11),(posR_X-6)] =               C_WALLS
    map_matrix[(posR_Y+6):(posR_Y+11),(posR_X+6)] =               C_WALLS
def add_T_cross_right():
    global map_matrix
    map_matrix[(posR_Y-10):(posR_Y+11),(posR_X-6)] =              C_WALLS
    map_matrix[(posR_Y-10):(posR_Y-5),(posR_X+6)] =               C_WALLS
    map_matrix[(posR_Y+6):(posR_Y+11),(posR_X+6)] =               C_WALLS
    map_matrix[(posR_Y+6),(posR_X+6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+11)] =               C_WALLS
def add_T_cross_left():
    global map_matrix
    map_matrix[(posR_Y-10):(posR_Y+11),(posR_X+6)] =              C_WALLS
    map_matrix[(posR_Y-10):(posR_Y-5),(posR_X-6)] =               C_WALLS
    map_matrix[(posR_Y+6):(posR_Y+11),(posR_X-6)] =               C_WALLS
    map_matrix[(posR_Y+6),(posR_X-10):(posR_X-5)] =               C_WALLS
    map_matrix[(posR_Y-6),(posR_X-10):(posR_X-5)] =               C_WALLS
    
def add_L_down_left_cross():
    global map_matrix
    map_matrix[(posR_Y-6),(posR_X-10):(posR_X+7)] =               C_WALLS
    map_matrix[(posR_Y+6),(posR_X-10):(posR_X-5)] =               C_WALLS
    map_matrix[(posR_Y-6):(posR_Y+11),(posR_X+6)] =               C_WALLS
    map_matrix[(posR_Y+6):(posR_Y+11),(posR_X-6)] =               C_WALLS
def add_L_down_right_cross():
    global map_matrix
    map_matrix[(posR_Y-6),(posR_X-6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y+6),(posR_X+6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y-6):(posR_Y+11),(posR_X-6)] =               C_WALLS
    map_matrix[(posR_Y+6):(posR_Y+11),(posR_X+6)] =               C_WALLS

def add_L_up_left_cross():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-10):(posR_X+7)] =               C_WALLS
    map_matrix[(posR_Y-6),(posR_X-10):(posR_X-5)] =               C_WALLS
    map_matrix[(posR_Y-10):(posR_Y+7),(posR_X+6)] =               C_WALLS
    map_matrix[(posR_Y-10):(posR_Y-5),(posR_X-6)] =               C_WALLS

def add_L_up_right_cross():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y-10):(posR_Y-5),(posR_X+6)] =               C_WALLS
    map_matrix[(posR_Y-10):(posR_Y+7),(posR_X-6)] =               C_WALLS
    
def add_X_cross():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-10):(posR_X-5)] =               C_WALLS
    map_matrix[(posR_Y+6):(posR_Y+11),(posR_X-6)] =               C_WALLS
    map_matrix[(posR_Y+6),(posR_X+6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y+6):(posR_Y+11),(posR_X+6)] =               C_WALLS
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+11)] =               C_WALLS
    map_matrix[(posR_Y-10):(posR_Y-5),(posR_X+6)] =               C_WALLS
    map_matrix[(posR_Y-6),(posR_X-10):(posR_X-5)] =               C_WALLS
    map_matrix[(posR_Y-10):(posR_Y-5),(posR_X-6)] =               C_WALLS
    
def add_dead_end_up():
    global map_matrix
    map_matrix[(posR_Y-6),(posR_X-6):(posR_X+7)] =                C_WALLS
    map_matrix[(posR_Y-6):(posR_Y+1),(posR_X-6)] =                C_WALLS
    map_matrix[(posR_Y-6):(posR_Y+1),(posR_X+6)] =                C_WALLS
def add_dead_end_down():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-6):(posR_X+7)] =                C_WALLS
    map_matrix[(posR_Y):(posR_Y+7),(posR_X-6)] =                  C_WALLS
    map_matrix[(posR_Y):(posR_Y+7),(posR_X+6)] =                  C_WALLS
def add_dead_end_right():
    global map_matrix
    map_matrix[(posR_Y-6):(posR_Y+7),(posR_X+6)] =                C_WALLS
    map_matrix[(posR_Y-6),(posR_X):(posR_X+7)] =                  C_WALLS
    map_matrix[(posR_Y+6),(posR_X):(posR_X+7)] =                  C_WALLS
def add_dead_end_left(): 
    global map_matrix
    map_matrix[(posR_Y-6):(posR_Y+7),(posR_X-6)] =                C_WALLS
    map_matrix[(posR_Y-6),(posR_X-6):(posR_X+1)] =                C_WALLS
    map_matrix[(posR_Y+6),(posR_X-6):(posR_X+1)] =                C_WALLS

    
def draw_crossing(data):
# || data == T_CROSS_UR || data == T_CROSS_UL || data == L_LEFT_CROSS || data == L_RIGHT_CROSS || data == X_CROSS
    if(data == T_CROSS_RL):
        if(orientation == FACING_UP):
            add_T_cross_down()
        elif(orientation == FACING_DOWN):
            add_T_cross_up()
        elif(orientation == FACING_RIGHT):
            add_T_cross_left()
        elif(orientation == FACING_LEFT):
            add_T_cross_right()

    elif(data == T_CROSS_UR):
        if(orientation == FACING_UP):
            add_T_cross_right()
        elif(orientation == FACING_DOWN):
            add_T_cross_left()
        elif(orientation == FACING_RIGHT):
            add_T_cross_down()
        elif(orientation == FACING_LEFT):
            add_T_cross_up()
            
    elif(data == T_CROSS_UL):
        if(orientation == FACING_UP):
            add_T_cross_left()
        elif(orientation == FACING_DOWN):
            add_T_cross_right()
        elif(orientation == FACING_RIGHT):
            add_T_cross_up()
        elif(orientation == FACING_LEFT):
            add_T_cross_down()
            
    elif(data == L_LEFT_CROSS):
        if(orientation == FACING_UP):
            add_L_down_left_cross()
        elif(orientation == FACING_DOWN):
            add_L_up_right_cross()
        elif(orientation == FACING_RIGHT):
            add_L_up_left_cross()
        elif(orientation == FACING_LEFT):
            add_L_down_right_cross()
            
    elif(data == L_RIGHT_CROSS):
        if(orientation == FACING_UP):
            add_L_down_right_cross()          
        elif(orientation == FACING_DOWN):
            add_L_up_left_cross()
        elif(orientation == FACING_RIGHT):
            add_L_down_left_cross()
        elif(orientation == FACING_LEFT):
            add_L_up_right_cross()
            
    elif(data == X_CROSS):
        add_X_cross()   
    
    elif(data == DEAD_END):
        if(orientation == FACING_UP):
            add_dead_end_up()          
        elif(orientation == FACING_DOWN):
            add_dead_end_down() 
        elif(orientation == FACING_RIGHT):
            add_dead_end_right() 
        elif(orientation == FACING_LEFT):
            add_dead_end_left() 
    
#------------------------------------------------------------------------------------------------    
def update_plot_elements(data):
    #update orientation if needed
    global posR_X
    global posR_Y
    global orientation

    if(data == FACING_UP or data == FACING_DOWN or data == FACING_RIGHT or data == FACING_LEFT):
        #global orientation 
        orientation = data
        update_epuck_position(STAY)   

    elif(data == MOVING_IN_INTERSECTION):
        update_epuck_position(GO)        
        update_epuck_position(GO)
        
    elif(data == CORRIDOR):
        update_epuck_position(GO)
        add_walls()
        update_epuck_position(GO)
        add_walls()
        
    elif(data == T_CROSS_RL or data == T_CROSS_UR or data == T_CROSS_UL or data == L_LEFT_CROSS or data == L_RIGHT_CROSS or data == X_CROSS or data == DEAD_END):
        draw_crossing(data)

    elif(data == D_FIRE):
        add_fire_front()
        
    elif(data == UNKNOWN):
        print("UNKNOWN")
        
    else:
        update_epuck_position(STAY)

#------------------------------------------------------------------------------------------------
def clear_map():
    global map_matrix
    #empty map matrix
    map_matrix[0:max_Y,0:max_X] = C_VOID 
    
    #BACKGROUND
    #start background
    map_matrix[start_Y:(start_Y+7),(start_X-5):(start_X+6)] = C_GROUND

    #WALLS
    #start walls
    #map_matrix[start_Y:(start_Y+7),start_X-6] = 3
    #map_matrix[start_Y:(start_Y+7),start_X+6] = 3
    map_matrix[start_Y+6,(start_X-6):(start_X+7)] = C_WALLS
    
    #position offset of the e-puck (in the bottom middle)
    global posR_X
    global posR_Y
    posR_X = start_X
    posR_Y = start_Y
    
    global orientation
    orientation = FACING_UP
    
    update_epuck_position(STAY)
    
#PLOT UPDATES-------------------------------------------------------------------------------------
#update the plots
def update_plot():
    if(reader_thd.need_to_update_plot()):

        fig.canvas.draw_idle()

        reader_thd.plot_updated()

#function used to update the plot of the map data
def update_map_plot(port):
    data_read = readUint8Serial(port)
    data = data_read
    print(data)
    update_plot_elements(data) 
    map_plot.set_data(map_matrix)
    fig.canvas.draw_idle()
    reader_thd.tell_to_update_plot()
    #else:
     #   print("not data")

#reads the data in uint8 from the serial-------------------------------------------------------------
def readUint8Serial(port):

    state = 0
#waits to recieve the string "start" to begin reading
    while(state != 5):

        #reads 1 byte
        c1 = port.read(1)
        #timeout condition
        if(c1 == b''):
            print('Timout..')
            return [];

        if(state == 0):
            if(c1 == b'S'):
                state = 1
            else:
                state = 0
        elif(state == 1):
            if(c1 == b'T'):
                state = 2
            elif(c1 == b'S'):
                state = 1
            else:
                state = 0
        elif(state == 2):
            if(c1 == b'A'):
                state = 3
            elif(c1 == b'S'):
                state = 1
            else:
                state = 0
        elif(state == 3):
            if(c1 == b'R'):
                state = 4
            elif (c1 == b'S'):
                state = 1
            else:
                state = 0
        elif(state == 4):
            if(c1 == b'T'):
                state = 5
            elif (c1 == b'S'):
                state = 1
            else:
                state = 0

    rcv_buffer = port.read(2)
    data_read = 0

    data_read = rcv_buffer[0]
    print('received !')
    return data_read    


#thread used to control the communication part
class serial_thread(Thread):

    #init function called when the thread begins
    def __init__(self, port):
        Thread.__init__(self)
        self.contReceive = False
        self.alive = True
        self.need_to_update = False

        print('Connecting to port {}'.format(port))

        try:
            self.port = serial.Serial(port, timeout=0.5)
        except:
            print('Cannot connect to the e-puck2')
            sys.exit(0)
    #function called after the init
    def run(self):
        
        while(self.alive):

            if(self.contReceive):
                #print("recoit")
                update_map_plot(self.port)
            else:
                #flush the serial
                #clear_map()
                #print("sleep")
                self.port.read(self.port.inWaiting())
                time.sleep(0.1)
            #refresh_map()

    #enables the continuous reading
    def setContReceive(self, val): 
        self.contReceive = True

    #disables the continuous reading
    def stop_reading(self, val):
        print("stopped reading")
        self.contReceive = False

    #tell the plot need to be updated
    def tell_to_update_plot(self):
        self.need_to_update = True

    #tell the plot has been updated
    def plot_updated(self):
        self.need_to_update = False

    #tell if the plot need to be updated
    def need_to_update_plot(self):
        return self.need_to_update

    #clean exit of the thread if we need to stop it
    def stop(self):
        self.alive = False
        self.join()
        if(self.port.isOpen()):
            while(self.port.inWaiting() > 0):
                self.port.read(self.port.inWaiting())
                time.sleep(0.01)
            self.port.close()

        
#test if the serial port as been given as argument in the terminal
if len(sys.argv) == 1:
    print('Please give the serial port to use as argument')
    sys.exit(0)

#serial reader thread config
#begins the serial thread
reader_thd = serial_thread(sys.argv[1])
reader_thd.start()

#figure config-------------------------------------------------------------------------------------------------------------------------
fig, ax = plt.subplots(num=None, figsize=(10, 9), dpi=80)
ax.set_xlim(0, max_X)
ax.set_ylim(0, max_Y)
#fig.canvas.set_window_title('Map of the house')
plt.subplots_adjust(left=0.1, bottom=0.25)
fig.patch.set_facecolor('black')
fig.canvas.mpl_connect('close_event', handle_close) #to detect when the window is closed and if we do a ctrl-c
plt.gca().invert_yaxis()

#cam graph config with initial plot
graph_cam = plt.subplot(111)
#graph_cam.set_ylim([0, max_value])
map_reshape = map_matrix.reshape(max_X, max_Y)
map_plot = plt.imshow(map_reshape, cmap = colormap, norm = perso_norm)
#plt.plot(np.arange(0,n,1), np.linspace(max_value, max_value, n),lw=1, color='red')

#timer to update the plot from within the state machine of matplotlib
#because matplotlib is not thread safe...
timer = fig.canvas.new_timer(interval=50)
timer.add_callback(update_plot)
timer.start()

def handle_clear(val):
    print("cleared")
    clear_map()
    map_plot.set_data(map_matrix)
    fig.canvas.draw_idle()

#positions of the buttons, sliders and radio buttons
colorAx             = 'lightgoldenrodyellow'
receiveAx           = plt.axes([0.333, 0.025, 0.1, 0.04])
stopAx              = plt.axes([0.44, 0.025, 0.1, 0.04])
clearAx             = plt.axes([0.555, 0.025, 0.1, 0.04])

#config of the buttons, sliders and radio buttons
receiveButton           = Button(receiveAx, 'Start reading', color=colorAx, hovercolor='0.975')
stop                    = Button(stopAx, 'Stop reading', color=colorAx, hovercolor='0.975')
clearbutton             = Button(clearAx, 'clear', color=colorAx, hovercolor='0.975')

#callback config of the buttons, sliders and radio buttons
receiveButton.on_clicked(reader_thd.setContReceive)
stop.on_clicked(reader_thd.stop_reading)
clearbutton.on_clicked(handle_clear)

#starts the matplotlib main
plt.show()