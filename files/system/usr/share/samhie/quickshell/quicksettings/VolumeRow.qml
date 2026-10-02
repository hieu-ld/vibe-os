// Hàng âm lượng trên cùng của quick settings, như bản 1.0 (user chốt
// 2026-09-10): [loa] ━━━━━●──── 42%  [˅]
//   * icon loa đổi theo mức (audio-volume-high/medium/low/muted — icon Tabler
//     đã có trong bộ Samhie); bấm vào là tắt/bật tiếng;
//   * kéo/bấm/lăn chuột trên thanh để đổi mức;
//   * nút ˅ (chỉ hiện khi có hơn một ngõ ra) mở danh sách chọn ngõ ra.
// Đọc/ghi thẳng qua Pipewire (module của Quickshell), không gọi lệnh — đổi ở
// chỗ khác (phím âm lượng, app) thì hàng này tự đổi theo.
import QtQuick
import Quickshell
import Quickshell.Services.Pipewire
import "../bar"
import ".."
import "../colors.js" as Colors

Column {
    id: root

    readonly property PwNode sink: Pipewire.defaultAudioSink
    readonly property bool ready: sink !== null && sink.audio !== null
    readonly property real volume: ready ? sink.audio.volume : 0
    readonly property bool muted: ready && sink.audio.muted
    // Ngõ ra thật (loa, tai nghe, HDMI…), không tính luồng của app.
    readonly property var sinks: Pipewire.nodes.values.filter(n => n.isSink && !n.isStream && n.audio)
    property bool choosing: false

    // Thuộc tính âm thanh của node chỉ có giá trị khi node được "theo dõi".
    PwObjectTracker {
        objects: root.sink ? [root.sink] : []
    }

    function setVolume(v) {
        if (!ready)
            return
        sink.audio.volume = Math.max(0, Math.min(1, v))
        if (sink.audio.muted && v > 0)
            sink.audio.muted = false
    }

    spacing: 4

    Item {
        width: parent.width
        height: 36

        // Loa: bấm tắt/bật tiếng.
        Rectangle {
            id: speaker
            width: 32
            height: 32
            radius: 8
            anchors.verticalCenter: parent.verticalCenter
            color: speakerMouse.containsMouse ? Theme.c.secondary : "transparent"
            SamhieIcon {
                anchors.centerIn: parent
                category: "status"
                size: 20
                color: Theme.c.text
                iconName: root.muted || root.volume <= 0 ? "audio-volume-muted"
                    : root.volume < 0.34 ? "audio-volume-low"
                    : root.volume < 0.67 ? "audio-volume-medium" : "audio-volume-high"
            }
            MouseArea {
                id: speakerMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: if (root.ready) root.sink.audio.muted = !root.sink.audio.muted
            }
        }

        // Thanh kéo: rãnh `sub-secondary`, phần đã chạy cyan, núm tròn.
        Item {
            id: track
            anchors.left: speaker.right
            anchors.leftMargin: 8
            anchors.right: percent.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            height: 24

            readonly property real shown: root.muted ? 0 : Math.min(1, root.volume)

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                height: 4
                radius: 2
                color: Theme.c.subSecondary
            }
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: Math.max(4, parent.width * track.shown)
                height: 4
                radius: 2
                color: Theme.c.accent
            }
            Rectangle {
                width: 14
                height: 14
                radius: 7
                anchors.verticalCenter: parent.verticalCenter
                x: Math.round(track.shown * (parent.width - width))
                color: Theme.c.text
            }
            MouseArea {
                anchors.fill: parent
                function at(mx) { return mx / width }
                onPressed: mouse => root.setVolume(at(mouse.x))
                onPositionChanged: mouse => { if (pressed) root.setVolume(at(mouse.x)) }
                onWheel: wheel => root.setVolume(root.volume + (wheel.angleDelta.y > 0 ? 0.05 : -0.05))
            }
        }

        Text {
            id: percent
            anchors.right: chooser.visible ? chooser.left : parent.right
            anchors.rightMargin: chooser.visible ? 4 : 2
            anchors.verticalCenter: parent.verticalCenter
            width: 40
            horizontalAlignment: Text.AlignRight
            text: Math.round(root.volume * 100) + "%"
            color: root.muted ? Theme.c.textMuted : Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 24
        }

        Rectangle {
            id: chooser
            visible: root.sinks.length > 1
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 28
            height: 28
            radius: 6
            color: chooserMouse.containsMouse || root.choosing ? Theme.c.secondary : "transparent"
            SamhieIcon {
                anchors.centerIn: parent
                iconName: root.choosing ? "arrow-up" : "arrow-down"
                size: 14
                color: Theme.c.text
            }
            MouseArea {
                id: chooserMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.choosing = !root.choosing
            }
        }
    }

    // Danh sách ngõ ra — ngõ đang dùng có chữ cyan.
    Column {
        visible: root.choosing
        width: parent.width
        Repeater {
            model: root.sinks
            delegate: Rectangle {
                id: sinkRow
                required property PwNode modelData
                readonly property bool current: root.sink !== null && root.sink.id === modelData.id
                width: parent.width
                height: 34
                radius: 6
                color: sinkMouse.containsMouse ? Theme.c.secondary : "transparent"
                Text {
                    x: 40
                    width: parent.width - x - 8
                    anchors.verticalCenter: parent.verticalCenter
                    elide: Text.ElideRight
                    text: sinkRow.modelData.description || sinkRow.modelData.nickname || sinkRow.modelData.name
                    color: sinkRow.current ? Theme.c.accentText : Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 24
                }
                MouseArea {
                    id: sinkMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        Pipewire.preferredDefaultAudioSink = sinkRow.modelData
                        root.choosing = false
                    }
                }
            }
        }
    }
}
