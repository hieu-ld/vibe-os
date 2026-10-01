// Nút nguồn góc phải: icon `system-shutdown` (nét user vẽ), bấm mở PowerMenu.
import QtQuick

Rectangle {
    id: root

    required property var bar
    readonly property bool lit: hover.hovered || bar.popup === "power"

    width: 30
    height: 26
    radius: 6
    color: lit ? bar.hoverFill : "transparent"

    SamhieIcon {
        anchors.centerIn: parent
        iconName: "system-shutdown"
        size: 18
        color: root.lit ? root.bar.hoverColor : root.bar.textColor
    }

    HoverHandler { id: hover }
    TapHandler { onTapped: root.bar.toggle("power") }
}
