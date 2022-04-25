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
  
         xxxxxxxxxxxxxxxxxxxxxx
           /_/  | x__[__]____    
          | |   |[___________]     
          '-'(_)=    (_(_)(_)  ___   ______            _______  _
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
colormap = matplotlib.colors.ListedColormap(['white','#c1c0bf', 'black', '#fe9200', '#0d48b1','red','yellow']) #in order: void,ground,walls,fire,e-pouck ext, e-pouck int, arrow
bounds = [0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5]
perso_norm = matplotlib.colors.BoundaryNorm(bounds, colormap.N)

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
start_X = 50
start_Y = 90

posR_Y = 50
posR_X = 90

#WINDOW STUFF
#handler when closing the window
def handle_close(evt):
    #we stop the serial thread
    reader_thd.stop()
    print(goodbye)

def draw_epuck():
    #ground from e-puck position
    map_matrix[(posR_Y-4):(posR_Y+5),(posR_X-5):(posR_X+6)] = 2

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
    
def draw_hole_up():
    #background hole up
    map_matrix[(posR_Y-16):(posR_Y-5),(posR_X-5):(posR_X+6)] = 2
def draw_hole_down():
    #background hole down
    map_matrix[(posR_Y+6):(posR_Y+17),(posR_X-5):(posR_X+6)] = 2
def draw_hole_right():
    #background hole right
    map_matrix[(posR_Y-5):(posR_Y+6),(posR_X+6):(posR_X+17)] = 2
def draw_hole_left():
    #background hole left
    map_matrix[(posR_Y-5):(posR_Y+6),(posR_X-16):(posR_X-5)] = 2
            
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
        map_matrix[(posR_Y-2):(posR_Y+3),posR_X] = 7
        map_matrix[(posR_Y-1),(posR_X-1):(posR_X+2)] = 7
        map_matrix[posR_Y,posR_X-2] = 7
        map_matrix[posR_Y,posR_X+2] = 7
    elif(orientation == FACING_DOWN):
    #forward epuck in direction
        posR_Y = posR_Y + STAY_OR_GO
    #draw epuck
        draw_epuck()
    #arrow facing down
        map_matrix[(posR_Y-2):(posR_Y+3),posR_X] = 7
        map_matrix[(posR_Y+1),(posR_X-1):(posR_X+2)] = 7
        map_matrix[posR_Y,posR_X-2] = 7
        map_matrix[posR_Y,posR_X+2] = 7
    elif(orientation == FACING_RIGHT):
    #forward epuck in direction
        posR_X = posR_X + STAY_OR_GO
    #draw epuck
        draw_epuck()
    #arrow facing right
        map_matrix[posR_Y,(posR_X-2):(posR_X+3)] = 7
        map_matrix[(posR_Y-1):(posR_Y+2),(posR_X+1)] = 7
        map_matrix[posR_Y-2,posR_X] = 7
        map_matrix[posR_Y+2,posR_X] = 7
    elif(orientation == FACING_LEFT):
    #forward epuck in direction
        posR_X = posR_X - STAY_OR_GO
    #draw epuck
        draw_epuck()
    #arrow facing right
        map_matrix[posR_Y,(posR_X-2):(posR_X+3)] = 7
        map_matrix[(posR_Y-1):(posR_Y+2),(posR_X-1)] = 7
        map_matrix[posR_Y-2,posR_X] = 7
        map_matrix[posR_Y+2,posR_X] = 7
        
def add_walls():
    global map_matrix
    if(orientation == FACING_UP or orientation == FACING_DOWN):
    #walls left and right
        map_matrix[(posR_Y-1):(posR_Y+2),posR_X-6] = 3
        map_matrix[(posR_Y-1):(posR_Y+2),posR_X+6] = 3
    elif(orientation == FACING_RIGHT or orientation == FACING_LEFT):
    #walls up and down
        map_matrix[posR_Y-6,(posR_X-1):(posR_X+2)] = 3
        map_matrix[posR_Y+6,(posR_X-1):(posR_X+2)] = 3   

def add_right_hole():
    if(orientation == FACING_UP):
        draw_hole_right()
    elif(orientation == FACING_DOWN):
        draw_hole_left()
    elif(orientation == FACING_RIGHT):
        draw_hole_down()
    elif(orientation == FACING_LEFT):
        draw_hole_up()
        
def add_left_hole():
    if(orientation == FACING_UP):
        draw_hole_left()
    elif(orientation == FACING_DOWN):
        draw_hole_right()
    elif(orientation == FACING_RIGHT):
        draw_hole_up()
    elif(orientation == FACING_LEFT):
        draw_hole_down()
        
def add_fire_front():
    global map_matrix
    global posF_X
    global posF_Y
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
    map_matrix[(posF_Y-1):(posF_Y+2),(posF_X-1):(posF_X+2)] = 4
    
#INTERSECTION MODELS---------------------------------------------------------------------------
#    map_matrix[(posR_Y),(posR_X)]
def add_T_cross_up():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-12):posR_X+13] = 3
    map_matrix[(posR_Y-6),(posR_X-12):(posR_X-5)] = 3
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+13)] = 3
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X-6)] = 3
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X+6)] = 3
def add_T_cross_down():
    global map_matrix
    map_matrix[(posR_Y-6),(posR_X-12):posR_X+13] = 3
    map_matrix[(posR_Y+6),(posR_X-12):(posR_X-5)] = 3
    map_matrix[(posR_Y+6),(posR_X+6):(posR_X+13)] = 3
    map_matrix[(posR_Y+6):(posR_Y+13),(posR_X-6)] = 3
    map_matrix[(posR_Y+6):(posR_Y+13),(posR_X+6)] = 3
