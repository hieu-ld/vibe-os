// Dòng trong Group: nhãn bên trái, giá trị bên phải (chữ mờ). Vạch ngăn
// `sub-secondary` phía trên, lùi vào như macOS — trừ dòng đầu của nhóm.
import QtQuick
import ".."
import "../colors.js" as Colors

Item {
    id: root

    property string label
    property string value
    readonly property bool first: parent && parent.children[0] === root

    width: parent.width
    height: 48

    Rectangle {
        visible: !root.first
        x: 16
        width: parent.width - 32
        height: 1
        color: Theme.c.subSecondary
    }

    Text {
        x: 16
        width: parent.width / 2 - 16
        height: parent.height
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        text: root.label
        color: Theme.c.text
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 26
    }
    Text {
        anchors.right: parent.right
        anchors.rightMargin: 16
        width: parent.width / 2 - 16
        height: parent.height
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        elide: Text.ElideMiddle
        text: root.value
        color: Theme.c.textMuted
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 26
    }
}
