import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Dialog
{
    id: tipDia

    title: "设置提醒"
    modal: true
    focus: true
    width: 420
    standardButtons: Dialog.Ok | Dialog.Cancel

    // 由调用方在 open() 之前设置，用于区分操作的是哪条便签
    property int noteIndex: -1

    // 确认后回传：便签索引、提醒时间、重复类型、自定义间隔（分钟）
    signal remindConfirmed(int index, var remindTime, int repeatType, int interval)

    onOpened:
    {
        var now = new Date()
        yearSpin.value  = now.getFullYear()
        monthSpin.value = now.getMonth() + 1
        daySpin.value   = now.getDate()
        hourSpin.value  = now.getHours()
        minuteSpin.value = now.getMinutes()
        repeatCombo.currentIndex = 0
        intervalSpin.value = 1
    }

    onAccepted:
    {
        var remindTime = new Date(yearSpin.value, monthSpin.value - 1, daySpin.value,
                                  hourSpin.value, minuteSpin.value, 0, 0)

        remindConfirmed(noteIndex, remindTime, repeatCombo.currentIndex, intervalSpin.value)
    }

    background: Rectangle
    {
        color: "#f8f8f8"
        radius: 6
    }

    ColumnLayout
    {
        width: tipDia.availableWidth
        spacing: 12

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 8

            Label { text: "日期" }

            SpinBox
            {
                id: yearSpin
                from: 2000; to: 2100
                editable: true
                Layout.fillWidth: true
            }

            Label { text: "年" }

            SpinBox
            {
                id: monthSpin
                from: 1; to: 12
                editable: true
                Layout.fillWidth: true
            }

            Label { text: "月" }

            SpinBox
            {
                id: daySpin
                from: 1; to: 31
                editable: true
                Layout.fillWidth: true
            }

            Label { text: "日" }
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 8

            Label
            {
                text: "提醒时间"
            }

            SpinBox
            {
                id: hourSpin
                from: 0
                to: 23
                editable: true
                Layout.fillWidth: true
            }

            Label
            {
                text: ":"
            }

            SpinBox
            {
                id: minuteSpin
                from: 0
                to: 59
                editable: true
                Layout.fillWidth: true
            }
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 8

            Label
            {
                text: "提醒间隔"
            }

            ComboBox
            {
                id: repeatCombo
                Layout.fillWidth: true
                model: ["单次", "每天", "每周", "每月", "自定义"]
            }
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 8
            visible: repeatCombo.currentIndex === 4

            Label
            {
                text: "间隔（天）"
            }

            SpinBox
            {
                id: intervalSpin
                from: 1
                to: 365
                editable: true
                Layout.fillWidth: true
            }
        }
    }
}