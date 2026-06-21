
import QtQuick
import QtQuick.Window
import QtQuick.Controls


Window {
    width: 460
    height: 1020
    visible: true
    title: qsTr("C++ Model ListView")
    color: "transparent"
    // flags: Qt.FramelessWindowHint
    Rectangle
    {
        width: parent.width
        height: parent.height
        color: "#80FFFFFF"
        ListOfNotes
        {
            anchors.fill: parent
            // background: Rectangle { color: "transparent" }
        }
    }

}
