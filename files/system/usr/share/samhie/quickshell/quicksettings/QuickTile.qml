// Một ô bật/tắt của quick settings — như bản 1.0 (org.samhie.quicksettings,
// Tile.qml): ô TẮT là một lớp màu chữ rất mờ (nổi lên khỏi nền popup đúng
// một bậc), ô BẬT nền `selection` + chữ `on-selection`. Không lấy cyan
// #00C0E8 làm nền: chữ trắng trên nó chỉ 2,16:1 (đo ở bản 1.0).
//
//   [icon]  Tên ô                     [›]
//           dòng phụ (tên mạng, thiết bị…)
// Ô có `more` (Wi-Fi) tách đôi kiểu Windows 11: phần trái bật/tắt, mũi tên
// bên phải mở trang chi tiết (moreClicked).
import QtQuick
import "../bar"
import ".."
import "../colors.js" as Colors

Item {
    id: root

    property string label
    property string sub
    property string iconName
    property string iconCategory: "status"
    property bool active: false
    property bool more: false

    signal clicked()
    signal moreClicked()

    readonly property color fg: active ? Theme.c.onSelection : Theme.c.text
    readonly property int moreWidth: more ? 32 : 0

    height: 56

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: root.active ? Theme.c.selection : Theme.c.text
        opacity: root.active ? 1 : mouse.pressed || moreMouse.pressed ? 0.18
            : mouse.containsMouse || moreMouse.containsMouse ? 0.12 : 0.07
        Behavior on opacity { NumberAnimation { duration: 100 } }
    }
    // Ô đang bật: hover là một lớp sáng mờ phủ lên (hạ độ đục nền màu thì ô
    // trông như đang tắt dần).
    Rectangle {
        anchors.fill: parent
        radius: 8
        visible: root.active
        color: Theme.c.onSelection
        opacity: mouse.pressed || moreMouse.pressed ? 0.16 : mouse.containsMouse || moreMouse.containsMouse ? 0.09 : 0
    }

    SamhieIcon {
        x: 12
        anchors.verticalCenter: parent.verticalCenter
        category: root.iconCategory
        iconName: root.iconName
        size: 20
        color: root.fg
    }

    // Dongle: dòng cao gần gấp đôi phần mực — đặt chiều cao cố định + căn giữa.
    Item {
        x: 42
        width: parent.width - x - 8 - root.moreWidth
        height: parent.height

        Text {
            width: parent.width
            y: root.sub !== "" ? 7 : 0
            height: root.sub !== "" ? 24 : parent.height
            verticalAlignment: Text.AlignVCenter
            text: root.label
            elide: Text.ElideRight
            color: root.fg
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
            color: root.active ? Theme.c.onSelectionMuted : Theme.c.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 22
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        anchors.rightMargin: root.moreWidth
        hoverEnabled: true
        onClicked: root.clicked()
    }

    // Phần mũi tên: vạch ngăn mờ + nền riêng khi rê chuột đúng vào nó.
    Item {
        visible: root.more
        anchors.right: parent.right
        width: root.moreWidth
        height: parent.height

        Rectangle {
            width: 1
            height: 28
            anchors.verticalCenter: parent.verticalCenter
            color: root.fg
            opacity: 0.2
        }
        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: 1
            radius: 8
            color: root.fg
            opacity: moreMouse.pressed ? 0.16 : moreMouse.containsMouse ? 0.1 : 0
        }
        SamhieIcon {
            anchors.centerIn: parent
            iconName: "arrow-right"
            size: 14
            color: root.fg
        }
        MouseArea {
            id: moreMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.moreClicked()
        }
    }
}
