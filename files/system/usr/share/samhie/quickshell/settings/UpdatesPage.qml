// Trang Updates — dáng Software Update của macOS (user chốt 2026-10-22):
//
//   ┌──────────────────────────────────────────────────────────┐
//   │ (↻)  Samhie is up to date                   [Check Now]  │
//   │      Version 2026-10-22 · Fedora 44                      │
//   │      Checked today at 04:02                              │
//   └──────────────────────────────────────────────────────────┘
//   Previous version  2026-10-21                     [Go Back…]
//   Apps  Update automatically every day           [Update Now]
//
// Hệ thống: image TỰ tải + chuẩn bị bản mới mỗi ngày 4 giờ (rpm-ostreed
// AutomaticUpdatePolicy=stage, timer của Universal Blue; mạng tính phí thì
// bỏ qua), có hiệu lực ở lần khởi động sau — trang này không đổi việc đó,
// chỉ hiện trạng thái + làm ngay:
//   Check Now          `rpm-ostree upgrade` (polkit: người đang ngồi trước máy
//                      không cần mật khẩu) — có bản mới thì tải + chuẩn bị luôn
//   Restart to Update  có deployment đang chờ (staged) → `systemctl reboot`
//   Go Back…           hộp xác nhận → `rpm-ostree rollback --reboot` (polkit:
//                      nhóm wheel không hỏi, người khác hỏi mật khẩu admin)
// Đọc: `rpm-ostree status --json` — deployments xếp: đang chờ, đang chạy,
// bản trước. `transaction` khác null = daemon đang làm (timer tự cập nhật
// hay lệnh khác) → hỏi lại mỗi 3 s tới khi xong.
// Lần kiểm gần nhất: lần chạy cuối của timer, hoặc lần bấm Check Now
// (~/.local/state/samhie/update-checked) — lấy cái mới hơn.
// App (Flatpak): tự cập nhật mỗi ngày (flatpak-system-update.timer); Update
// Now = `flatpak update -y --noninteractive` (cập nhật không cần mật khẩu).
import QtQuick
import Quickshell
import Quickshell.Io
import "../bar"
import ".."
import "../colors.js" as Colors

