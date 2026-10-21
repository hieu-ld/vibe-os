// Trang Sound Output của quick settings — như mục Sound trong Control Center
// macOS (user chốt 2026-10-21, ảnh tham khảo Ubuntu): bấm › cạnh thanh âm
// lượng thì cả bảng đổi sang trang này (cùng kiểu trang Wi-Fi), ← để quay lại.
//
//   [←] Sound Output
//   (loa)  Built-in Audio              ✓
//   (màn)  HDMI / DisplayPort
//   ─────────────────────────────────────
//   Sound Settings                          ← trang Sound của app Settings
//
// Chọn ngõ ra = Pipewire.preferredDefaultAudioSink (module của Quickshell,
// không gọi lệnh); ngõ đang dùng có dấu ✓.
import QtQuick
import Quickshell
import Quickshell.Services.Pipewire
import "../bar"
import ".."
import "../colors.js" as Colors
import "../audio.js" as Audio

Column {
    id: root

    required property var qs        // QuickSettings.qml

    readonly property PwNode sink: Pipewire.defaultAudioSink
    // Ngõ ra thật (loa, tai nghe, HDMI…), không tính luồng của app.
    readonly property var sinks: Pipewire.nodes.values.filter(n => n.isSink && !n.isStream && n.audio)

    // `properties` của node chỉ có khi node được theo dõi — chỉ lúc trang mở.
    PwObjectTracker {
        objects: root.visible ? root.sinks : []
    }

    function openSettings() {
        Quickshell.execDetached(["/usr/libexec/samhie/settings", "sound"])
        qs.bar.popup = ""
    }

    spacing: 8
    Keys.onEscapePressed: qs.bar.popup = ""

    // Đầu trang: ← Sound Output
    Item {
        width: parent.width
        height: 36

        Rectangle {
            id: back
            width: 32
            height: 32
            radius: 8
            anchors.verticalCenter: parent.verticalCenter
            color: backMouse.containsMouse ? Theme.c.secondary : "transparent"
            SamhieIcon {
                anchors.centerIn: parent
                iconName: "arrow-left"
                size: 16
                color: Theme.c.text
            }
            MouseArea {
                id: backMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.qs.page = ""
            }
        }
        Text {
            anchors.left: back.right
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            height: 36
            verticalAlignment: Text.AlignVCenter
            text: "Sound Output"
            color: Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 28
        }
    }

    // Danh sách ngõ ra.
    Column {
        width: parent.width
        spacing: 2
        visible: root.sinks.length > 0

        Repeater {
            model: root.sinks
            delegate: Rectangle {
                id: row
                required property PwNode modelData
                readonly property bool current: root.sink !== null && root.sink.id === modelData.id

                width: parent.width
                height: 44
                radius: 8
                color: rowMouse.containsMouse ? Theme.c.secondary : "transparent"

                SamhieIcon {
                    x: 12
                    anchors.verticalCenter: parent.verticalCenter
                    category: "devices"
                    size: 20
                    color: Theme.c.text
                    iconName: Audio.outputIcon(row.modelData)
                }
                Text {
                    x: 42
                    width: parent.width - x - 40
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: Audio.deviceName(row.modelData)
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 26
                }
                SamhieIcon {
                    visible: row.current
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "dialog-ok"
                    size: 16
                    color: Theme.c.accentText
                }
                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: Pipewire.preferredDefaultAudioSink = row.modelData
                }
            }
        }
    }

    // Không có ngõ ra nào (Pipewire chưa chạy / máy không có card âm thanh).
    Text {
        visible: root.sinks.length === 0
        width: parent.width
        height: 52
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: "No output devices"
        color: Theme.c.textMuted
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 24
    }

    Rectangle {
        width: parent.width
        height: 1
        color: Theme.c.subSecondary
    }

    Rectangle {
        width: parent.width
        height: 34
        radius: 6
        color: moreMouse.containsMouse ? Theme.c.secondary : "transparent"
        Text {
            x: 10
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: "Sound Settings"
            color: Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 24
        }
        MouseArea {
            id: moreMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.openSettings()
        }
    }
}
