// Chuông thông báo, đứng sau bộ gõ (`en`/`vn`) như bản 1.0 (user chốt
// 2026-09-13, giữ ở bản 2). Bấm mở danh sách (NotificationCenter.qml). Icon
// user vẽ: chuông thường / có thông báo (rung) / Đừng làm phiền (gạch).
import QtQuick

Rectangle {
    id: root

    required property var bar
    readonly property var notifs: bar.shell.notifs
    readonly property bool lit: hover.hovered || bar.popup === "notifications"

    width: 30
    height: 26
    radius: 6
    color: lit ? bar.hoverFill : "transparent"

    SamhieIcon {
        anchors.centerIn: parent
        category: "status"
        iconName: root.notifs.doNotDisturb ? "notifications-disabled"
            : root.notifs.history.length > 0 ? "notification-active" : "notification-inactive"
        size: 18
        color: root.lit ? root.bar.hoverColor : root.bar.textColor
    }

    HoverHandler { id: hover }
    TapHandler { onTapped: root.bar.toggle("notifications") }
}
