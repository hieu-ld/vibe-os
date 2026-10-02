// Một thẻ thông báo — dùng chung cho popup (NotificationPopups.qml) và danh
// sách của chuông (bar/NotificationCenter.qml). Bản đầu do Claude dựng theo
// kiểu sẵn có (user chốt 2026-10-01: dựng trước, user chỉnh sau): nền `card`,
// viền 1px `sub-secondary` (thông báo khẩn: `danger`), bo 12 như popup lịch.
//
//   [icon]  Tên app · 13:36                 [X]
//           Tiêu đề
//           Nội dung, tối đa 4 dòng
//           [ảnh xem trước]
//           [Nút] [Nút]
//
// Bấm vào thẻ = activated (việc "default" của app). X hiện khi rê chuột.
import QtQuick
import Quickshell
import Quickshell.Widgets
import ".."
import "../bar"
import "../colors.js" as Colors

Rectangle {
    id: root

    required property var entry
    property bool showTime: false

    signal activated()
    signal closeClicked()
    signal actionClicked(var action)

    readonly property var actions: entry && entry.n
        ? entry.n.actions.filter(a => a.identifier !== "default" && a.text !== "")
        : []
    // Icon: tên icon, đường dẫn file hoặc url (Notifications.qml chọn). ""
    // = không có / không tìm thấy → chuông nét của Samhie, tô theo màu chữ
    // (file SVG để màu sáng cứng, nền sáng là biến mất).
    readonly property string iconSource: {
        const i = entry ? entry.icon : ""
        if (i === "")
            return ""
        if (i.startsWith("/"))
            return "file://" + i
        if (i.indexOf("://") > 0)
            return i
        return Quickshell.iconPath(i, true)
    }
    readonly property bool hovered: hover.hovered

    width: 360
    height: content.height + 2 * pad
    radius: 12
    color: Theme.c.card
    border.width: 1
    border.color: entry && entry.critical ? Theme.c.danger : Theme.c.subSecondary

    readonly property int pad: 12
    readonly property int iconSize: 32

    // Bấm dùng MouseArea (không TapHandler): nút X / nút hành động nằm TRÊN
    // nhận cú bấm trước và giữ luôn, thẻ bên dưới không nhận thêm.
    HoverHandler { id: hover }
    MouseArea {
        anchors.fill: parent
        onClicked: root.activated()
    }

    IconImage {
        visible: root.iconSource !== ""
        x: root.pad
        y: root.pad + 2
        implicitSize: root.iconSize
        source: root.iconSource
    }
    SamhieIcon {
        visible: root.iconSource === ""
        x: root.pad
        y: root.pad + 2
        category: "status"
        iconName: "notification-inactive"
        size: root.iconSize
        color: Theme.c.text
    }

    Column {
        id: content
        x: root.pad + root.iconSize + 10
        y: root.pad
        width: root.width - x - root.pad - 20
        spacing: 2

        // Dongle: dòng cao gần gấp đôi phần mực → mỗi dòng đơn đặt chiều cao
        // cố định + căn giữa (bài học ở thanh trên).
        Text {
            width: parent.width
            height: 18
            verticalAlignment: Text.AlignVCenter
            text: (root.entry ? root.entry.appName : "")
                + (root.showTime && root.entry ? " · " + Qt.formatTime(root.entry.time, "HH:mm") : "")
            elide: Text.ElideRight
            color: Theme.c.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 22
        }
        Text {
            width: parent.width
            visible: text !== ""
            text: root.entry ? root.entry.summary : ""
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            lineHeightMode: Text.FixedHeight
            lineHeight: 22
            color: Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Font.Normal
            font.pixelSize: 30
        }
        Text {
            width: parent.width
            visible: text !== ""
            text: root.entry ? root.entry.body : ""
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            maximumLineCount: 4
            elide: Text.ElideRight
            lineHeightMode: Text.FixedHeight
            lineHeight: 19
            color: Theme.c.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 26
        }

        // Ảnh xem trước — vd ảnh vừa chụp (notify-send -i <file>). Điều kiện
        // hiện đặt ở khung NGOÀI: `visible` của ảnh đọc ra là "có thật sự
        // hiện không" — khung ẩn thì ảnh cũng ẩn, lấy nó làm điều kiện là kẹt
        // ẩn mãi.
        Item {
            readonly property bool shown: root.entry !== null && root.entry.preview !== ""
                && preview.status === Image.Ready
            width: parent.width
            height: shown ? preview.height + 6 : 0
            visible: shown
            Image {
                id: preview
                y: 6
                width: parent.width
                height: Math.min(140, implicitHeight > 0 ? width * implicitHeight / implicitWidth : 0)
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignLeft
                source: root.entry ? root.entry.preview : ""
                sourceSize.width: 600
                asynchronous: true
            }
        }

        // Nút hành động của app (trừ "default" — đó là bấm vào thẻ).
        Item {
            width: parent.width
            height: root.actions.length > 0 ? 36 : 0
            visible: root.actions.length > 0
            Row {
                y: 6
                spacing: 6
                Repeater {
                    model: root.actions
                    delegate: Rectangle {
                        id: button
                        required property var modelData
                        width: label.implicitWidth + 20
                        height: 28
                        radius: 6
                        color: buttonHover.hovered ? Theme.c.subSecondary : Theme.c.secondary
                        Text {
                            id: label
                            anchors.centerIn: parent
                            text: button.modelData.text
                            color: Theme.c.text
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pixelSize: 24
                        }
                        HoverHandler { id: buttonHover }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.actionClicked(button.modelData)
                        }
                    }
                }
            }
        }
    }

    // Nút X góc trên-phải, hiện khi rê chuột lên thẻ.
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        width: 24
        height: 24
        radius: 6
        visible: root.hovered
        color: closeHover.hovered ? Theme.c.secondary : "transparent"
        SamhieIcon {
            anchors.centerIn: parent
            iconName: "window-close-symbolic"
            size: 14
            color: Theme.c.text
        }
        HoverHandler { id: closeHover }
        MouseArea {
            anchors.fill: parent
            onClicked: root.closeClicked()
        }
    }
}
