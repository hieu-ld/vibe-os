// Trang Date & Time — dáng trang Date & Time của macOS System Settings:
//
//   │ Set time and date automatically                       [●━] │
//   │ Date and time                  14:41 - Thứ 4, 21/10/2026  │
//   │ Set date and time   [21/10/2026 14:41]           [Set]    │  ← chỉ khi tắt tự động
//   │ Time zone                       [ Ho Chi Minh — Asia   ˅ ] │
//   │                                   GMT+07:00               │
//
// Đọc `timedatectl show` (Timezone, NTP, NTPSynchronized) mỗi 5 s lúc trang
// mở. Đổi bằng `timedatectl set-ntp / set-timezone / set-time` — systemd-
// timedated hỏi polkit, luật mặc định của systemd là auth_admin_keep: hộp
// mật khẩu của polkit-kde hiện lên (một lần, nhớ vài phút). Không thêm luật
// polkit nào — đổi giờ hệ thống thì phải là admin, như macOS phải mở khoá.
//
// Ngày giờ hiển thị giữ tiếng Việt như đồng hồ thanh trên (CLAUDE.md mục 8).
import QtQuick
import QtQuick.Controls.Basic as C
import Quickshell
import Quickshell.Io
import "../bar"
import ".."
import "../colors.js" as Colors

