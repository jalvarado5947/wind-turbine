# File: ADNS3080ImageGrabber.py
import string
import math
import time
from Tkinter import *

from threading import Timer

import sys, serial, argparse
from time import sleep
from collections import deque

import matplotlib.pyplot as plt
import matplotlib.animation as animation

from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
import pylab as p1

import sys, serial, argparse
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph.console


comPort = '/dev/ttyACM0'  #default com port
comPortBaud = 9600

#QtGui.QApplication.setGraphicsSystem('raster')
app = QtGui.QApplication([])
#mw = QtGui.QMainWindow()
#mw.resize(800,800)

class AnalogPlot:
    def save():
        global state
        state = area.saveState()
        restoreBtn.setEnabled(True)
    def load():
        global state
        area.restoreState(state)



    def main():
        parser = argparse.ArgumentParser(description="LDR serial")
          # add expected arguments
          
          #parser.add_argument('--poif __name__ == '__main__':
          #parser.add_argument('--port', dest='port', required=True)

          # parse args
          
        args = parser.parse_args()
    def addToBuf(buf,buf1,val):
          if len(buf) < maxLen:
              buf.append(val[0])
              buf1.append(val[5])
          else:
              buf.pop()
              buf1.pop()
              buf.append(val[0])
              buf1.append(val[5])
    def update():
        global curve, ptr, p6, counter, counter2

        line = ser.readline()
        dataArduino = [float(val) for val in line.split()]

        #dataArduino = [ counter ,counter2 ]
        curve.setData(data,data1)
        
        addToBuf(data, data1, dataArduino)# constr
  
