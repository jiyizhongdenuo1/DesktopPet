import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15


Dialog {
    id: inputDialog
    title: "输入内容"
    modal: true
    focus: true

    standardButtons: Dialog.Ok | Dialog.Cancel

    property bool isEdit: false
    property string dialogTitle: "添加便签"
    property string initialText: ""
    property string inputText: textInput.text

    onOpened:
    {
        if (isEdit)
        {
            title = dialogTitle
            textInput.text = initialText
            textInput.focus = true
        }
    }

    ColumnLayout
    {
        spacing: 10
        width: inputDialog.availableWidth

        Text { text: "请输入便签内容：" }

        TextField
        {
            id: textInput
            Layout.fillWidth: true
            placeholderText: "在这里输入..."
            focus: true
        }
    }
}