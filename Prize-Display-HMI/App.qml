// import library
import QtQuick.Controls 2.1
 
// properties of the application window containing UI elements
ApplicationWindow {
    id: application
    width: 1280
    height: 720
    visible: true
    visibility: "FullScreen"
 
    // initialize the first window of the application
    property var iniITEM: "HmiGui.qml"
 
    // stack-based navigation model
    StackView {
        id: stackview
        initialItem: iniITEM
    }
}