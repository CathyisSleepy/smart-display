
//import libraries
import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
    // identify the qml
    id: hmiControl
    // define width and height of the app
    width: 1280
    height: 720
    focus: true
    Keys.onEscapePressed:_Setting.closeWindow()

    property int motor_readout: 0
    property bool mfault: false
    property bool estopped: false
    property bool not_running: true
    property bool running: false
    property bool ethfault: false
    property bool auto_on: true

    Component.onCompleted: {
        _Streaming.FaultSignal.connect(mfaultchangevalue)
        _Streaming.EstopSignal.connect(estoppedchangevalue)
        _Streaming.RunSignal.connect(runningchangevalue)
        _Streaming.EthSignal.connect(ethfaultchangevalue)
        _Streaming.MotorSignal.connect(readoutchangevalue)
        setautolighting()
        setstartstoplighting()
        _Setting.motorSpeedGet()
        focus = true
    }

    Rectangle{
        anchors.fill: parent
        color: "light gray"
        focus: true
        
        Text {
            id: speed_label
            x: 500
            y: 200
            text: "Motor Readout" 
            font.pixelSize: 40
        }
    
        Text {
            id: speed_perc
            x: 500
            y: 255
            visible: true
            focus: true
            text: motor_readout
            font.pixelSize: 45
            Text {
                id: perc
                anchors.left: parent.right
                anchors.leftMargin: 3
                text: "%"
                font.pixelSize: 35
            }
        }

        Text {
            id: eth_fault_warning
            focus: true
            visible: ethfault
            x: 1000
            y: 100
            text: "CONNECTION FAULT"
            font.pixelSize: 30
            color: "red"
        }

        Text {
            id: fault_warning
            focus: true
            visible: mfault
            x: 1000
            y: 140
            text: "MOTOR FAULT"
            font.pixelSize: 30
            color: "red"
        }

        Text {
            id: estop_warning
            visible: estopped
            x: 1000
            y: 180
            text: "ESTOP PRESSED"
            font.pixelSize: 30
            color: "red"
        }

        Text {
            id: running_lable
            visible: running
            focus: true
            x: 200
            y: 220
            text: "Machine Running"
            font.pixelSize: 30
            color: "green"
        }

        Button {
            id: stop
            focus: true
            x: 800
            y: 600
            width: 200
            height: 100
            palette.button: "red"
            palette.buttonText: "white"
            text: "Stop"
            onClicked:
            {
                _Setting.ccStop()
            }
        }

        Button {
            id: start
            focus: true
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
                _Setting.motorSpeedGet()
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
                _Setting.ccReset()
            }
        }

        Button {
            id: manual
            x: 1000
            y: 350
            width: 200
            height: 100
            palette.button: "black"
            palette.buttonText: "white"
            text: "Manual"
            onClicked:
            {
                _Setting.ccManual(),
                setautolighting() 
            }
        }

        Button {
            id: auto
            x: 1000
            y: 455
            width: 200
            height: 100
            palette.button: "dark gray"
            palette.buttonText: "white"
            text: "Automatic"
            onClicked:
            {
                _Setting.ccAuto(),
                setautolighting()
            }
        }

        Button {
            id: up
            x: 820
            y: 245
            width: 150
            height: 100
            autoRepeat: true
            autoRepeatDelay: 20
            palette.button: "yellow"
            palette.buttonText: "white"
            text: "Up"
            onClicked:
            {
                _Setting.ccFwdButton() 
            }
        }

        Button {
            id: down
            x: 820
            y: 350
            width: 150
            height: 100
            autoRepeat: true
            autoRepeatDelay: 20
            palette.button: "yellow"
            palette.buttonText: "white"
            text: "Down"
            onClicked:
            {
                _Setting.ccRevButton()
            }
        }

        Button {
            id: pause
            x: 820
            y: 455
            width: 150
            height: 100
            autoRepeat: true
            autoRepeatDelay: 20
            palette.button: "orange"
            palette.buttonText: "white"
            text: "Pause"
            onClicked:
            {
                _Setting.ccStopButton()
            }
        }
    }
    
    function readoutchangevalue(value){
        if(value != undefined) {
            motor_readout = parseInt(value)
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

            setstartstoplighting()
        }
    }

    function ethfaultchangevalue(value){
        if(value != undefined){
            ethfault = value
        }
    }

    function setautolighting(){
        auto_on = _Setting.getAutoState()

        if(auto_on){
            auto.palette.button = "dark gray"
            manual.palette.button = "black"
        }

        else{
            auto.palette.button = "black"
            manual.palette.button = "dark gray"
        }
    }

    function setstartstoplighting(){
        if(running){
                start.palette.button = "green"
                stop.palette.button = "dark red"
            }
            else{
                start.palette.button = "dark green"
                stop.palette.button = "red"
            }
    }
     
}