# import libraries
from PySide2.QtQml import QQmlApplicationEngine
from PySide2.QtWidgets import *
from PySide2.QtCore import *
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