class App:
            
    def __init__(self, master):

        # set main window's title
        master.title("Wind Turbine Test")
        self.write_text_file = open(" ", 'w')
        self.ser = serial.Serial()
        
        frame = Frame(master)
        frame.grid(row=0,column=0)

        
        self.comPortStr = StringVar()
        self.comPort = Entry(frame,textvariable=self.comPortStr)
        self.comPort.grid(row=0,column=0)
        self.comPort.delete(0, END)
        self.comPort.insert(0,comPort)

        self.rotorDiameter = StringVar()
        self.textFile = Entry(frame,textvariable=self.rotorDiameter)
        self.textFile.grid(row=0,column=1)
        self.textFile.delete(0,END)
        self.textFile.insert(0,"Enter Rotor Diameter")
        
        self.textFileStr = StringVar()
        self.textFile = Entry(frame,textvariable=self.textFileStr)
        self.textFile.grid(row=0,column=2)
        self.textFile.delete(0,END)
        self.textFile.insert(0,"Enter Text File")

        self.windStr = StringVar()
        self.windSpeed = Entry(frame,textvariable=self.windStr)
        self.windSpeed.grid(row=1,column=0)
        self.windSpeed.delete(0, END)
        self.windSpeed.insert(0,"Enter Wind Speed")

        self.Kp = StringVar()
        self.windSpeed = Entry(frame,textvariable=self.Kp)
        self.windSpeed.grid(row=1,column=1)
        self.windSpeed.delete(0, END)
        self.windSpeed.insert(0,"Enter Kp Value")
        
        self.Kd = StringVar()
        self.windSpeed = Entry(frame,textvariable=self.Kd)
        self.windSpeed.grid(row=1,column=2)
        self.windSpeed.delete(0, END)
        self.windSpeed.insert(0,"Enter Kd Value")
        

        self.button = Button(frame, text="Start Test", fg="red", command=self.open_serial)
        self.button.grid(row=2,column=0)

        self.button = Button(frame, text="Choose File", fg="blue", command=self.specify_file)
        self.button.grid(row=0,column=3)

        self.send_button = Button(frame, text="Start Sample", command=self.send_to_serial_start)
        self.send_button.grid(row=2,column=1)

        self.send_button = Button(frame, text="Stop Sample", command=self.send_to_serial_pause)
        self.send_button.grid(row=2,column=2)



        self.send_button = Button(frame, text="Brake", command=self.brakeTurbine)
        self.send_button.grid(row=3,column=0)

        self.send_button = Button(frame, text="Go", command=self.runTurbine)
        self.send_button.grid(row=3,column=1)
        
        self.send_button = Button(frame, text="graph", command=self.realtime_graph)
        self.send_button.grid(row=3,column=2)

        ## start attempts to read from serial port
        self.read_loop()

    def __del__(self):
        self.stop_read_loop()
        

    def specify_file(self):
        self.write_text_file = open(self.textFileStr.get(), 'w')
        print "File Specified is " + self.textFileStr.get()

    def brakeTurbine(self):
        self.ser.write('2')
        print "Braking Turbine " 

    def runTurbine(self):
        self.ser.write('0')
        print "Disengaging Electrical Brake " 
    
    def graph_powerVSresistance(self):
        max_sp2 = 0
        self.write_text_file.close()
        time.sleep(2)
        f = open(self.textFileStr.get())
        left_list = []
        right_list = []
        for l in f:
            sp = l.split("\t")
            sp1 = float(sp[1])
            sp2 = float(sp[5])
            if isinstance(sp1, float):
                if  (sp2> max_sp2-1):
                    left_list.append(float(sp1))
                    right_list.append(float(sp2))
                    max_sp2 = sp2 
        p1.plot(left_list,right_list,'o')
        p1.show()

    def realtime_graph(self):
        strPort = '/dev/ttyACM1'
        ser = serial.Serial(strPort, 9600)


        counter =0
        counter2 = 0

        #QtGui.QApplication.setGraphicsSystem('raster')
        app = QtGui.QApplication([])
        #mw = QtGui.QMainWindow()
        #mw.resize(800,800)

        win = pg.GraphicsWindow(title="Basic plotting examples")
        win.resize(1000,600)
        win.setWindowTitle('pyqtgraph example: Plotting')

        # Enable antialiasing for prettier plots
        pg.setConfigOptions(antialias=True)



        maxLen =10000
        p6 = win.addPlot(title="Updating plot")
        curve = p6.plot( pen=None, symbol='o', symbolPen=None, symbolSize=5)
        data = []
        data1= []

        ptr = 0



        timer = QtCore.QTimer()
        timer.timeout.connect(update)
        timer.start(1)


    def open_serial(self):
        # close the serial port
        if( self.ser.isOpen() ):
            try:
                self.ser.close()
            except:
                i=i  # do nothing
        # open the serial port
        try:
            self.ser = serial.Serial(port=self.comPortStr.get(),baudrate=comPortBaud, timeout=1)
            print("serial port '" + self.comPortStr.get() + "' opened!")            
        except:
            print("failed to open serial port '" + self.comPortStr.get() + "'")

    def send_to_serial_start(self):
        if self.ser.isOpen():
            self.ser.write('1')
            self.ser.write(self.windStr.get())
            self.ser.write(self.Kp.get())
            self.ser.write(self.Kd.get())
        else:
            print "Serial port not open!"

    def send_to_serial_pause(self):
        if self.ser.isOpen():
            self.ser.write('0')
        else:
            print "Serial port not open!"

    def read_loop(self):
        try:
            self.t.cancel()
        except:
            aVar = 1  # do nothing
        #print("reading")
        if( self.ser.isOpen() ) :
            self.read_from_serial();

        self.t = Timer(0.0,self.read_loop)
        self.t.start()

    def stop_read_loop(self):
        try:
            self.t.cancel()
        except:
            print("failed to cancel timer")
            # do nothing

    def read_from_serial(self):
        if self.ser.inWaiting():
            x=self.ser.readline()
            print(x)
            self.write_text_file.write(x)
            if x=="\n":
                 self.write_text_file.seek(0)
                 
                 self.write_text_file.truncate()
                 self.ser.flush()
                



## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
    import sys
    root = Tk()
    #root.withdraw()
    #serPort = SerialHandler(comPort,comPortBaud)

    # create main display
    app = App(root)

    print("entering main loop!")

    root.mainloop()
    
    app.stop_read_loop()

    app.write_text_file.close()
    app.ser.close()
    print("exiting")
    
