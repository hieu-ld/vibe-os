// Trang Sound — thay pavucontrol (docs/NOTES.md, user chốt 2026-10-21), dáng trang
// Sound của macOS System Settings:
//
//   Output          (loa)  Built-in Audio            ✓
//                   (màn)  HDMI / DisplayPort
//                   ─────────────────────────────────────
//                   Output volume   ━━━━━━●──────  42%
//                   Mute                          [  ]
//   Input           (mic)  Built-in Microphone       ✓
//                   Input volume    ━━━━━━●──────  80%
//                   Mute                          [  ]
//   Applications    Firefox         ━━━━━━●──────  100%
//
// Thiết bị (loa, micro): đọc/ghi thẳng Pipewire qua module của Quickshell
// (không gọi lệnh) — đổi ở chỗ khác (phím âm lượng, quick settings) thì trang
// tự đổi theo.
// Âm lượng từng app: qua `pactl` (pipewire-pulse). App nói giao thức
// PulseAudio (gần như mọi app: Edge, Firefox…) thì Pipewire không gửi channel
// map của luồng → Quickshell không đọc/ghi được âm lượng (cảnh báo "channelVolumes
// and channelMap are not the same size", volume NaN — container 2026-10-21,
// pactl vẫn thấy 100%). Hỏi `pactl -f json list sink-inputs` mỗi giây lúc
// trang mở.
import QtQuick
import Quickshell
import Quickshell.Io
import Quickshell.Services.Pipewire
import ".."
import "../colors.js" as Colors
import "../audio.js" as Audio

