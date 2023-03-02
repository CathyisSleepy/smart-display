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
    property var iniITEM: "HmiHome.qml"
 
    // stack-based navigation model
    StackView {
        id: stackview
        initialItem: iniITEM
    }

    Button {
        id: button
        x: -21
        y: -22
        width: 172
        height: 181
        scale: 0.76
        onClicked: {
            stackview.pop(),
            stackview.push("HmiHome.qml",StackView.Immediate)
            button1.enabled = true,
            button.enabled = false
        }
    }

    Button {
        id: button1
        x: -21
        y: 172
        width: 172
        height: 181
        scale: 0.76
        onClicked: {
            stackview.pop()
            stackview.push("HmiSettings.qml",StackView.Immediate)
            button1.enabled = false
            button.enabled = true
        }
    }
}