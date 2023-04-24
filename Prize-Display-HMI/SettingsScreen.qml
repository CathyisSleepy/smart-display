//import libraries
import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
    // identify the qml
    id: hmiSettings
    // define width and height of the app
    width: 800
    height: 480

    //initialize and attatch variables
    property int mspeed: 0
    property int wtime: 0

    Component.onCompleted: {
        mspeedchangevalue(_Setting.motorSpeedGet())
        wtimechangevalue(_Setting.waitTimeGet())
        focus = true
    }

    //this is the settings page with a slider and a changing tag to show speed percent
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
            font.pixelSize: 25
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

        Text {
            id: time_label
            x: 250
            y: 210
            text: "Adjust Wait Time to Return to Forward" 
            font.pixelSize: 25
        }
    
        Text {
            id: time_read
            x: 250
            y: 245
            visible: true
            text: wtime
            font.pixelSize: 25
            Text {
                id: secs
                anchors.left: parent.right
                anchors.leftMargin: 3
                text: " seconds"
                font.pixelSize: 15
            }
        }

        Slider {
            id: time_slider
            x: 225
            y: 280
            width: 220
            height: 32
            scale: 1.7
            stepSize: 1
            to: 255
            value: _Setting.waitTimeGet()
            onValueChanged:
            { 
                _Setting.waitTimeSet(value),
                wtimechangevalue(value)
            }
        } 
    }

    //function to update mspeed
    function mspeedchangevalue(value){
        if(value != undefined) {
            mspeed = parseInt(value / 255 * 100)
        }
    }

    //function to update wtime
    function wtimechangevalue(value){
        if(value != undefined) {
            wtime = parseInt(5 + (60 - 5) * (value / 255))
        }
    }
}