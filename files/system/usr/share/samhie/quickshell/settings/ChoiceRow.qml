// Dòng chọn một-trong-nhiều trong Group (loa, micro…): icon + tên, ✓ ở
// dòng đang chọn. Trỏ chuột = lớp chữ mờ 6% (nền nhóm `secondary` bản tối
// trùng màu `hover` nên không dùng được `hover`). Vạch ngăn như InfoRow.
import QtQuick
import "../bar"
import ".."
import "../colors.js" as Colors
import "rows.js" as Rows

Item {
    id: root

    property string label
    property string iconName: ""
    property string iconCategory: "devices"
    property bool selected: false
    signal clicked()
    readonly property bool first: Rows.isFirst(root)

    width: parent.width
    height: 48

    // Ô trỏ: lùi 4px khỏi mép thẻ, bo theo thẻ.
    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        radius: 8
        color: Qt.alpha(Theme.c.text, mouse.containsMouse ? 0.06 : 0)
    }
    Rectangle {
        visible: !root.first
        x: 16
        width: parent.width - 32
        height: 1
        color: Theme.c.subSecondary
    }

    SamhieIcon {
        visible: root.iconName !== ""
        x: 16
        anchors.verticalCenter: parent.verticalCenter
        category: root.iconCategory
        iconName: root.iconName
        size: 20
        color: Theme.c.text
    }
    Text {
        x: root.iconName !== "" ? 48 : 16
        width: parent.width - x - 48
        height: parent.height
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        text: root.label
        color: Theme.c.text
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 26
    }
    SamhieIcon {
        visible: root.selected
        anchors.right: parent.right
        anchors.rightMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        iconName: "dialog-ok"
        size: 16
        color: Theme.c.accentText
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
