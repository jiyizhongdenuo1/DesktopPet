import QtQuick
import QtQuick.Controls

Menu
{
    id:contextMenu

    enum Action { TopWindow, Category, Remind, SetLevel, SetTimeSpan, PinNote, Delete }
    signal requestAction(int action, int index, int value)
    property int noteIndex: -1

    MenuItem
    {
        text:"窗口置顶"
        onTriggered: contextMenu.requestDelete(contextMenu.Action.TopWindow, contextMenu.noteIndex, 0)
    }
    MenuItem
    {
        text:"分类"
        onTriggered: contextMenu.requestDelete(contextMenu.Action.Category, contextMenu.noteIndex, 0)
    }
    MenuItem
    {
        text:"设置提醒"
        onTriggered: contextMenu.requestDelete(contextMenu.Action.Remind, contextMenu.noteIndex, 0)

    }
    MenuItem
    {
        text:"设置优先级"
        Menu
        {
            MenuItem
            {
                text:"重要且紧急"
                onTriggered: contextMenu.requestDelete(contextMenu.Action.SetLevel, contextMenu.noteIndex, 0)
            }
            MenuItem
            {
                text:"不重要但紧急"
                onTriggered: contextMenu.requestDelete(contextMenu.Action.SetLevel, contextMenu.noteIndex, 1)
            }
            MenuItem
            {
                text:"重要但不紧急"
                onTriggered: contextMenu.requestDelete(contextMenu.Action.SetLevel, contextMenu.noteIndex, 2)
            }
            MenuItem
            {
                text:"不重要不紧急"
                onTriggered: contextMenu.requestDelete(contextMenu.Action.SetLevel, contextMenu.noteIndex, 3)
            }
        }
    }
    MenuItem
    {
        text:"单条等级分类"
        onTriggered: contextMenu.requestDelete(contextMenu.Action.SetTimeSpan, contextMenu.noteIndex, 0)

    }
    MenuItem
    {
        text:"置顶便签"
        onTriggered: contextMenu.requestDelete(contextMenu.Action.PinNote, contextMenu.noteIndex, 0)

    }
    MenuItem
    {
        text:"删除"
        onTriggered: contextMenu.requestDelete(contextMenu.Action.Delete, contextMenu.noteIndex, 0)
    }
}
