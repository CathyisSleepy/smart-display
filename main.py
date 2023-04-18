# import libraries
import sys, os

#check if we are using pyside2 or pyqt5 and import the correct library
try:
	import PySide2.QtQml
except ImportError:
	import PyQt5.QtQml

if 'PyQt5' in sys.modules:
	from PyQt5.QtQml import QQmlApplicationEngine
	from PyQt5.QtWidgets import *
	from PyQt5.QtCore import *
	print("this app use pyqt5")
else:
	from PySide2.QtQml import QQmlApplicationEngine
	from PySide2.QtWidgets import *
	from PySide2.QtQuick import *
	from PySide2.QtCore import *
	print("this app use pyside2")

#import the classes we are using
from hmiControl import Setting
from hmiStream import Streaming
 
# launch the app
if __name__ == '__main__':
    app = QApplication([])
    engine = QQmlApplicationEngine()
    # location of the fullscreen app
    url = QUrl("./App.qml")
    context = engine.rootContext()
    #storing classes in vars
    seting = Setting()
    streaming = Streaming()

    #setting context for QML files making _Setting and _Streaming callable
    context.setContextProperty("_Setting", seting)
    context.setContextProperty("_Streaming", streaming)

    #start the stream thread
    streaming.start()

    #load and start the app
    engine.load(url)
    app.exec_()