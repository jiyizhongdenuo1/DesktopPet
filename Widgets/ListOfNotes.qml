import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts
import content 1.0

Item
{
    id: mainListView
    height: 1020
    property int currentClickIndex: -1
    property bool isAddNote: false

    function addNote()
    {
        if (noteList.footerItem &&
            noteList.footerItem.newNoteInput &&
            noteList.footerItem.newNoteInput.text.length > 0)
        {
            noteModel.AddNote(noteList.footerItem.newNoteInput.text, new Date())
            noteList.footerItem.newNoteInput.text = ""
            addNoteTime.stop()
        }
    }

    function confirmEdit()
    {
        if (currentClickIndex >= 0)
        {
            var editDelegate = noteList.itemAtIndex(currentClickIndex)
            if (!editDelegate || !editDelegate.editTextField)
            {
                currentClickIndex = -1
                return
            }

            var newNoteContent = editDelegate.editTextField.text
            var perNoteContent = editDelegate.perNoteContent
            if (newNoteContent.length > 0 && newNoteContent !== perNoteContent)
            {
                noteModel.UpdateNoteContent(currentClickIndex, newNoteContent)
            }
            currentClickIndex = -1
        }
    }

    function cancelEdit()
    {
        currentClickIndex = -1
    }

    Column
    {
        anchors.fill: parent
        spacing: 5

        ListView
        {
            id: noteList
            width: parent.width
            height: parent.height - 50
            model: noteModel

            delegate: Rectangle
            {
                property string perNoteContent: ""
                width: parent.width
                height: 50
                color: mouseArea.containsMouse ? "#f0f0f0" : "#80ffffff"
                border.color: (currentClickIndex == index) ? "#4a90d9" : "#eee"
                border.width: (currentClickIndex == index) ? 2 : 1

                MouseArea
                {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked:
                    {
                        if (currentClickIndex != index)
                        {
                            if (currentClickIndex >= 0)
                            {
                                confirmEdit()
                            }
                            currentClickIndex = index
                            perNoteContent = noteContent
                        }
                        else
                        {
                            if (editTextField)
                            {
                                editTextField.forceActiveFocus()
                            }
                        }
                    }

                    onDoubleClicked:
                    {
                        currentClickIndex = index
                        inputDialog.dialogTitle = "Edit Note"
                        inputDialog.isEdit = true
                        inputDialog.initialText = noteContent
                        inputDialog.open()
                    }
                }

                RowLayout
                {
                    anchors.fill: parent
                    anchors.margins: 10
                    visible: currentClickIndex != index

                    Text
                    {
                        text: writeTime
                        font.pixelSize: 12
                        color: "gray"
                        Layout.preferredWidth: 120
                    }

                    Text
                    {
                        text: noteContent
                        font.pixelSize: 16
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }

                RowLayout
                {
                    anchors.fill: parent
                    anchors.margins: 10
                    visible: currentClickIndex == index

                    Text
                    {
                        text: writeTime
                        font.pixelSize: 12
                        color: "gray"
                        Layout.preferredWidth: 120
                    }

                    TextField
                    {
                        id: editTextField
                        font.pixelSize: 16
                        Layout.fillWidth: true
                        focus: true

                        background: Rectangle
                        {
                            color: "transparent"
                            border.width: 0
                        }

                        Keys.onEnterPressed:function(event)
                        {
                            if (event.modifiers & Qt.ControlModifier)
                            {
                                event.accepted = false
                            }
                            else
                            {
                                confirmEdit()
                            }
                            console.log("点击列表区域")
                        }

                        Keys.onReturnPressed:function(event)
                        {
                            if (event.modifiers & Qt.ControlModifier)
                            {
                                event.accepted = false
                            }
                            else
                            {
                                confirmEdit()
                            }
                        }

                        Keys.onEscapePressed:function()
                        {
                            cancelEdit()
                        }

                        Component.onCompleted:
                        {
                            text = noteContent
                            editTextField.forceActiveFocus()
                        }
                    }
                }
            }

            footer: Rectangle
            {
                id: footerItem
                width: parent.width
                height: 50
                color: "#80f8f8f8"
                border.color: "#eee"

                RowLayout
                {
                    anchors.fill: parent

                    TextField
                    {
                        id: newNoteInput
                        color: "#000000"
                        placeholderText: "Add a new note..."
                        Layout.fillWidth: true
                        font.pixelSize: 16

                        background: Rectangle
                        {
                            color: "transparent"
                            border.width: 0
                        }

                        Keys.onEnterPressed:
                        {
                            addNote()
                        }

                        onTextChanged:
                        {
                            if (newNoteInput.text.length > 0)
                            {
                                addNoteTime.restart()
                            }
                            else
                            {
                                addNoteTime.stop()
                            }
                        }

                        onFocusChanged:
                        {
                            if (!newNoteInput.focus)
                            {
                                addNote()
                            }
                        }
                    }
                }
            }

            Timer
            {
                id: addNoteTime
                interval: 2000
                repeat: false

                onTriggered:
                {
                    addNote()
                }
            }
        }

        Rectangle
        {
            width: parent.width
            height: 50
            color: "#80f8f8f8"

            Button
            {
                text: "Add Note"
                anchors.centerIn: parent

                onClicked:
                {
                    if (currentClickIndex >= 0)
                    {
                        confirmEdit()
                    }

                    currentClickIndex = -1
                    inputDialog.title = "Add Note"
                    inputDialog.isEdit = false
                    inputDialog.open()
                }
            }
        }
    }

    PopInputNoteDia
    {
        id: inputDialog
        width: 400
        height: 200
        anchors.centerIn: parent

        onRejected:
        {
            currentClickIndex = -1
        }

        onAccepted:
        {
            if (inputDialog.inputText.length > 0)
            {
                if (currentClickIndex === -1)
                {
                    noteModel.AddNote(inputDialog.inputText, new Date())
                }
                else
                {
                    noteModel.UpdateNoteContent(currentClickIndex, inputDialog.inputText)
                }
            }
            currentClickIndex = -1
        }
    }

    MouseArea
    {
        anchors.fill: parent
        z: -1

        onClicked:
        {
            if (currentClickIndex >= 0)
            {
                confirmEdit()
            }
        }
    }
}