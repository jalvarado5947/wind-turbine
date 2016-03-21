
import sys, serial, argparse
from pyqtgraph.Qt import QtGui, QtCore

import pyqtgraph.console
import numpy as np
from time import sleep
from collections import deque
from pyqtgraph.dockarea import *

import pyqtgraph as pg

import matplotlib.pyplot as plt
import matplotlib.animation as animation


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
          buf1.append(val[1])
      else:
          buf.pop()
          buf1.pop()
          buf.append(val[0])
          buf1.append(val[1])
def update():
    global curve, ptr, p6, counter, counter2

    line = ser.readline()
    dataArduino = [float(val) for val in line.split()]

    #dataArduino = [ counter ,counter2 ]
    curve.setData(data,data1)
    
    addToBuf(data, data1, dataArduino)
    #counter += 1
    #counter2 = 5*counter

strPort = '/dev/ttyACM0'
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



maxLen =100000
p6 = win.addPlot(title="Updating plot")
curve = p6.plot( pen=None, symbol='o', symbolPen=None, symbolSize=5)
data = []
data1= []
ptr = 0



timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(1)


    


## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
    import sys
    #main()
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        QtGui.QApplication.instance().exec_()
