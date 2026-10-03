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
        mainWin.flags = isTopWindow ? (mainWin.flags | Qt.WindowStaysOnTopFlag) : (mainWin.flags | ~Qt.WindowStaysOnTopFlag)
    }

    Rectangle
    {
        width: parent.width
        height: parent.height
        color: "#80FFFFFF"
        Row
        {
            anchors.fill: parent
            Rectangle
            {
                id: sideNav
                height: parent.height
                color: "transparent"
                width: isSidebarCollapsed ? 0 : navWidth  // 折叠时宽度为0
                // width: 70
                Behavior on width
                {
                    NumberAnimation
                    {
                        duration: 250
                        easing.type: Easing.InOutCubic
                    }
                }
                // ===== 侧边栏右边缘的 1px 分隔线 =====
                Rectangle
                {
                    width: 1
                    height: parent.height
                    color: "#E0E0E0"              // 浅灰色
                    anchors.right: parent.right
                }
                // ===== 分类按钮列表：用 Repeater 按 sidebarModel 自动生成 =====
                Column
                {
                    anchors.centerIn: parent       // 在侧边栏内垂直居中
                    spacing: 10                   // 各按钮之间间距 10px

                    // Repeater：把数据模型 sidebarModel 的每一项，套用下面的 delegate 模板生成一个 ToolButton
                    Repeater
                    {
                        model: sidebarModel      // 数据来源：C++ 的 CSidebarModel（提供分类名/颜色/是否系统分类）
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
        // ===== 侧边栏折叠/展开按钮（浮层箭头，位于侧边栏右缘）=====
        // ToolButton {
        //     x: 46                           // 贴合展开态侧边栏右缘(70-24)
        //     y: 0
        //     z: 1                            // 浮于 Row 之上，sideNav 折叠时仍可点击
        //     width: 24
        //     height: 24
        //     // 箭头随折叠状态切换：展开显示"←"，折叠显示"→"
        //     text: isSidebarCollapsed ? "→" : "←"
        //     // 点击箭头：切换侧边栏的折叠/展开状态
        //     onClicked: isSidebarCollapsed = !isSidebarCollapsed
        // }
    }
}
