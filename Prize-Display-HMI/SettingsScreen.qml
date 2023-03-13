//import libraries
import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
    // identify the qml
    id: hmiSettings
    // define width and height of the app
    width: 1280
    height: 720

    Text {
            id: speed_label
            x: 500
            y: 200
            text: "Hello World" 
            font.pixelSize: 45
        }
}