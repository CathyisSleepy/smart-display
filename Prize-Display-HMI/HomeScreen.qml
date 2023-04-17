
//import libraries
import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
    // identify the qml
    id: hmiControl
    // define width and height of the app
    width: 800
    height: 480
    focus: true
    Keys.onEscapePressed:_Setting.closeWindow()

    //define all of our varibles
    property int motor_readout: 0
    property bool mfault: false
    property bool estopped: false
    property bool not_running: true
    property bool running: false
    property bool ethfault: false
    property bool auto_on: true

    //attach functions to the incoming streamed variables
    Component.onCompleted: {
        _Streaming.FaultSignal.connect(mfaultchangevalue)
        _Streaming.EstopSignal.connect(estoppedchangevalue)
        _Streaming.RunSignal.connect(runningchangevalue)
        _Streaming.EthSignal.connect(ethfaultchangevalue)
        _Streaming.MotorSignal.connect(readoutchangevalue)
        setautolighting()
        setstartstoplighting()
        motor_readout = parseInt(_Setting.motorSpeedGet() / 255 * 100)
        focus = true
    }

    //grab motor speed every second this forces the gui to update
    Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: {
            _Setting.motorSpeedGet()
        }
    }

    //background fill
    Rectangle{
        anchors.fill: parent
        color: "light gray"
        focus: true
        
        //motor setpoint readout
        Text {
            id: speed_label
            x: 250
            y: 100
            text: "Motorspeed Setpoint" 
            font.pixelSize: 20
        }
    
        Text {
            id: speed_perc
            x: 250
            y: 123
            visible: true
            focus: true
            text: motor_readout
            font.pixelSize: 23
            Text {
                id: perc
                anchors.left: parent.right
                anchors.leftMargin: 3
                text: "%"
                font.pixelSize: 15
            }
        }

        //warnings that only pop up if relevant (most of the time)
        Text {
            id: eth_fault_warning
            focus: true
            visible: ethfault
            x: 500
            y: 50
            text: "CONNECTION FAULT"
            font.pixelSize: 15
            color: "red"
        }

        Text {
            id: fault_warning
            focus: true
            visible: mfault
            x: 500
            y: 65
            text: "MOTOR FAULT"
            font.pixelSize: 15
            color: "red"
        }

        Text {
            id: estop_warning
            visible: estopped
            x: 500
            y: 80
            text: "ESTOP PRESSED"
            font.pixelSize: 15
            color: "red"
        }

        Text {
            id: running_lable
            visible: running
            focus: true
            x: 100
            y: 110
            text: "Machine Running"
            font.pixelSize: 15
            color: "green"
        }

        //here are our buttons to stop, start, toggle manual/auto, and to interact with the clearcore
        Button {
            id: stop
            focus: true
            x: 400
            y: 300
            width: 100
            height: 50
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
            x: 100
            y: 300
            width: 100
            height: 50
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
            x: 500
            y: 300
            width: 100
            height: 50
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
            x: 500
            y: 160
            width: 100
            height: 50
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
            x: 500
            y: 226
            width: 100
            height: 50
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
            x: 410
            y: 122
            width: 75
            height: 50
            autoRepeat: true
            autoRepeatInterval: 100
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
            x: 410
            y: 175
            width: 75
            height: 50
            autoRepeat: true
            autoRepeatInterval: 100
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
            x: 410
            y: 228
            width: 75
            height: 50
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
    
    //here are our attachted functions to update gui variables
    function readoutchangevalue(value){
        if(value != undefined) {
            motor_readout = parseInt(value / 255 * 100)
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

    //these are functions to update the gui state
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
        running = _Setting.getRunState()

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