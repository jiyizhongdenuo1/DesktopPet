import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window
{
    width: 500
    height: 1020
    visible: true
    title: qsTr("C++ Model ListView")
    color: "transparent"
    property int navWidth: 70
    property bool isSidebarCollapsed: false

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
                Behavior on width {
                    NumberAnimation {
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

                        // 每个分类项渲染成一个按钮（delegate 是每项的模板）
                        delegate: ToolButton
                        {
                            width: 50
                            height: 40
                            text: model.name     // 按钮文字 = 分类名（来自 CSidebarModel）
                            font.pixelSize: 14
                            hoverEnabled: true    // 开启鼠标悬停状态
                            scale: pressed ? 0.9 : 1   // 按下时缩到 0.9，做出按压反馈
                            // 缩放变化用 150ms 缓动动画，使按下/松开更顺滑
                            Behavior on scale
                            {
                                NumberAnimation
                                {
                                    duration: 150
                                    easing.type: Easing.InOutQuad
                                }
                            }

                            // 按钮背景样式
                            background: Rectangle
                            {
                                radius: 6        // 圆角 6px
                                // 悬停时浅蓝底，否则透明
                                color: parent.hovered ? "#E8F4FD" : "transparent"
                                // 当前选中的分类描边 2px，其余不描边（0px）
                                border.width: index === sidebarModel.currentSelection ? 2 : 0
                                border.color: model.color   // 描边颜色用分类自身的颜色

                                // 背景色变化用 150ms 颜色动画过渡
                                Behavior on color
                                {
                                    ColorAnimation
                                    {
                                        duration: 150
                                    }
                                }
                            }

                            // 点击某个分类：打印日志，并把"当前选中项"设为该分类的索引
                            onClicked:
                            {
                                console.log("Clicked: " + model.name + " (index: " + index + ")")
                                sidebarModel.currentSelection = index
                            }

                            // 鼠标悬停满 300ms 后弹出的分类名提示气泡
                            ToolTip
                            {
                                text: model.name
                                visible: parent.hovered
                                delay: 300
                            }
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