// import library
import QtQuick 2.8
import QtQuick.Controls 2.1
 
// properties of the application window containing UI elements
ApplicationWindow {
    id: application
    width: 800
    height: 480
    visible: true
    visibility: "FullScreen"
 
    // initialize the first window of the application
    property var iniITEM: "HomeScreen.qml"

    // stack-based navigation model
    StackView {
        id: stackview
        initialItem: iniITEM
    }

    //navigation buttons on sidebar
    Button {
        id: home
        x: -11
        y: -12
        width: 80
        height: 80
        scale: 0.76
        onClicked: {
            stackview.push("HomeScreen.qml",StackView.Immediate),
            button1.enabled = true,
            button.enabled = false 
        }
    }

    Button {
        id: settings
        x: -11
        y: 60
        width: 80
        height: 80
        scale: 0.76
        onClicked: {
            stackview.push("SettingsScreen.qml",StackView.Immediate),
            button1.enabled = false,
            button.enabled = true
        }
    }
    
    //popups for estop and ethernet warnings
    Item{
        id: warnings
        focus: true
        property bool ethfault: false
        property bool estopped: false
    
        Component.onCompleted: {
            _Streaming.EthSignal.connect(ethfaultchangevalue),
            _Streaming.EstopSignal.connect(estoppedchangevalue)
        } 

        Popup{
            id: conn_error
            x: 5
            y: 5
            width: 750
            height: 400
            modal: true
            focus: true
            closePolicy: Popup.NoAutoClose

            Text{
                x: 150
                y: 150
                text: "CONTROLLER CONNECTION FAULT"
                font.pixelSize: 35
            }

            Button{
                id: reset
                x: 500
                y: 200
                width: 175
                height: 75
                palette.button: "blue"
                palette.buttonText: "white"
                text: "Reconnect"
                onClicked:
                {
                    _Setting.ethReset()
                }
            }
        }

        Popup{
            id: estop_error
            x: 5
            y: 5
            width: 750
            height: 400
            modal: true
            focus: true
            closePolicy: Popup.NoAutoClose

            Text{
                x: 150
                y: 150
                text: "ESTOP PRESSED"
                font.pixelSize: 35
            }
        }

        //open the estop pop up if the estop is pressed
        //close if it is released
        function estoppedchangevalue(value){
            if(value != undefined){
                estopped = value

                if(estopped == 1){
                    estop_error.open()
                }

                if(estopped == 0){
                    estop_error.close()
                }
            }  
        }

        //open the ethernet fault pop up if the connection is broken
        //close if reconnnected
        function ethfaultchangevalue(value){
            if(value != undefined){
                ethfault = value

                if(ethfault == true){
                    conn_error.open()
                }

                if(ethfault == false){
                    conn_error.close()
                }
            }
        } 
    }

}