Column {
    id: root

    property string title

    width: parent ? parent.width : 0
    spacing: 20

    readonly property string checkedFile: (Quickshell.env("XDG_STATE_HOME") || Quickshell.env("HOME") + "/.local/state")
        + "/samhie/update-checked"

    // ── Trạng thái hệ thống ────────────────────────────────────────────────
    property bool loaded: false
    property var booted: null       // deployment đang chạy
    property var pending: null      // có hiệu lực ở lần khởi động sau
    property var previous: null     // bản trước (quay về được)
    property bool busy: false       // daemon đang có transaction
    property real checkedAt: 0      // ms, 0 = chưa kiểm lần nào
    property string error: ""       // dòng lỗi (đỏ) dưới tiêu đề
    property string errorTitle: ""  // tiêu đề thay "up to date" khi lỗi
    // Lỗi của rpm-ostree → câu cho người dùng. Hay gặp nhất: timer tự cập nhật
    // đang chạy ("Transaction in progress: upgrade (initiated by …)").
    function fail(title, lastLine, fallback) {
        errorTitle = title
        error = /Transaction in progress/i.test(lastLine)
            ? "Samhie is already updating in the background. Try again in a few minutes."
            : lastLine.replace(/^error:\s*/i, "") || fallback
    }

    Process {
        id: status
        running: true
        command: ["sh", "-c", 'rpm-ostree status --json; echo; echo @@;'
            + ' systemctl show -P LastTriggerUSec --timestamp=unix rpm-ostreed-automatic.timer;'
            + ' cat "$1" 2>/dev/null', "sh", root.checkedFile]
        stdout: StdioCollector {
            onStreamFinished: {
                const parts = text.split("\n@@\n")
                let st
                try {
                    st = JSON.parse(parts[0])
                } catch (e) {
                    root.error = "Couldn't read the update status"
                    return
                }
                const list = st.deployments || []
                const i = list.findIndex(d => d.booted)
                root.booted = i >= 0 ? list[i] : null
                root.pending = i > 0 ? list[0] : null
                root.previous = i >= 0 && i + 1 < list.length ? list[i + 1] : null
                root.busy = !!st.transaction
                // "@1761080591" (timer) và giây Unix (file của trang này).
                const times = (parts[1] || "").split("\n")
                    .map(l => Number(l.trim().replace(/^@/, "")))
                    .filter(n => n > 0)
                root.checkedAt = times.length > 0 ? Math.max(...times) * 1000 : 0
                root.loaded = true
            }
        }
    }
    function refresh() {
        if (!status.running)
            status.running = true
    }
    Timer {
        interval: 3000
        repeat: true
        running: root.busy && !check.running
        onTriggered: root.refresh()
    }

    // "44.20261022" → "2026-10-22 · Fedora 44" (BlueBuild đặt version = bản
    // Fedora + ngày build). Dạng khác thì hiện nguyên. Hai bản build cùng
    // ngày cùng version → thêm giờ build (`timestamp` của deployment = giờ
    // build image, đo 2026-10-24): "2026-10-24 12:52 · Fedora 44".
    function versionText(d, withFedora) {
        const v = d ? (d.version || "") : ""
        const twin = v !== "" && [root.booted, root.pending, root.previous]
            .some(o => o && o !== d && o.version === v)
        const time = twin && d.timestamp ? " " + Qt.formatTime(new Date(d.timestamp * 1000), "HH:mm") : ""
        const m = v.match(/^(\d+)\.(\d{4})(\d{2})(\d{2})(\.\d+)?$/)
        if (!m)
            return v !== "" ? v + time : "unknown"
        return m[2] + "-" + m[3] + "-" + m[4] + (m[5] || "") + time + (withFedora ? " · Fedora " + m[1] : "")
    }
    function checkedText(ms) {
        if (ms <= 0)
            return "Not checked yet"
        const d = new Date(ms)
        const today = new Date()
        const yesterday = new Date(today.getFullYear(), today.getMonth(), today.getDate() - 1)
        const time = Qt.formatTime(d, "HH:mm")
        if (d.toDateString() === today.toDateString())
            return "Checked today at " + time
        if (d.toDateString() === yesterday.toDateString())
            return "Checked yesterday at " + time
        return "Checked on " + Qt.formatDate(d, "yyyy-MM-dd")
    }

    // ── Check Now ──────────────────────────────────────────────────────────
    Process {
        id: check
        command: ["rpm-ostree", "upgrade"]
        property string lastLine: ""
        stderr: SplitParser { onRead: line => { if (line.trim() !== "") check.lastLine = line.trim() } }
        onStarted: {
            root.error = ""
            root.errorTitle = ""
            lastLine = ""
        }
        onExited: code => {
            if (code !== 0)
                root.fail("Couldn't check for updates", lastLine, "Something went wrong.")
            markChecked.running = true
        }
    }
    Process {
        id: markChecked
        command: ["sh", "-c", 'mkdir -p "${1%/*}" && date +%s > "$1"', "sh", root.checkedFile]
        onExited: root.refresh()
    }

    // ── Restart / Go Back ──────────────────────────────────────────────────
    Process {
        id: reboot
        command: ["systemctl", "reboot"]
    }
    property bool confirmBack: false
    Variants {
        model: root.confirmBack ? Quickshell.screens : []
        ConfirmDialog {
            title: "Go back to version " + root.versionText(root.previous, false) + "?"
            message: "Your computer will restart. The newer version comes back with the next update."
            confirmLabel: "Go Back and Restart"
            onConfirmed: {
                root.confirmBack = false
                rollback.running = true
            }
            onCancelled: root.confirmBack = false
        }
    }
    Process {
        id: rollback
        command: ["rpm-ostree", "rollback", "--reboot"]
        property string lastLine: ""
        stderr: SplitParser { onRead: line => { if (line.trim() !== "") rollback.lastLine = line.trim() } }
        onStarted: {
            root.error = ""
            root.errorTitle = ""
            lastLine = ""
        }
        onExited: code => {
            if (code !== 0)
                root.fail("Couldn't go back", lastLine, "Something went wrong.")
            root.refresh()
        }
    }

    // ── Thẻ trạng thái ─────────────────────────────────────────────────────
    readonly property bool working: check.running || rollback.running || busy
    readonly property string headline: !loaded ? "Checking…"
        : rollback.running ? "Going back…"
        : check.running ? "Checking for updates…"
        : busy ? "Updating in the background…"
        : errorTitle !== "" ? errorTitle
        : pending ? "An update is ready"
        : "Samhie is up to date"

    Group {
        Item {
            width: parent.width
            height: Math.max(info.height, action.height) + 32

            Rectangle {
                id: badge
                x: 16
                y: 16
                width: 44
                height: 44
                radius: 22
                color: root.pending && !root.working ? Theme.c.selection : Theme.c.subSecondary
                SamhieIcon {
                    anchors.centerIn: parent
                    iconName: "view-refresh"
                    size: 22
                    color: root.pending && !root.working ? Theme.c.onSelection : Theme.c.text
                }
            }

            Column {
                id: info
                anchors.left: badge.right
                anchors.leftMargin: 14
                anchors.right: action.left
                anchors.rightMargin: 12
                y: 14
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: root.headline
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 30
                }
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: root.pending && !root.working
                        ? "Version " + root.versionText(root.pending, true) + " · restart to finish"
                        : "Version " + root.versionText(root.booted, true)
                    color: Theme.c.textMuted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 22
                    visible: root.loaded
                }
                Text {
                    width: parent.width
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    text: root.error !== "" ? root.error : root.checkedText(root.checkedAt)
                    color: root.error !== "" ? Theme.c.danger : Theme.c.textMuted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 22
                    visible: root.loaded
                }
            }

            Button {
                id: action
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                label: root.pending ? "Restart to Update" : "Check Now"
                primary: !!root.pending
                enabled: root.loaded && !root.working
                onClicked: {
                    if (root.pending)
                        reboot.running = true
                    else
                        check.running = true
                }
            }
        }
    }

    Group {
        visible: !!root.previous
        SettingRow {
            label: "Previous version"
            sub: root.versionText(root.previous, true) + " · if an update broke something"
            labelWidth: width * 0.7
            Button {
                label: "Go Back…"
                enabled: !root.working
                onClicked: root.confirmBack = true
            }
        }
    }

    // ── App (Flatpak) ──────────────────────────────────────────────────────
    property string appsNote: ""
    property bool appsFailed: false
    Process {
        id: apps
        command: ["flatpak", "update", "-y", "--noninteractive"]
        property bool nothing: false
        property string lastLine: ""
        stdout: SplitParser {
            // "Nothing to do." = không có gì mới (chắc chắn có dòng này); dòng
            // báo đang cập nhật thì định dạng đổi theo bản flatpak — không dựa vào.
            onRead: line => {
                if (/Nothing to do/i.test(line))
                    apps.nothing = true
            }
        }
        stderr: SplitParser { onRead: line => { if (line.trim() !== "") apps.lastLine = line.trim() } }
        onStarted: {
            nothing = false
            lastLine = ""
            root.appsFailed = false
        }
        onExited: code => {
            root.appsFailed = code !== 0
            root.appsNote = code !== 0 ? (lastLine.replace(/^error:\s*/i, "") || "Couldn't update apps")
                : nothing ? "Apps are up to date" : "Apps updated"
        }
    }

    Group {
        SettingRow {
            label: "Apps"
            sub: apps.running ? "Updating apps…"
                : root.appsNote !== "" ? root.appsNote
                : "Update automatically every day"
            subColor: root.appsFailed && !apps.running ? Theme.c.danger : Theme.c.textMuted
            labelWidth: width * 0.7
            Button {
                label: "Update Now"
                enabled: !apps.running
                onClicked: apps.running = true
            }
        }
    }
}
