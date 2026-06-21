import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    width: 640
    height: 1020
    visible: true
    title: qsTr("C++ Model ListView")

    Column {
        anchors.fill: parent
        spacing: 10

        // 按钮用于测试 C++ 的添加功能
        Button {
            text: "通过C++添加黄色"
            onClicked: {
                // 直接调用 C++ 中 Q_INVOKABLE 标记的函数
                colorModel.addColor("Yellow", "yellow")
            }
        }

        // ListView 核心代码
        ListView {
            width: parent.width
            height: parent.height - 50
            // 4. 设置数据源为 C++ 暴露的模型
            model: colorModel

            // 5. 定义每一项的外观 (Delegate)
            delegate: Rectangle {
                height: 50
                width: parent.width
                color: "lightgray"
                border.color: "white"

                // 使用 Row 布局让文字和颜色块并排
                Row {
                    anchors.centerIn: parent
                    spacing: 20

                    // 显示名字
                    Text {
                        text: name // 对应 C++ roleNames 中的 "name"
                        font.pixelSize: 18
                        color: "black"
                    }

                    // 显示颜色块
                    Rectangle {
                        width: 30; height: 30
                        color: model.color // 对应 C++ roleNames 中的 "color"
                        border.color: "black"
                    }
                }

                // 点击效果
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("点击了:", name)
                    }
                }
            }
        }
    }
}
