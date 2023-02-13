
//import libraries
import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
    // identify the qml
    id: hmiControl
    // define width and height of the app
    width: 1280
    height: 720
    Keys.onEscapePressed:_Setting.closeWindow()

    property int mspeed: 0
    property bool mfault: false
    property bool estopped: false
    property bool running: false

    Component.onCompleted: {
        _Streaming.FaultSignal.connect(mfaultchangevalue)
        _Streaming.EstopSignal.connect(estoppedchangevalue)
        _Streaming.RunSignal.connect(runningchangevalue)
    }

    Rectangle{
        anchors.fill: parent
        color: "gray"
        
        Text {
            id: speed_label
            x: 500
            y: 200
            text: "Adjust Motor Speed" 
            font.pixelSize: 45
        }
    
        Text {
            id: speed
            x: 500
            y: 255
            visible: true
            text: mspeed
            font.pixelSize: 45
            Text {
                id: perc
                anchors.left: parent.right
                anchors.leftMargin: 3
                text: "%"
                font.pixelSize: 30
            }
        }

        Text {
            id: eth_fault
            visible: _Setting.getEthFault()
            x: 1000
            y: 100
            text: "CONNECTION FAULT"
            font.pixelSize: 30
            color: "red"
        }

        Text {
            id: fault
            visible: mfault
            x: 1000
            y: 140
            text: "MOTOR FAULT"
            font.pixelSize: 30
            color: "red"
        }

        Text {
            id: estop
            visible: estopped
            x: 1000
            y: 180
            text: "ESTOP PRESSED"
            font.pixelSize: 30
            color: "red"
        }

        Text {
            id: run_state
            visible: running
            x: 1000
            y: 220
            text: "Machine Stopped"
            font.pixelSize: 30
            color: "yellow"
        }

        Slider {
            id: motor_slider
            x: 450
            y: 360
            width: 456
            height: 62
            scale: 1.7
            stepSize: 1
            to: 255
            value: 0
            onValueChanged:
            { 
                _Setting.motorSpeedSet(value),
                mspeedchangevalue(value),
                setethfaultvisibility()
            }
        }
    
        Button {
            id: stop
            x: 800
            y: 600
            width: 200
            height: 100
            palette.button: "red"
            palette.buttonText: "white"
            text: "Stop"
            onClicked:
            {
                _Setting.ccStop(),
                setethfaultvisibility()
            }
        }

        Button {
            id: start
            x: 200
            y: 600
            width: 200
            height: 100
            palette.button: "green"
            palette.buttonText: "white"
            text: "Start"
            onClicked:
            {
                _Setting.ccStart(),
                setethfaultvisibility()
            }
        }

        Button {
            id: reset
            x: 1000
            y: 600
            width: 200
            height: 100
            palette.button: "blue"
            palette.buttonText: "white"
            text: "Reset"
            onClicked:
            {
                _Setting.ccReset(),
                setethfaultvisibility()
            }
        }
    }

    function mspeedchangevalue(value){
        if(value != undefined) {
            mspeed = parseInt(value / 255 * 100)
        }  
    }

    function mfaultchangevalue(value){
        if(value != undefined){
            mfault = value
        }
    }

    function estoppedchangevalue(value){
        if(value != undefined){
            estopped = value
        }
    }

    function runningchangevalue(value){
        if(value != undefined){
            running = value
        }
    }

    function setethfaultvisibility(){
        eth_fault.visible = _Setting.getEthFault()
    }
}