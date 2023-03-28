//import libraries
import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
    // identify the qml
    id: hmiSettings
    // define width and height of the app
    width: 1280
    height: 720

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
            x: 500
            y: 200
            text: "Adjust Motor Speed" 
            font.pixelSize: 45
        }
    
        Text {
            id: speed_perc
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

        Slider {
            id: motor_slider
            x: 450
            y: 360
            width: 456
            height: 62
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