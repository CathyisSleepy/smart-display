// import library
import QtQuick 2.8
import QtQuick.Controls 2.1
 
// properties of the application window containing UI elements
ApplicationWindow {
    id: application
    width: 1280
    height: 720
    visible: true
    visibility: "FullScreen"
 
    // initialize the first window of the application
    property var iniITEM: "HomeScreen.qml"

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
            stackview.push("HomeScreen.qml",StackView.Immediate),
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
            stackview.push("SettingsScreen.qml",StackView.Immediate),
            button1.enabled = false,
            button.enabled = true
        }
    }

    Item{

        focus: true
        property bool ethfault: false
    
        Component.onCompleted: _Streaming.EthSignal.connect(ethfaultchangevalue)

        Popup{
            id: popup
            x: 100
            y: 100
            width: 200
            height: 300
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

            Text{
                x: 150
                y: 150
                text: "CONTROLLER CONNECTION FAULT"
                font.pixelSize: 35
            }
        }

        function ethfaultchangevalue(value){
            if(value != undefined){
                ethfault = value
            
                if(value == 1){
                    popup.open()
                }
            }
        } 
    }

}