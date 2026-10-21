// Hàng âm lượng trên cùng của quick settings, như bản 1.0 (user chốt
// 2026-09-10): [loa] ━━━━━●──── 42%  [˅]
//   * icon loa đổi theo mức: 4 bậc 25/50/75/100% + tắt tiếng (user vẽ
//     2026-10-06); bấm vào là tắt/bật tiếng;
//   * kéo/bấm/lăn chuột trên thanh để đổi mức;
//   * nút › mở trang Sound Output (SoundPage.qml — chọn loa/tai nghe/HDMI),
//     như mũi tên của ô Wi-Fi; luôn hiện vì trang có cả "Sound Settings".
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
    signal moreClicked()

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
                // Ba mức + tắt, theo bộ Hugeicons user chọn (volume-35/70/full).
                iconName: root.muted || root.volume <= 0 ? "audio-volume-muted"
                    : root.volume <= 0.35 ? "audio-volume-low"
                    : root.volume <= 0.7 ? "audio-volume-medium" : "audio-volume-high"
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
            // Núm: bản sáng trắng viền xám như núm thanh trượt Qt (vai trò
            // slider-knob*, user 2026-10-21 — `text` bản sáng ra núm đen);
            // bản tối giữ núm `text` không viền.
            Rectangle {
                width: 14
                height: 14
                radius: 7
                anchors.verticalCenter: parent.verticalCenter
                x: Math.round(track.shown * (parent.width - width))
                color: !Theme.light ? Theme.c.text
                    : trackMouse.pressed ? Theme.c.sliderKnobPressed : Theme.c.sliderKnob
                border.width: Theme.light ? 1 : 0
                border.color: trackMouse.pressed ? Theme.c.sliderKnobRimPressed : Theme.c.sliderKnobRim
            }
            MouseArea {
                id: trackMouse
                anchors.fill: parent
                function at(mx) { return mx / width }
                onPressed: mouse => root.setVolume(at(mouse.x))
                onPositionChanged: mouse => { if (pressed) root.setVolume(at(mouse.x)) }
                onWheel: wheel => root.setVolume(root.volume + (wheel.angleDelta.y > 0 ? 0.05 : -0.05))
            }
        }

        Text {
            id: percent
            anchors.right: chooser.left
            anchors.rightMargin: 4
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
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 28
            height: 28
            radius: 6
            color: chooserMouse.containsMouse ? Theme.c.secondary : "transparent"
            SamhieIcon {
                anchors.centerIn: parent
                iconName: "arrow-right"
                size: 14
                color: Theme.c.text
            }
            MouseArea {
                id: chooserMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.moreClicked()
            }
        }
    }
}
