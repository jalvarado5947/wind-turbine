## import the serial library
import serial
import string
import math
import time
from Tkinter import *
from threading import Timer

class App:

    grid_size = 15
    num_pixels = 30
    image_started = FALSE
    image_current_row = 0;
    ser = serial.Serial()
    pixel_dictionary = {}
    
    def __init__(self, master):
        master.title("Wind Turbine System Test")

        frame = Frame(master)
        frame.grid(row=0,column=0)

        self.textFileStr = StringVar()
        self.textFile = Entry(frame,textvariable=self.textFileStr)
        self.textFile.grid(row=0,column=0)
        self.textFile.delete(0,END)
        self.textFile.insert(0,"Enter Text File")

        self.windStr = StringVar()
        self.windSpeed = Entry(frame,textvariable=self.windStr)
        self.windSpeed.grid(row=0,column=1)
        self.windSpeed.delete(0, END)
        self.windSpeed.insert(0,"Enter Wind Speed")

        self.rotorDiameter = StringVar()
        self.textFile = Entry(frame,textvariable=self.rotorDiameter)
        self.textFile.grid(row=0,column=2)
        self.textFile.delete(0,END)
        self.textFile.insert(0,"Enter Rotor Diameter")


        self.send_button = Button(frame, text="Start Test", command=self.open_serial)
        self.send_button.grid(row=1,column=0)

        self.send_button = Button(frame, text="Stop Test", command=self.send_to_serial)
        self.send_button.grid(row=1,column=1)
        
        self.send_button = Button(frame, text="Start Sample", command=self.send_to_serial)
        self.send_button.grid(row=2,column=0)

        self.send_button = Button(frame, text="Stop Sample", command=self.send_to_serial)
        self.send_button.grid(row=2,column=1)

    def open_serial(self):
        # close the serial port
        if( self.ser.isOpen() ):
            try:
                self.ser.close()
            except:
                i=i  # do nothing
        # open the serial port
    ## establish connection to the serial port that your arduino 
        ## is connected to.

        locations=['/dev/ttyACM0','/dev/ttyACM1','/dev/ttyACM2','/dev/ttyACM3']


        for device in locations:
            try:
                print "Trying...",device
                ser = serial.Serial(device, 9600)
                break
            except:
                print "Failed to connect on",device
                
    def send_to_serial(self):
        if self.ser.isOpen():
            self.ser.write(self.windStr.get())
            self.ser.write(self.rotorDiameter.get())
            print "sent '" + self.windStr.get() + " "+ self.rotorDiameter.get()
        else:
            print "Serial port not open!"
            
## main loop ##

root = Tk()
#root.withdraw()
#serPort = SerialHandler(comPort,comPortBaud)

# create main display
app = App(root)

print("entering main loop!")

root.mainloop()

app.stop_read_loop()

## Boolean variable that will represent 
## whether or not the arduino is connected
connected = False



## loop until the arduino tells us it is ready
while not connected:
    serin = ser.read()
    connected = True

## open text file to store the current 

write_text_file = open(text_file, 'w')
## read serial data from arduino and 
## write it to the text file 'position.txt'
while 1:
    if ser.inWaiting():
        
        x=ser.readline()
        print(x) 
        write_text_file.write(x)
        if x=="\n":
             write_text_file.seek(0)
             write_text_file.truncate()
             write_text_file.flush()

## close the serial connection and text file
write_text_file.close()
ser.close()
