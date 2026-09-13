import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window
{
    width: 640
    height: 1020
    visible: true
    title: qsTr("C++ Model ListView")

    Column
    {
        anchors.fill: parent
        spacing: 10

        Button
        {
            text: "通过C++添加黄色"
            onClicked:
            {
                colorModel.addColor("Yellow", "yellow")
            }
        }

        ListView
        {
            width: parent.width
            height: parent.height - 50
            model: colorModel

            delegate: Rectangle
            {
                height: 50
                width: parent.width
                color: "lightgray"
                border.color: "white"

                Row
                {
                    anchors.centerIn: parent
                    spacing: 20

                    Text
                    {
                        text: name
                        font.pixelSize: 18
                        color: "black"
                    }

                    Rectangle
                    {
                        width: 30
                        height: 30
                        color: model.color
                        border.color: "black"
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        console.log("点击了:", name)
                    }
                }
            }
        }
    }
}