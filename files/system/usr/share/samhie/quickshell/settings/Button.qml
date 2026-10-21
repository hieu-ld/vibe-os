// Nút chữ trong dòng của Group — cùng hình với nút của trang Wi-Fi quick
// settings (TextButton trong WifiPage.qml): chính = nền `selection`, phụ =
// lớp màu chữ mờ (8%, trỏ 14%).
import QtQuick
import ".."
import "../colors.js" as Colors

Rectangle {
    id: root

    property string label
    property bool primary: false
    signal clicked()

    width: text.implicitWidth + 28
    height: 32
    radius: 6
    color: primary ? Theme.c.selection
        : Qt.alpha(Theme.c.text, mouse.containsMouse && enabled ? 0.14 : 0.08)
    opacity: enabled ? 1 : 0.5

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        visible: root.primary
        color: Theme.c.onSelection
        opacity: !root.enabled ? 0 : mouse.pressed ? 0.16 : mouse.containsMouse ? 0.09 : 0
    }
    Text {
        id: text
        anchors.centerIn: parent
        text: root.label
        color: root.primary ? Theme.c.onSelection : Theme.c.text
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 24
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: if (root.enabled) root.clicked()
    }
}
