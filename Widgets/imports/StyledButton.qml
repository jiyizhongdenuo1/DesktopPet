import QtQuick
import QtQuick.Controls

ToolButton
{
    id: root

    property string bgColor: "transparent"
    property string hoverColor: "#E8F4FD"
    property string borderColor: "#1976D2"
    property int borderWidth: 0
    property int radius: 6
    property real scaleDown: 0.9
    property int animDuration: 150
    property string tipText: ""
    property int tipDelay: 300

    hoverEnabled: true

    scale: pressed ? scaleDown : 1

    Behavior on scale
    {
        NumberAnimation
        {
            duration: root.animDuration
            easing.type: Easing.InOutQuad
        }
    }

    background: Rectangle
    {
        radius: root.radius
        color: parent.hovered ? root.hoverColor : root.bgColor
        border.width: root.borderWidth
        border.color: root.borderColor

        Behavior on color
        {
            ColorAnimation
            {
                duration: root.animDuration
            }
        }
    }

    ToolTip.visible: hovered
    ToolTip.text: root.tipText.length > 0 ? root.tipText : root.text
    ToolTip.delay: root.tipDelay
}