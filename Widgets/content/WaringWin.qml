import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window
{
    id: warningWin

    width: 320
    height: 120
    visible: false

    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus
    color: "transparent"

    property string strWaring: ""
    property int margin: 16
    property int autoCloseMs: 5000
    property int slideDuration: 260

    // 桌面可用区域右下角的目标位置（避开任务栏）
    readonly property real shownX: Screen.availableWidth - width - margin
    readonly property real shownY: Screen.availableHeight - height - margin

    // 屏幕下缘之外，滑入的起点与滑出的终点
    readonly property real hiddenY: Screen.availableHeight

    x: shownX

    Component.onCompleted: y = hiddenY

    /** 从右下角升起并显示提示文本；不传文本则沿用 strWaring */
    function open(strText)
    {
        if (strText !== undefined && strText !== null)
        {
            strWaring = strText
        }

        slideOut.stop()
        y = hiddenY
        visible = true
        slideIn.start()
    }

    /** 向下滑出并隐藏 */
    function close()
    {
        slideIn.stop()
        slideOut.start()
    }

    NumberAnimation
    {
        id: slideIn
        target: warningWin
        property: "y"
        to: warningWin.shownY
        duration: warningWin.slideDuration
        easing.type: Easing.OutCubic
        onFinished: autoCloseTimer.restart()
    }

    NumberAnimation
    {
        id: slideOut
        target: warningWin
        property: "y"
        to: warningWin.hiddenY
        duration: warningWin.slideDuration
        easing.type: Easing.InCubic
        onFinished: warningWin.visible = false
    }

    Timer
    {
        id: autoCloseTimer
        interval: warningWin.autoCloseMs
        onTriggered: warningWin.close()
    }

    Rectangle
    {
        anchors.fill: parent
        radius: 8
        color: "#f8f8f8"
        border.color: "#d0d0d0"

        Label
        {
            anchors.fill: parent
            anchors.margins: 16
            text: warningWin.strWaring
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
        }

        MouseArea
        {
            anchors.fill: parent
            onClicked: warningWin.close()
        }
    }
}
