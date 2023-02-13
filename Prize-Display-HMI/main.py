# import libraries
import sys, os

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

from hmiControl import Setting, Streaming
 
# launch the app
if __name__ == '__main__':
    app = QApplication([])
    engine = QQmlApplicationEngine()
    # location of the fullscreen app that we created before
    url = QUrl("./App.qml")
    context = engine.rootContext()
    seting = Setting()
    streaming = Streaming()

    context.setContextProperty("_Setting", seting)
    context.setContextProperty("_Streaming", streaming)

    streaming.start()

    engine.load(url)
    app.exec_()