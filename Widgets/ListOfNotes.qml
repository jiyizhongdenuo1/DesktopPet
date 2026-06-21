import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts
import content 1.0

Item {
    id: mainListView
    width: 640
    height: 1020
    property int currentClickIndex: -1

    Column {
        anchors.fill: parent
        spacing: 5

        ListView {
            id: noteList
            width: parent.width
            height: parent.height - 50
            model: noteModel

            delegate: Rectangle {
                width: parent.width
                height: 50
                color: mouseArea.containsMouse ? "#f0f0f0" : "#80ffffff"
                border.color: "#eee"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10

                    Text {
                        text: writeTime
                        font.pixelSize: 12
                        color: "gray"
                        Layout.preferredWidth: 120
                    }

                    Text {
                        text: noteContent
                        font.pixelSize: 16
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        currentClickIndex = index
                        inputDialog.title = "Edit Note"
                        inputDialog.isEdit = true
                        inputDialog.textInput.text = noteContent
                        inputDialog.open()
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 50
            color: "#80f8f8f8"

            Button {
                text: "Add Note"
                anchors.centerIn: parent
                onClicked: {
                    currentClickIndex = -1
                    inputDialog.title = "Add Note"
                    inputDialog.isEdit = false
                    inputDialog.open()
                }
            }
        }
    }

    PopInputNoteDia {
        id: inputDialog
        width: 400
        height: 200
        anchors.centerIn: parent

        onRejected: {
            currentClickIndex = -1
        }

        onAccepted: {
            if (inputDialog.inputText.length > 0) {
                if (currentClickIndex === -1) {
                    // 添加新笔记
                    noteModel.AddNote(inputDialog.inputText, new Date())
                } else {
                    // 编辑现有笔记
                    noteModel.UpdateNoteContent(currentClickIndex, inputDialog.inputText)
                }
            }
            currentClickIndex = -1
        }
    }
}