def add_T_cross_right():
    global map_matrix
    map_matrix[(posR_Y-12):(posR_Y+13),(posR_X-6)] = 3
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X+6)] = 3
    map_matrix[(posR_Y+6):(posR_Y+13),(posR_X+6)] = 3
    map_matrix[(posR_Y+6),(posR_X+6):(posR_X+13)] = 3
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+13)] = 3    
def add_T_cross_left():
    global map_matrix
    map_matrix[(posR_Y-12):(posR_Y+13),(posR_X+6)] = 3
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X-6)] = 3
    map_matrix[(posR_Y+6):(posR_Y+13),(posR_X-6)] = 3
    map_matrix[(posR_Y+6),(posR_X-12):(posR_X-5)] = 3
    map_matrix[(posR_Y-6),(posR_X-12):(posR_X-5)] = 3
    
def add_L_down_left_cross():
    global map_matrix
    map_matrix[(posR_Y-6),(posR_X-12):(posR_X+7)] = 3
    map_matrix[(posR_Y+6),(posR_X-12):(posR_X-5)] = 3   
    map_matrix[(posR_Y-6):(posR_Y+13),(posR_X+6)] = 3
    map_matrix[(posR_Y+6):(posR_Y+13),(posR_X-6)] = 3
def add_L_down_right_cross():
    global map_matrix
    map_matrix[(posR_Y-6),(posR_X-6):(posR_X+13)] = 3
    map_matrix[(posR_Y+6),(posR_X+6):(posR_X+13)] = 3  
    map_matrix[(posR_Y-6):(posR_Y+13),(posR_X-6)] = 3
    map_matrix[(posR_Y+6):(posR_Y+13),(posR_X+6)] = 3
def add_L_up_left_cross():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-6):(posR_X+13)] = 3
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+13)] = 3  
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X+6)] = 3
    map_matrix[(posR_Y-12):(posR_Y+7),(posR_X-6)] = 3
def add_L_up_right_cross():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-12):(posR_X+7)] = 3
    map_matrix[(posR_Y-6),(posR_X-12):(posR_X-5)] = 3   
    map_matrix[(posR_Y-12):(posR_Y+7),(posR_X+6)] = 3
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X-6)] = 3
    
def add_X_cross():
    global map_matrix
    map_matrix[(posR_Y+6),(posR_X-12):(posR_X-5)] = 3  
    map_matrix[(posR_Y+6):(posR_Y+13),(posR_X-6)] = 3
    map_matrix[(posR_Y+6),(posR_X+6):(posR_X+13)] = 3  
    map_matrix[(posR_Y+6):(posR_Y+13),(posR_X+6)] = 3
    map_matrix[(posR_Y-6),(posR_X+6):(posR_X+13)] = 3  
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X+6)] = 3
    map_matrix[(posR_Y-6),(posR_X-12):(posR_X-5)] = 3  
    map_matrix[(posR_Y-12):(posR_Y-5),(posR_X-6)] = 3
    
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
    
