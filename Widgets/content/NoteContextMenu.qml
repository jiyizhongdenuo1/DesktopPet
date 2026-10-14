import QtQuick
import QtQuick.Controls

Menu
{
    id: contextMenu

    enum E_NOTE_ACTION
    {
        E_NOTE_ACTION_TOP_WINDOW,
        E_NOTE_ACTION_CATEGORY,
        E_NOTE_ACTION_REMIND,
        E_NOTE_ACTION_SET_LEVEL,
        E_NOTE_ACTION_PIN_NOTE,
        E_NOTE_ACTION_DELETE
    }
    signal requestAction(int action, int index, var value)
    property int noteIndex: -1

    MenuItem
    {
        text:"窗口置顶"
        onTriggered: contextMenu.requestAction(NoteContextMenu.E_NOTE_ACTION.E_NOTE_ACTION_TOP_WINDOW, contextMenu.noteIndex, 0)
    }
    MenuItem
    {
        text:"分类"
        Menu
        {
            Repeater
            {
                model: sidebarModel
                delegate: MenuItem
                {
                    text: model.name
                    onTriggered: contextMenu.requestAction(NoteContextMenu.E_NOTE_ACTION.E_NOTE_ACTION_CATEGORY, contextMenu.noteIndex, index)
                }
            }
        }
    }
    MenuItem
    {
        text:"设置提醒"
        onTriggered: contextMenu.requestAction(NoteContextMenu.E_NOTE_ACTION.E_NOTE_ACTION_REMIND, contextMenu.noteIndex, 0)

    }
    MenuItem
    {
        text:"设置优先级"
        Menu
        {
            Repeater
            {
                model: ["重要且紧急", "不重要但紧急", "重要但不紧急", "不重要不紧急"]
                delegate: MenuItem
                {
                    text: modelData
                    onTriggered: contextMenu.requestAction(NoteContextMenu.E_NOTE_ACTION.E_NOTE_ACTION_SET_LEVEL, contextMenu.noteIndex, index)
                }
            }
        }
    }
    MenuItem
    {
        text:"置顶便签"
        onTriggered: contextMenu.requestAction(NoteContextMenu.E_NOTE_ACTION.E_NOTE_ACTION_PIN_NOTE, contextMenu.noteIndex, 0)
    }
    MenuItem
    {
        text:"删除"
        onTriggered: contextMenu.requestAction(NoteContextMenu.E_NOTE_ACTION.E_NOTE_ACTION_DELETE, contextMenu.noteIndex, 0)
    }
}
