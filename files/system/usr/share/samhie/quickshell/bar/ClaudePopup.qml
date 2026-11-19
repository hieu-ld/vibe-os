// Bảng Claude: % đã dùng của giới hạn 5 giờ + giới hạn tuần, kèm giờ reset
// (user chốt 2026-10-22). Mở bằng cụm CPU/RAM trên thanh (StatsButton), mép
// phải thẳng mép phải cụm đó.
//
// Số do Claude Code đưa ra qua dòng trạng thái (statusLine) — script
// /usr/libexec/samhie/claude-statusline ghi ~/.cache/samhie/claude.json:
//   {"at": <giây Unix lúc ghi>, "rate_limits": {"five_hour": {"used_percentage",
//    "resets_at"}, "seven_day": {…}}}
// Nên số chỉ mới khi Claude Code đang chạy; tắt hết thì hiện lần cuối +
// "Updated … ago". Qua giờ reset thì cửa sổ đó coi như 0%.
// Chưa có file → nút Connect (claude-connect thêm statusLine vào
// ~/.claude/settings.json; đã có statusLine khác thì không đè, báo lại).
//
// Dưới Claude: Codex + Antigravity (agy) nếu người dùng đã cài (2026-11-19).
// Số hỏi trực tiếp bằng /usr/libexec/samhie/ai-usage lúc mở bảng và mỗi phút
// khi bảng còn mở (agy mất ~4 giây); trong lúc chờ hiện số lần trước.
import QtQuick
import Quickshell
import Quickshell.Io
import ".."
import "../colors.js" as Colors

