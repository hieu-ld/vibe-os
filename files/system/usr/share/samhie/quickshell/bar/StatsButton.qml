// Cụm "CPU 12%  45°C   RAM 45%   Swap 0%" trên thanh trên, trước khay (user
// chốt 2026-10-22: bốn số hay theo dõi nằm ở thanh; bấm vào mở bảng Claude
// 5h/7d — ClaudePopup — chứ không lặp lại bốn số này). Số lấy từ SystemStats
// (shell.qml, chung mọi màn hình).
// Mỗi số giữ bề rộng của số dài nhất ("100%", "100°C") — số đổi mỗi 2 s mà
// cụm co giãn thì nhãn nhảy qua lại.
import QtQuick
import ".."
import "../colors.js" as Colors

Item {
    id: root

    required property var bar
    readonly property var stats: bar.shell.stats
    readonly property bool lit: hover.hovered || bar.popup === "claude"
    readonly property color color: lit ? bar.hoverColor : bar.textColor

    width: row.width + 16
    height: 26

    // Ô hover như nút icon (HoverBox), chỉ khác bề ngang.
    Rectangle {
        anchors.fill: parent
        radius: 6
        color: root.lit ? root.bar.hoverFill : "transparent"
    }

    TextMetrics {
        id: pct
        font: cpuLabel.font
        text: "100%"
    }
    TextMetrics {
        id: deg
        font: cpuLabel.font
        text: "100°C"
    }

    component Label: Text {
        anchors.verticalCenter: parent.verticalCenter
        color: root.color
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pointSize: Colors.fonts.size
    }
    component Value: Label {
        property int value: -1
        property string unit: "%"
        width: Math.ceil(unit === "%" ? pct.advanceWidth : deg.advanceWidth)
        text: value >= 0 ? value + unit : "–"
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 5

        Label { id: cpuLabel; text: "CPU" }
        Value { value: root.stats.cpu }
        Value {
            visible: root.stats.temp >= 0
            value: root.stats.temp
            unit: "°C"
        }
        Item { width: 4; height: 1 }
        Label { text: "RAM" }
        Value { value: root.stats.memory }
        Item { width: 4; height: 1; visible: root.stats.swap >= 0 }
        Label { text: "Swap"; visible: root.stats.swap >= 0 }
        Value { value: root.stats.swap; visible: root.stats.swap >= 0 }
    }

    HoverHandler { id: hover }
    TapHandler {
        onTapped: {
            root.bar.claudeX = root.mapToItem(null, root.width, 0).x
            root.bar.toggle("claude")
        }
    }
}
