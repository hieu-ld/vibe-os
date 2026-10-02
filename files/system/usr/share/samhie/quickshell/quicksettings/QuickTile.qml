// Một ô bật/tắt của quick settings — như bản 1.0 (org.samhie.quicksettings,
// Tile.qml): ô TẮT là một lớp màu chữ rất mờ (nổi lên khỏi nền popup đúng
// một bậc), ô BẬT nền `selection` + chữ `on-selection`. Không lấy cyan
// #00C0E8 làm nền: chữ trắng trên nó chỉ 2,16:1 (đo ở bản 1.0).
//
//   [icon]  Tên ô
//           dòng phụ (tên mạng, thiết bị…)
import QtQuick
import "../bar"
import "../colors.js" as Colors

Item {
    id: root

    property string label
    property string sub
    property string iconName
    property string iconCategory: "status"
    property bool active: false

    signal clicked()

    height: 56

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: root.active ? Colors.dark.selection : Colors.dark.text
        opacity: root.active ? 1 : mouse.pressed ? 0.18 : mouse.containsMouse ? 0.12 : 0.07
        Behavior on opacity { NumberAnimation { duration: 100 } }
    }
    // Ô đang bật: hover là một lớp sáng mờ phủ lên (hạ độ đục nền màu thì ô
    // trông như đang tắt dần).
    Rectangle {
        anchors.fill: parent
        radius: 8
        visible: root.active
        color: Colors.dark.onSelection
        opacity: mouse.pressed ? 0.16 : mouse.containsMouse ? 0.09 : 0
    }

    SamhieIcon {
        x: 12
        anchors.verticalCenter: parent.verticalCenter
        category: root.iconCategory
        iconName: root.iconName
        size: 20
        color: root.active ? Colors.dark.onSelection : Colors.dark.text
    }

    // Dongle: dòng cao gần gấp đôi phần mực — đặt chiều cao cố định + căn giữa.
    Item {
        x: 42
        width: parent.width - x - 8
        height: parent.height

        Text {
            width: parent.width
            y: root.sub !== "" ? 7 : 0
            height: root.sub !== "" ? 24 : parent.height
            verticalAlignment: Text.AlignVCenter
            text: root.label
            elide: Text.ElideRight
            color: root.active ? Colors.dark.onSelection : Colors.dark.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 26
        }
        Text {
            visible: root.sub !== ""
            width: parent.width
            y: 29
            height: 20
            verticalAlignment: Text.AlignVCenter
            text: root.sub
            elide: Text.ElideRight
            color: root.active ? Colors.dark.onSelectionMuted : Colors.dark.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 22
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
