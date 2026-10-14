import QtQuick
import QtQuick.Window
import QtQuick.Controls
import components

Window
{
    id:mainWin
    width:  configModel ? configModel.windowWidth  : 500
    height: configModel ? configModel.windowHeight : 1020
    visible: true
    title: qsTr("点滴便签")
    color: "transparent"
    property int navWidth: 70
    property bool isSidebarCollapsed: false
    property bool isTopWindow: false

    function toggleTopWindow()
    {
        isTopWindow = !isTopWindow
        mainWin.flags = isTopWindow ? (mainWin.flags | Qt.WindowStaysOnTopFlag) : (mainWin.flags & ~Qt.WindowStaysOnTopFlag)
    }

    Rectangle
    {
        width: parent.width
        height: parent.height
        radius: 12
        clip: true
        color: "#80FFFFFF"

        Row
        {
            anchors.fill: parent
            Rectangle
            {
                id: sideNav
                height: parent.height
                color: "transparent"
                width: isSidebarCollapsed ? 0 : navWidth
                Behavior on width
                {
                    NumberAnimation
                    {
                        duration: 250
                        easing.type: Easing.InOutCubic
                    }
                }
                Rectangle
                {
                    width: 1
                    height: parent.height
                    color: "#E0E0E0"
                    anchors.right: parent.right
                }
                Column
                {
                    anchors.centerIn: parent
                    spacing: 10

                    Repeater
                    {
                        model: sidebarModel
                        delegate: StyledButton
                        {
                            width: 50
                            height: 40
                            font.pixelSize: 14
                            text: model.name
                            tipText: model.name
                            borderWidth: index === sidebarModel.currentSelection ? 2 : 0
                            borderColor: model.color
                            onClicked: sidebarModel.currentSelection = index
                        }
                    }
                }
            }
            Rectangle
            {
                id: mainContent
                height: parent.height
                color: "transparent"
                width: parent.width - sideNav.width
                ListOfNotes
                {
                    anchors.fill: parent
                    onRequestToggleTopWin: mainWin.toggleTopWindow()
                }
            }
        }
    }
}