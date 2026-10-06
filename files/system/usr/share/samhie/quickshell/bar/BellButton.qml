// Chuông thông báo, đứng sau bộ gõ (`en`/`vn`) như bản 1.0 (user chốt
// 2026-09-13, giữ ở bản 2). Bấm mở danh sách (NotificationCenter.qml). Icon
// user vẽ: chuông thường / có thông báo (chấm) / Đừng làm phiền (gạch).
import QtQuick
import ".."

Item {
    id: root

    required property var bar
    readonly property var notifs: bar.shell.notifs
    readonly property bool lit: hover.hovered || bar.popup === "notifications"

    width: 30
    height: 26

    HoverBox {
        bar: root.bar
        lit: root.lit
        iconName: icon.iconName
        iconSize: icon.size
    }

    SamhieIcon {
        id: icon
        anchors.centerIn: parent
        category: "status"
        iconName: root.notifs.doNotDisturb ? "notifications-disabled"
            : root.notifs.history.length > 0 ? "notification-active" : "notification-inactive"
        size: 18
        color: root.lit ? root.bar.hoverColor : root.bar.textColor
    }

    // Chấm "có thông báo" — bản vẽ user (noti.svg) chấm tâm (13, 6), r 1,5,
    // màu riêng. SamhieIcon tô MỘT màu cả hình nên chấm vẽ ở đây (đã gỡ khỏi
    // SVG); 3×3 ở toạ độ nguyên (12…15, 5…8) cho sắc — lệch bản vẽ 0,5px.
    Rectangle {
        visible: icon.iconName === "notification-active"
        x: icon.x + 12
        y: icon.y + 5
        width: 3
        height: 3
        radius: 1.5
        color: Theme.c.barBadge
    }

    HoverHandler { id: hover }
    TapHandler { onTapped: root.bar.toggle("notifications") }
}