#------------------------------------------------------------------------------------------------    
def update_plot_elements(data):
    #update orientation if needed
    global posR_X
    global posR_Y
    global orientation
    print("posR_X: " +str(posR_X))
    print("posR_Y: " +str(posR_Y))
    print("orientation" +str(orientation))
    if(data == FACING_UP or data == FACING_DOWN or data == FACING_RIGHT or data == FACING_LEFT):
        #global orientation 
        orientation = data
        update_epuck_position(STAY)   

    elif(data == MOVING_IN_INTERSECTION):
        update_epuck_position(GO)
        #print("g")
    elif(data == CORRIDOR):
        update_epuck_position(GO)
        add_walls()

    elif(data == T_CROSS_RL or data == T_CROSS_UR or data == T_CROSS_UL or data == L_LEFT_CROSS or data == L_RIGHT_CROSS or data == X_CROSS):
        draw_crossing(data)

    elif(data == D_FIRE):
        add_fire_front()
        
    else:
        update_epuck_position(STAY)
    #print("updated")
    #print(" ")

#------------------------------------------------------------------------------------------------
def clear_map():
    global map_matrix
    #empty map matrix
    map_matrix[0:max_Y,0:max_X] = 0
    
    #BACKGROUND
    #start background
    map_matrix[start_Y:(start_Y+7),(start_X-5):(start_X+6)] = 2

    #WALLS
    #start walls
    map_matrix[start_Y:(start_Y+7),start_X-6] = 3
    map_matrix[start_Y:(start_Y+7),start_X+6] = 3
    map_matrix[start_Y+6,(start_X-6):(start_X+7)] = 3
    
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
    #print("lu")
       # map_plot.set_ydata(ir_data)
       # graph_cam.relim()
       # graph_cam.autoscale()
    
    data = data_read
    print(data)
    update_plot_elements(data) #uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
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
            print('Timout...')
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
    #print("start")
    #reads the size
    #converts as short int in little endian the two bytes read
    size = struct.unpack('<h',port.read(2)) 
    #removes the second element which is void
    size = size[0]  
   # print("size")
   # print(size)
    #reads the data
    rcv_buffer = port.read(size)
    data_read = 0

    #if we receive the good amount of data, we convert them in uint8
    if(len(rcv_buffer) == size):
       # print("in if")
        data_read = rcv_buffer[0]
        #print(rcv_buffer[0])
    #    i = 0
     #   while(i < size):
      #      data_tab.append(struct.unpack_from('<I',rcv_buffer, i))
       #     i = i+1

        print('received !')
        return data_read
    else:
        print('Timout...')
        return []


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
                print("recoit")
                update_map_plot(self.port)
            else:
                #flush the serial
                #clear_map()
                print("sleep")
                self.port.read(self.port.inWaiting())
                time.sleep(0.1)
            #refresh_map()

    #enables the continuous reading
    def setContReceive(self, val):  
        self.contReceive = True

    #disables the continuous reading
    def stop_reading(self, val):
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
fig.canvas.set_window_title('Map of the house')
plt.subplots_adjust(left=0.1, bottom=0.25)
fig.canvas.mpl_connect('close_event', handle_close) #to detect when the window is closed and if we do a ctrl-c

#cam graph config with initial plot
graph_cam = plt.subplot(111)
#graph_cam.set_ylim([0, max_value])
map_reshape = map_matrix.reshape(max_X, max_Y)#uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
map_plot = plt.imshow(map_reshape, cmap = colormap, norm = perso_norm)
#plt.plot(np.arange(0,n,1), np.linspace(max_value, max_value, n),lw=1, color='red')

#timer to update the plot from within the state machine of matplotlib
#because matplotlib is not thread safe...
timer = fig.canvas.new_timer(interval=50)
timer.add_callback(update_plot)
timer.start()

def handle_clear(val):
    clear_map()
    map_plot.set_data(map_matrix)
    fig.canvas.draw_idle()

#positions of the buttons, sliders and radio buttons
colorAx             = 'lightgoldenrodyellow'
receiveAx           = plt.axes([0.4, 0.025, 0.1, 0.04])
stopAx              = plt.axes([0.5, 0.025, 0.1, 0.04])
clearAx             = plt.axes([0.6, 0.025, 0.1, 0.04])

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