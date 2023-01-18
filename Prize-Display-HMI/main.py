# import libraries
from PySide2.QtQml import QQmlApplicationEngine
from PySide2.QtWidgets import *
from PySide2.QtCore import *
from hmiControl import Setting
 
# launch the app
if __name__ == '__main__':
    app = QApplication([])
    engine = QQmlApplicationEngine()
    # location of the fullscreen app that we created before
    url = QUrl("./App.qml")
    context = engine.rootContext()
    seting = Setting()
    context.setContextProperty("_Setting", seting)
    engine.load(url)
    app.exec_()