Column {
    id: root

    property string title

    readonly property var devices: Pipewire.nodes.values.filter(n => !n.isStream && n.audio)
    readonly property var sinks: devices.filter(n => n.isSink)
    readonly property var sources: devices.filter(n => !n.isSink)

    readonly property PwNode sink: Pipewire.defaultAudioSink
    readonly property PwNode source: Pipewire.defaultAudioSource

    // Âm lượng / properties chỉ có khi node được theo dõi — trang mở mới theo.
    PwObjectTracker {
        objects: root.devices
    }

    // ── Luồng phát của app (pactl) ─────────────────────────────────────────
    // ListModel sửa tại chỗ — gán mảng mới mỗi giây thì dựng lại mọi dòng.
    // Vai trò: sinkInput (số sink-input của pactl), name, volume (0…1), muted.
    ListModel { id: streams }
    property bool dragging: false   // đang kéo một thanh → bỏ kết quả hỏi

    Process {
        id: streamQuery
        command: ["pactl", "-f", "json", "list", "sink-inputs"]
        stdout: StdioCollector {
            onStreamFinished: {
                if (root.dragging)
                    return
                let list
                try {
                    list = JSON.parse(text)
                } catch (e) {
                    return
                }
                const now = list.map(x => {
                    const ch = Object.values(x.volume ?? {})
                    const vol = ch.length ? ch.reduce((a, c) => a + c.value, 0) / ch.length / 65536 : 0
                    return {
                        sinkInput: x.index, muted: x.mute, volume: vol,
                        name: x.properties["application.name"] || x.properties["media.name"] || "Unknown app"
                    }
                })
                for (let i = streams.count - 1; i >= 0; i--)
                    if (!now.some(n => n.sinkInput === streams.get(i).sinkInput))
                        streams.remove(i)
                for (const n of now) {
                    let j = -1
                    for (let k = 0; k < streams.count; k++)
                        if (streams.get(k).sinkInput === n.sinkInput) { j = k; break }
                    if (j < 0)
                        streams.append(n)
                    else
                        streams.set(j, n)
                }
            }
        }
    }
    Timer {
        running: root.visible
        interval: 1000
        repeat: true
        triggeredOnStart: true
        onTriggered: if (!streamQuery.running) streamQuery.running = true
    }
    // Kéo thanh: gộp lại, 60 ms mới gửi một lệnh.
    property var pending: ({})
    function setStream(i, sinkInput, v) {
        streams.setProperty(i, "volume", v)
        pending[sinkInput] = v
        sendTimer.start()
    }
    Timer {
        id: sendTimer
        interval: 60
        onTriggered: {
            for (const id in root.pending)
                Quickshell.execDetached(["pactl", "set-sink-input-volume", id,
                                         Math.round(root.pending[id] * 100) + "%"])
            root.pending = {}
        }
    }

    width: parent ? parent.width : 0
    spacing: 20

    // Âm lượng + % của một node (thiết bị hoặc luồng app).
    component Volume: Row {
        id: vol
        required property PwNode node
        readonly property bool ready: node !== null && node.audio !== null
        // Luồng vừa mở chưa có thông tin kênh → volume NaN (container
        // 2026-10-21) — coi như chưa sẵn sàng.
            && Number.isFinite(node.audio.volume)
        spacing: 12
        Slider {
            anchors.verticalCenter: parent.verticalCenter
            width: 220
            value: vol.ready ? vol.node.audio.volume : 0
            dimmed: vol.ready && vol.node.audio.muted
            onMoved: v => {
                if (!vol.ready)
                    return
                vol.node.audio.volume = v
                if (vol.node.audio.muted && v > 0)
                    vol.node.audio.muted = false
            }
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            width: 48
            horizontalAlignment: Text.AlignRight
            text: vol.ready ? Math.round(vol.node.audio.volume * 100) + "%" : ""
            color: Theme.c.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 24
        }
    }

    // ── Output ─────────────────────────────────────────────────────────────
    Group {
        title: "Output"
        Repeater {
            model: root.sinks
            ChoiceRow {
                required property PwNode modelData
                label: Audio.deviceName(modelData)
                iconName: Audio.outputIcon(modelData)
                selected: root.sink !== null && root.sink.id === modelData.id
                onClicked: Pipewire.preferredDefaultAudioSink = modelData
            }
        }
        InfoRow {
            visible: root.sinks.length === 0
            label: "No output devices"
        }
    }
    Group {
        visible: root.sink !== null
        SettingRow {
            label: "Output volume"
            Volume { node: root.sink }
        }
        SettingRow {
            label: "Mute"
            Switch {
                checked: root.sink !== null && root.sink.audio !== null && root.sink.audio.muted
                onToggled: on => root.sink.audio.muted = on
            }
        }
    }

    // ── Input ──────────────────────────────────────────────────────────────
    Group {
        title: "Input"
        Repeater {
            model: root.sources
            ChoiceRow {
                required property PwNode modelData
                label: Audio.deviceName(modelData)
                iconName: "audio-input-microphone"
                selected: root.source !== null && root.source.id === modelData.id
                onClicked: Pipewire.preferredDefaultAudioSource = modelData
            }
        }
        InfoRow {
            visible: root.sources.length === 0
            label: "No input devices"
        }
    }
    Group {
        visible: root.source !== null
        SettingRow {
            label: "Input volume"
            Volume { node: root.source }
        }
        SettingRow {
            label: "Mute"
            Switch {
                checked: root.source !== null && root.source.audio !== null && root.source.audio.muted
                onToggled: on => root.source.audio.muted = on
            }
        }
    }

    // ── Âm lượng từng app ──────────────────────────────────────────────────
    Group {
        title: "Applications"
        Repeater {
            model: streams
            SettingRow {
                id: app
                required property int index
                required property var model
                label: model.name
                Row {
                    spacing: 12
                    Slider {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 220
                        value: app.model.volume
                        dimmed: app.model.muted
                        onPressedChanged: root.dragging = pressed
                        onMoved: v => root.setStream(app.index, app.model.sinkInput, v)
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 48
                        horizontalAlignment: Text.AlignRight
                        text: Math.round(app.model.volume * 100) + "%"
                        color: Theme.c.textMuted
                        font.family: Colors.fonts.ui
                        font.weight: Colors.fonts.uiWeight
                        font.pixelSize: 24
                    }
                }
            }
        }
        InfoRow {
            visible: streams.count === 0
            label: "No apps are playing sound"
        }
    }
}