Column {
    id: root

    property string title

    property string zone: ""
    property bool ntp: false
    property bool synced: false
    property string offset: ""       // "+07:00" của múi đang dùng
    property var zones: []           // timedatectl list-timezones
    property string error: ""

    // ── Đọc ────────────────────────────────────────────────────────────────
    Process {
        id: query
        command: ["sh", "-c", "timedatectl show; echo Offset=$(date +%:z)"]
        stdout: StdioCollector {
            onStreamFinished: {
                const kv = {}
                for (const line of text.split("\n")) {
                    const i = line.indexOf("=")
                    if (i > 0)
                        kv[line.slice(0, i)] = line.slice(i + 1)
                }
                root.zone = kv.Timezone ?? ""
                root.ntp = kv.NTP === "yes"
                root.synced = kv.NTPSynchronized === "yes"
                root.offset = kv.Offset ?? ""
            }
        }
    }
    Process {
        id: zoneList
        command: ["timedatectl", "list-timezones"]
        running: true
        stdout: StdioCollector {
            onStreamFinished: root.zones = text.split("\n").filter(z => z !== "")
        }
    }
    function refresh() {
        if (!query.running)
            query.running = true
    }
    Timer {
        running: root.visible
        interval: 5000
        repeat: true
        triggeredOnStart: true
        onTriggered: root.refresh()
    }

    // ── Đổi (qua polkit) ───────────────────────────────────────────────────
    // Huỷ hộp mật khẩu / sai mật khẩu: timedatectl trả lỗi → dòng chữ đỏ.
    Process {
        id: action
        property string what
        stderr: StdioCollector { id: actionErr }
        onExited: code => {
            root.error = code === 0 ? "" : action.what
            if (code !== 0)
                console.warn("timedatectl:", actionErr.text)
            else
                root.clockChanged()
            root.refresh()
        }
    }
    // Giờ / múi giờ vừa đổi: đồng hồ của trang này và của thanh trên vẽ lại
    // ngay, không đợi sang phút mới. SystemClock giữ giờ "treo tường" cũ
    // (`date` đọc ra vẫn 22 giờ sau khi sang Tokyo — container 2026-10-21;
    // new Date() thì đúng ngay) → tắt/bật lại để nó đọc giờ mới. Shell
    // chính: IPC `clock refresh` (shell.qml); SAMHIE_SHELL do samhie.lua đặt
    // khi mở shell — mở app từ terminal thì không có, dùng đường dẫn mặc định.
    function clockChanged() {
        clock.enabled = false
        clock.enabled = true
        Quickshell.execDetached(["quickshell", "ipc", "-p",
            Quickshell.env("SAMHIE_SHELL") || "/usr/share/samhie/quickshell", "call", "clock", "refresh"])
    }
    function run(args, what) {
        if (action.running)
            return
        error = ""
        action.what = what
        action.command = ["timedatectl"].concat(args)
        action.running = true
    }

    // ── Hiển thị ───────────────────────────────────────────────────────────
    readonly property var weekdays: ["Chủ nhật", "Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7"]
    function pad2(n) {
        return n < 10 ? "0" + n : "" + n
    }
    function dateText(d) {
        return pad2(d.getDate()) + "/" + pad2(d.getMonth() + 1) + "/" + d.getFullYear()
    }
    function timeText(d) {
        return pad2(d.getHours()) + ":" + pad2(d.getMinutes())
    }
    // "Asia/Ho_Chi_Minh" → "Ho Chi Minh — Asia"; "America/Argentina/Salta" →
    // "Salta — America/Argentina"; "UTC" → "UTC".
    function zoneLabel(z) {
        const i = z.lastIndexOf("/")
        return i < 0 ? z : z.slice(i + 1).replace(/_/g, " ") + " — " + z.slice(0, i).replace(/_/g, " ")
    }
    SystemClock {
        id: clock
        precision: SystemClock.Minutes
    }

    width: parent ? parent.width : 0
    spacing: 20

    Group {
        SettingRow {
            label: "Set time and date automatically"
            labelWidth: width * 0.6
            sub: !root.ntp ? "" : root.synced ? "Synced with internet time servers" : "Waiting for a time server…"
            Switch {
                checked: root.ntp
                onToggled: on => root.run(["set-ntp", on ? "true" : "false"],
                                          on ? "Couldn't turn on automatic time" : "Couldn't turn off automatic time")
            }
        }
        InfoRow {
            label: "Date and time"
            value: root.timeText(clock.date) + " - " + root.weekdays[clock.date.getDay()] + ", " + root.dateText(clock.date)
        }
        // Đặt tay: một ô "dd/mm/yyyy hh:mm" (giống chữ của dòng trên) + Set.
        SettingRow {
            visible: !root.ntp
            label: "Set date and time"
            sub: manual.valid || manual.text === "" ? "" : "Use dd/mm/yyyy hh:mm"
            Row {
                spacing: 8
                Rectangle {
                    width: 200
                    height: 32
                    radius: 6
                    color: Qt.alpha(Theme.c.text, 0.08)
                    TextInput {
                        id: manual
                        // dd/mm/yyyy hh:mm → "yyyy-mm-dd hh:mm:00" cho timedatectl.
                        readonly property var parts: text.trim().match(/^(\d{1,2})\/(\d{1,2})\/(\d{4})\s+(\d{1,2}):(\d{2})$/)
                        readonly property bool valid: parts !== null && +parts[1] >= 1 && +parts[1] <= 31
                            && +parts[2] >= 1 && +parts[2] <= 12 && +parts[4] <= 23 && +parts[5] <= 59
                        function value() {
                            const p = parts
                            return p[3] + "-" + root.pad2(+p[2]) + "-" + root.pad2(+p[1]) + " "
                                + root.pad2(+p[4]) + ":" + p[5] + ":00"
                        }
                        x: 12
                        width: parent.width - 24
                        height: parent.height
                        verticalAlignment: TextInput.AlignVCenter
                        clip: true
                        color: Theme.c.text
                        selectionColor: Theme.c.selection
                        selectedTextColor: Theme.c.onSelection
                        font.family: Colors.fonts.ui
                        font.weight: Colors.fonts.uiWeight
                        font.pixelSize: 24
                        // Điền sẵn giờ hiện tại mỗi lần dòng hiện ra.
                        Component.onCompleted: reset()
                        function reset() {
                            text = root.dateText(clock.date) + " " + root.timeText(clock.date)
                        }
                        onAccepted: if (valid) root.run(["set-time", value()], "Couldn't set the date and time")
                    }
                }
                Button {
                    label: "Set"
                    enabled: manual.valid
                    onClicked: root.run(["set-time", manual.value()], "Couldn't set the date and time")
                }
            }
        }
        // Huỷ / sai mật khẩu polkit, hoặc timedatectl từ chối.
        Item {
            visible: root.error !== ""
            width: parent.width
            height: 40
            Text {
                x: 16
                width: parent.width - 32
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                text: root.error
                color: Theme.c.danger
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 22
            }
        }
    }

    Group {
        SettingRow {
            label: "Time zone"
            sub: root.offset !== "" ? "GMT" + root.offset : ""
            Rectangle {
                id: zoneButton
                width: 260
                height: 32
                radius: 6
                color: Qt.alpha(Theme.c.text, zoneMouse.containsMouse ? 0.14 : 0.08)
                Text {
                    x: 14
                    width: parent.width - 44
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: root.zone !== "" ? root.zoneLabel(root.zone) : ""
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 24
                }
                SamhieIcon {
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "arrow-down"
                    size: 16
                    color: Theme.c.textMuted
                }
                MouseArea {
                    id: zoneMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: picker.open()
                }

                // Chọn múi giờ: ô tìm + danh sách (~600 múi — Dropdown thường
                // quá dài). Gõ tên thành phố hoặc vùng, Enter chọn dòng đầu.
                C.Popup {
                    id: picker
                    y: zoneButton.height + 4
                    x: zoneButton.width - width
                    width: 340
                    height: 380
                    padding: 6
                    margins: 8
                    closePolicy: C.Popup.CloseOnEscape | C.Popup.CloseOnPressOutside
                    onOpened: {
                        find.text = ""
                        find.forceActiveFocus()
                        const i = list.model.indexOf(root.zone)
                        list.positionViewAtIndex(Math.max(0, i), ListView.Center)
                    }
                    function choose(z) {
                        close()
                        if (z !== root.zone)
                            root.run(["set-timezone", z], "Couldn't change the time zone")
                    }

                    background: Rectangle {
                        radius: 10
                        color: Theme.c.card
                        border.width: 1
                        border.color: Theme.c.subSecondary
                    }
                    contentItem: Column {
                        spacing: 6
                        Rectangle {
                            width: parent.width
                            height: 36
                            radius: 8
                            color: Theme.c.secondary
                            SamhieIcon {
                                x: 10
                                anchors.verticalCenter: parent.verticalCenter
                                iconName: "system-search"
                                size: 16
                                color: Theme.c.textMuted
                            }
                            TextInput {
                                id: find
                                x: 34
                                width: parent.width - x - 10
                                height: parent.height
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                                color: Theme.c.text
                                selectionColor: Theme.c.selection
                                selectedTextColor: Theme.c.onSelection
                                font.family: Colors.fonts.ui
                                font.weight: Colors.fonts.uiWeight
                                font.pixelSize: 24
                                onAccepted: if (list.count > 0) picker.choose(list.model[0])
                                Text {
                                    visible: find.text === ""
                                    height: parent.height
                                    verticalAlignment: Text.AlignVCenter
                                    text: "Search city or region"
                                    color: Theme.c.textMuted
                                    font: find.font
                                }
                            }
                        }
                        ListView {
                            id: list
                            width: parent.width
                            height: picker.availableHeight - 42
                            clip: true
                            boundsBehavior: Flickable.StopAtBounds
                            // Khớp không phân biệt hoa thường, "_" = cách.
                            model: {
                                const q = find.text.trim().toLowerCase().replace(/_/g, " ")
                                return q === "" ? root.zones
                                    : root.zones.filter(z => z.toLowerCase().replace(/_/g, " ").includes(q))
                            }
                            C.ScrollBar.vertical: C.ScrollBar { policy: C.ScrollBar.AsNeeded }
                            delegate: Item {
                                id: zoneItem
                                required property string modelData
                                width: ListView.view.width
                                height: 36
                                Rectangle {
                                    anchors.fill: parent
                                    radius: 6
                                    color: Qt.alpha(Theme.c.text, itemMouse.containsMouse ? 0.08 : 0)
                                }
                                Text {
                                    x: 12
                                    width: parent.width - 48
                                    height: parent.height
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                    text: root.zoneLabel(zoneItem.modelData)
                                    color: Theme.c.text
                                    font.family: Colors.fonts.ui
                                    font.weight: Colors.fonts.uiWeight
                                    font.pixelSize: 24
                                }
                                SamhieIcon {
                                    visible: zoneItem.modelData === root.zone
                                    anchors.right: parent.right
                                    anchors.rightMargin: 12
                                    anchors.verticalCenter: parent.verticalCenter
                                    iconName: "dialog-ok"
                                    size: 16
                                    color: Theme.c.accentText
                                }
                                MouseArea {
                                    id: itemMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: picker.choose(zoneItem.modelData)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
