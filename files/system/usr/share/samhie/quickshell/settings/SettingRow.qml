// Dòng có điều khiển trong Group: nhãn (+ dòng phụ mờ, tuỳ chọn) bên trái,
// điều khiển (Switch, Slider…) đặt làm con, nằm bên phải giữa dòng. Vạch
// ngăn phía trên như InfoRow.
//
//   SettingRow { label: "Mute"; Switch { checked: …; onToggled: on => … } }
import QtQuick
import ".."
import "../colors.js" as Colors
import "rows.js" as Rows

Item {
    id: root

    property string label
    property string sub: ""
    property color subColor: Theme.c.textMuted   // lỗi: Theme.c.danger
    // Bề rộng dành cho nhãn; phần còn lại cho điều khiển.
    property real labelWidth: width * 0.4
    default property alias control: slot.data
    readonly property bool first: Rows.isFirst(root)

    width: parent.width
    height: sub === "" ? 48 : 60

    Rectangle {
        visible: !root.first
        x: 16
        width: parent.width - 32
        height: 1
        color: Theme.c.subSecondary
    }

    Column {
        x: 16
        width: root.labelWidth - 16
        anchors.verticalCenter: parent.verticalCenter
        Text {
            width: parent.width
            elide: Text.ElideRight
            text: root.label
            color: Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 26
        }
        Text {
            visible: root.sub !== ""
            width: parent.width
            elide: Text.ElideRight
            text: root.sub
            color: root.subColor
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 22
        }
    }

    // Điều khiển: căn phải, giữa dòng.
    Item {
        id: slot
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        width: childrenRect.width
        height: childrenRect.height
    }
}