PopupMotion {
    id: root

    required property var bar
    readonly property bool open: bar.popup === "claude"

    readonly property string file: (Quickshell.env("XDG_CACHE_HOME") || Quickshell.env("HOME") + "/.cache")
        + "/samhie/claude.json"
    property var usage: null
    // Giờ "bây giờ" cho các dòng reset / Updated — nhích theo timer bên dưới.
    property real now: Date.now()

    FileView {
        id: view
        path: root.file
        printErrors: false   // chưa nối Claude Code = chưa có file, bình thường
        onLoaded: {
            try {
                root.usage = JSON.parse(text())
            } catch (e) {
                root.usage = null
            }
        }
        onLoadFailed: root.usage = null
    }
    onOpenChanged: if (open) {
        now = Date.now()
        view.reload()
        if (!usageProc.running)
            usageProc.running = true
    }
    Timer {
        running: root.open
        interval: 5000
        repeat: true
        onTriggered: {
            root.now = Date.now()
            view.reload()
        }
    }

    // ── Codex / Antigravity ────────────────────────────────────────────────
    property var others: null
    Process {
        id: usageProc
        command: ["/usr/libexec/samhie/ai-usage"]
        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    root.others = JSON.parse(text)
                } catch (e) {}
            }
        }
    }
    Timer {
        running: root.open
        interval: 60000
        repeat: true
        onTriggered: if (!usageProc.running) usageProc.running = true
    }
    readonly property var otherTools: others && others.tools ? others.tools : []

    readonly property var windows: {
        const r = usage && usage.rate_limits ? usage.rate_limits : {}
        return [
            { label: "5-hour limit", w: r.five_hour },
            { label: "Weekly limit", w: r.seven_day }
        ].filter(x => x.w)
    }

    // Tên thứ tiếng Anh tự ghi: giao diện tiếng Anh (CLAUDE.md mục 8), không
    // để locale máy quyết.
    readonly property var days: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
    function resetText(sec) {
        const d = new Date(sec * 1000)
        const time = Qt.formatTime(d, "HH:mm")
        if (sec * 1000 <= now)
            return "Reset at " + time
        const today = new Date(now)
        const diff = Math.round((new Date(d.getFullYear(), d.getMonth(), d.getDate())
            - new Date(today.getFullYear(), today.getMonth(), today.getDate())) / 86400000)
        if (diff === 0)
            return "Resets today at " + time
        if (diff === 1)
            return "Resets tomorrow at " + time
        if (diff < 7)
            return "Resets " + root.days[d.getDay()] + " at " + time
        return "Resets " + Qt.formatDate(d, "dd/MM") + " at " + time
    }
    function updatedText(sec) {
        const min = Math.floor((now - sec * 1000) / 60000)
        if (min < 1)
            return "Updated just now"
        if (min < 60)
            return "Updated " + min + " min ago"
        const d = new Date(sec * 1000)
        return "Updated " + (d.toDateString() === new Date(now).toDateString() ? "at " : "on " + Qt.formatDate(d, "dd/MM") + " at ")
            + Qt.formatTime(d, "HH:mm")
    }

    // ── Connect ────────────────────────────────────────────────────────────
    property string connectNote: ""
    property bool connectFailed: false
    Process {
        id: connect
        command: ["/usr/libexec/samhie/claude-connect"]
        property string lastLine: ""
        stderr: SplitParser { onRead: line => { if (line.trim() !== "") connect.lastLine = line.trim() } }
        onStarted: lastLine = ""
        onExited: code => {
            root.connectFailed = code !== 0
            root.connectNote = code === 0 ? "Connected. Numbers appear after Claude Code's next reply."
                : lastLine || "Couldn't connect to Claude Code"
        }
    }

    // Nút chữ phụ — cùng hình Button của Settings / TextButton của trang
    // Wi-Fi (thư mục settings/ không import được từ thanh: "Button is not a type").
    component TextButton: Rectangle {
        id: btn
        property string label
        signal clicked()
        width: btnText.implicitWidth + 28
        height: 32
        radius: 6
        color: Qt.alpha(Theme.c.text, btnMouse.containsMouse && enabled ? 0.14 : 0.08)
        opacity: enabled ? 1 : 0.5
        Text {
            id: btnText
            anchors.centerIn: parent
            text: btn.label
            color: Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 24
        }
        MouseArea {
            id: btnMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: if (btn.enabled) btn.clicked()
        }
    }

    // Một giới hạn: tên + % + thanh + giờ reset. Qua giờ reset thì 0%.
    component LimitRow: Column {
        id: limit
        property string label
        property real usedPercent
        property real resetsAt
        property string note: ""   // nhóm model của agy, đứng trước giờ reset
        readonly property bool past: resetsAt * 1000 <= root.now
        readonly property int used: past ? 0 : Math.round(usedPercent)

        width: content.width
        spacing: 4

        Item {
            width: parent.width
            height: name.height
            Text {
                id: name
                text: limit.label
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 24
            }
            Text {
                anchors.right: parent.right
                text: limit.used + "%"
                color: limit.used >= 90 ? Theme.c.danger : Theme.c.text
                font: name.font
            }
        }
        // Thanh như rãnh thanh trượt của Settings (Slider.qml).
        Rectangle {
            width: parent.width
            height: 6
            radius: 3
            color: Theme.c.subSecondary
            Rectangle {
                width: Math.max(limit.used > 0 ? height : 0, parent.width * Math.min(limit.used, 100) / 100)
                height: parent.height
                radius: 3
                color: limit.used >= 90 ? Theme.c.danger : Theme.c.accent
            }
        }
        Text {
            width: parent.width
            elide: Text.ElideRight
            text: (limit.note ? limit.note + " · " : "") + root.resetText(limit.resetsAt)
            color: Theme.c.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 20
        }
    }

    // Hiện/ẩn có hiệu ứng (PopupMotion).
    showing: open
    transformOrigin: Item.TopRight
    width: card.width
    height: card.height

    Rectangle {
        id: card
        width: 300
        height: content.implicitHeight + 32
        radius: 8
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        // Hút cú bấm vào chỗ trống — không thì lọt xuống PopupLayer và đóng.
        MouseArea { anchors.fill: parent }

        Column {
            id: content
            x: 16
            y: 16
            width: parent.width - 32
            spacing: 14

            Text {
                text: "Claude"
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 26
            }

            // ── Chưa có số ─────────────────────────────────────────────────
            Column {
                visible: root.windows.length === 0
                width: parent.width
                spacing: 12
                Text {
                    width: parent.width
                    wrapMode: Text.Wrap
                    text: root.usage ? "Claude Code hasn't reported any plan limits yet."
                        : "See your plan limits here while Claude Code is running."
                    color: Theme.c.textMuted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 22
                }
                TextButton {
                    visible: !root.usage
                    label: "Connect Claude Code"
                    enabled: !connect.running
                    onClicked: connect.running = true
                }
                Text {
                    visible: root.connectNote !== ""
                    width: parent.width
                    wrapMode: Text.Wrap
                    text: root.connectNote
                    color: root.connectFailed ? Theme.c.danger : Theme.c.textMuted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 22
                }
            }

            // ── Hai giới hạn ───────────────────────────────────────────────
            Repeater {
                model: root.windows
                delegate: LimitRow {
                    required property var modelData
                    label: modelData.label
                    usedPercent: modelData.w.used_percentage
                    resetsAt: modelData.w.resets_at
                }
            }

            Text {
                visible: root.windows.length > 0 && !!root.usage.at
                text: root.usage && root.usage.at ? root.updatedText(root.usage.at) : ""
                color: Theme.c.textMuted
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 20
            }

            // ── Codex / Antigravity ────────────────────────────────────────
            Repeater {
                model: root.otherTools
                delegate: Column {
                    id: tool
                    required property var modelData
                    width: content.width
                    spacing: 14

                    // Vạch ngăn giữa các công cụ.
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: Theme.c.subSecondary
                    }
                    Text {
                        text: tool.modelData.name
                        color: Theme.c.text
                        font.family: Colors.fonts.ui
                        font.weight: Colors.fonts.uiWeight
                        font.pixelSize: 26
                    }
                    Text {
                        visible: !!tool.modelData.error
                        width: parent.width
                        wrapMode: Text.Wrap
                        text: tool.modelData.error || ""
                        color: Theme.c.textMuted
                        font.family: Colors.fonts.ui
                        font.weight: Colors.fonts.uiWeight
                        font.pixelSize: 22
                    }
                    Repeater {
                        model: tool.modelData.windows
                        delegate: LimitRow {
                            required property var modelData
                            label: modelData.window === "5h" ? "5-hour limit" : "Weekly limit"
                            usedPercent: modelData.used
                            resetsAt: modelData.resets_at
                            note: (modelData.group || "").replace(/ models$/i, "").replace(" and ", " & ")
                        }
                    }
                }
            }

            Text {
                visible: root.otherTools.length > 0 && !!root.others.at
                text: root.others && root.others.at ? root.updatedText(root.others.at) : ""
                color: Theme.c.textMuted
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 20
            }
        }
    }
}
