import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: root

    signal clicked()

    onClicked: root.clicked()
}