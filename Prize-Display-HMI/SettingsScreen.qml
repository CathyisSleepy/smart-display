//import libraries
import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
    // identify the qml
    id: hmiSettings
    // define width and height of the app
    width: 800
    height: 480

    property int mspeed: 0

    Component.onCompleted: {
        mspeedchangevalue(_Setting.motorSpeedGet())
        focus = true
    }

    Rectangle{
        anchors.fill: parent
        color: "light gray"
        focus: true

        Text {
            id: speed_label
            x: 250
            y: 100
            text: "Adjust Motor Speed" 
            font.pixelSize: 25
        }
    
        Text {
            id: speed_perc
            x: 250
            y: 125
            visible: true
            text: mspeed
            font.pixelSize: 45
            Text {
                id: perc
                anchors.left: parent.right
                anchors.leftMargin: 3
                text: "%"
                font.pixelSize: 15
            }
        }

        Slider {
            id: motor_slider
            x: 225
            y: 180
            width: 220
            height: 32
            scale: 1.7
            stepSize: 1
            to: 255
            value: _Setting.motorSpeedGet()
            onValueChanged:
            { 
                _Setting.motorSpeedSet(value),
                mspeedchangevalue(value)
            }
        }  
    }

    function mspeedchangevalue(value){
        if(value != undefined) {
            mspeed = parseInt(value / 255 * 100)
        }
    }
}