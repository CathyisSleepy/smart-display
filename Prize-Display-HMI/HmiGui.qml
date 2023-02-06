
//import libraries
import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
    // identify the qml
    id: hmiControl
    // define width and height of the app
    width: 1280
    height: 720

    Text {
        id: speed_lable
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
        text: "0"
        font.pixelSize: 45
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
        value: 1
        onValueChanged:
        { 
            _Setting.motorSpeedSet(value)
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
            _Setting.ccStop()
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
            _Setting.ccStart()
        }
    }

    function textset() {
        hmiControl.speed.text = "500";
    }
}