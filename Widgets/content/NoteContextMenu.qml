import QtQuick
import QtQuick.Controls

Menu
{
    id:contextMenu
    signal requestDelete(int index)

    property int noteIndex: -1
    MenuItem
    {
        text:"界面顶层显示"
        onTriggered: contextMenu.requestDelete(contextMenu.noteIndex)
    }
    MenuItem
    {
        text:"分类"
        onTriggered: contextMenu.requestDelete(contextMenu.noteIndex)
    }
    MenuItem
    {
        text:"设置提醒"
        onTriggered: contextMenu.requestDelete(contextMenu.noteIndex)

    }
    MenuItem
    {
        text:"设置优先级"
        onTriggered: contextMenu.requestDelete(contextMenu.noteIndex)

    }
    MenuItem
    {
        text:"单条等级分类"
        onTriggered: contextMenu.requestDelete(contextMenu.noteIndex)

    }
    MenuItem
    {
        text:"设置头条"
        onTriggered: contextMenu.requestDelete(contextMenu.noteIndex)

    }
}
