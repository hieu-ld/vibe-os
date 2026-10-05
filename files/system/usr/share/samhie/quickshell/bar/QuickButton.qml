// Nút mở quick settings: một mũi tên như bản 1.0 (user chốt 2026-10-02),
// giữa bộ gõ và chuông: `en ˅ 🔔 ⏻`.
import QtQuick

Item {
    id: root

    required property var bar
    readonly property bool lit: hover.hovered || bar.popup === "quick"

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
        iconName: "arrow-down"
        size: 16
        color: root.lit ? root.bar.hoverColor : root.bar.textColor
    }

    HoverHandler { id: hover }
    TapHandler { onTapped: root.bar.toggle("quick") }
}
