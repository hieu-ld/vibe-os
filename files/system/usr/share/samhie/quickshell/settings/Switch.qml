// Công tắc — cùng hình với công tắc trang Wi-Fi của quick settings: bật =
// nền `selection`, núm `on-selection`; tắt = viền mờ, núm mờ.
// `checked` do nơi dùng giữ; bấm chỉ báo `toggled(giá trị mới)`.
import QtQuick
import ".."

Rectangle {
    id: root

    property bool checked: false
    signal toggled(bool on)

    width: 40
    height: 22
    radius: 11
    color: checked ? Theme.c.selection : "transparent"
    border.width: checked ? 0 : 1
    border.color: Theme.c.textMuted

    Rectangle {
        width: root.checked ? 14 : 12
        height: width
        radius: width / 2
        anchors.verticalCenter: parent.verticalCenter
        x: root.checked ? parent.width - width - 4 : 5
        color: root.checked ? Theme.c.onSelection : Theme.c.textMuted
        Behavior on x { NumberAnimation { duration: 120 } }
    }
    MouseArea {
        anchors.fill: parent
        anchors.margins: -4
        onClicked: root.toggled(!root.checked)
    }
}
