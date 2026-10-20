// Trang Wi-Fi của quick settings — kiểu Windows 11: bấm mũi tên trên ô Wi-Fi
// thì cả bảng đổi sang trang này, ← để quay lại.
//
//   [←] Wi-Fi                         [công tắc]
//   (sóng) Nhà-5G        Connected
//          [Forget] [Disconnect]           ← mạng đang mở rộng
//   (sóng🔒) Hàng xóm     Secured
//   ─────────────────────────────────────────
//   More Wi-Fi settings                       ← nmtui trong Konsole
//
// Bấm một mạng là mở rộng nó: mạng có khoá chưa lưu thì hiện ô mật khẩu;
// đã lưu / mạng mở thì nút Connect. Phần lệnh (nmcli, mật khẩu qua stdin)
// ở /usr/libexec/samhie/wifi. Mạng doanh nghiệp (802.1X) và mạng ẩn đưa
// sang nmtui.
//
// Hỏi danh sách CHỈ KHI trang đang mở (mỗi 3 s), xin quét lại lúc mở và mỗi
// 20 s — như quick settings, đóng thì không tốn gì.
import QtQuick
import Quickshell
import Quickshell.Io
import "../bar"
import ".."
import "../colors.js" as Colors

Column {
    id: root

    required property var qs        // QuickSettings.qml
    readonly property bool shown: visible

    readonly property string helper: "/usr/libexec/samhie/wifi"

    property bool radio: qs.wifiRadio
    property var networks: []
    property bool loaded: false

    // Mạng đang mở rộng (SSID), mạng đang kết nối, lỗi của lần kết nối trước.
    property string expanded: ""
    property string connecting: ""
    property string errorFor: ""
    property string errorText: ""
    // Mạng đã lưu nhưng mật khẩu sai/đổi: hiện ô mật khẩu dù đã lưu.
    property string askPassword: ""

    function reset() {
        expanded = ""
        errorFor = ""
        askPassword = ""
    }
    onShownChanged: {
        if (shown) {
            scan()
            poll()
        } else {
            reset()
            imGuard.restore()
        }
    }

    // ── Hỏi danh sách ──────────────────────────────────────────────────────
    Process {
        id: listProc
        command: [root.helper, "list"]
        // Lần hỏi bắt đầu TRƯỚC một cú bấm (bật/tắt sóng, ngắt, quên…) mà
        // xong SAU nó thì kéo giao diện về trạng thái cũ (đo 2026-10-02:
        // tắt sóng xong công tắc nhảy lại "bật") — bỏ kết quả đó, hỏi lại.
        property int startGen: 0
        stdout: StdioCollector {
            onStreamFinished: {
                if (listProc.startGen !== root.gen) {
                    recheck.restart()
                    return
                }
                try {
                    const r = JSON.parse(text)
                    root.radio = r.radio
                    root.networks = r.networks
                    root.sync(r.networks)
                    root.loaded = true
                } catch (e) {
                    console.warn("wifi list:", e)
                }
            }
        }
    }
    // ListModel sửa TẠI CHỖ (set/move/insert/remove) — gán mảng mới thì
    // ListView dựng lại mọi hàng mỗi 3 s, chữ đang gõ trong ô mật khẩu mất
    // theo. Đang mở rộng một mạng thì giữ nguyên thứ tự (sóng lên xuống làm
    // danh sách nhảy dưới tay), mạng mới thêm xuống cuối.
    ListModel { id: netModel }
    function sync(nets) {
        for (let i = netModel.count - 1; i >= 0; i--)
            if (!nets.some(n => n.ssid === netModel.get(i).ssid))
                netModel.remove(i)
        const keep = expanded !== ""
        nets.forEach((n, i) => {
            let j = -1
            for (let k = 0; k < netModel.count; k++)
                if (netModel.get(k).ssid === n.ssid) { j = k; break }
            if (j < 0)
                netModel.insert(keep ? netModel.count : Math.min(i, netModel.count), n)
            else {
                if (!keep && j !== i && i < netModel.count)
                    netModel.move(j, i, 1)
                netModel.set(keep ? j : Math.min(i, netModel.count - 1), n)
            }
        })
    }
    property int gen: 0
    function poll() {
        if (!listProc.running) {
            listProc.startGen = gen
            listProc.running = true
        }
    }
    // Đang quét (vài giây sau khi xin quét): danh sách trống thì ghi
    // "Searching…" chứ chưa phải "No networks found".
    property bool scanning: false
    function scan() {
        Quickshell.execDetached([helper, "scan"])
        scanning = true
        scanDone.restart()
        rescanPoll.restart()
    }
    Timer {
        id: scanDone
        interval: 6000
        onTriggered: {
            root.scanning = false
            root.poll()
        }
    }
    Timer {
        running: root.shown
        interval: 3000
        repeat: true
        onTriggered: root.poll()
    }
    Timer {
        running: root.shown
        interval: 20000
        repeat: true
        onTriggered: root.scan()
    }
    // Quét mất vài giây — hỏi lại sớm để danh sách đầu tiên không trống lâu.
    Timer {
        id: rescanPoll
        interval: 1500
        onTriggered: root.poll()
    }

    // ── Kết nối / ngắt / quên ──────────────────────────────────────────────
    Process {
        id: connectProc
        property string ssid
        property string secret
        command: [root.helper, "connect", ssid]
        stdinEnabled: true
        onStarted: {
            write(secret + "\n")
            secret = ""
        }
        stderr: StdioCollector { id: connectErr }
        onExited: code => {
            const ssid = connectProc.ssid
            root.connecting = ""
            if (code === 0) {
                root.reset()
            } else {
                root.errorFor = ssid
                if (code === 3 || code === 4) {
                    root.askPassword = ssid
                    root.expanded = ssid
                    root.errorText = code === 4 ? "Use 8 to 63 characters"
                        : root.hadPassword ? "Wrong password" : "Password needed"
                } else {
                    root.errorText = "Couldn't connect"
                    console.warn("wifi connect:", connectErr.text)
                }
            }
            root.poll()
        }
    }
    property bool hadPassword: false
    function connect(ssid, secret) {
        if (connectProc.running)
            return
        hadPassword = secret !== ""
        connectProc.ssid = ssid
        connectProc.secret = secret
        errorFor = ""
        connecting = ssid
        gen++
        connectProc.running = true
        imGuard.restore()
    }
    function disconnect() {
        gen++
        Quickshell.execDetached([helper, "disconnect"])
        expanded = ""
        recheck.restart()
    }
    function forget(ssid) {
        gen++
        Quickshell.execDetached([helper, "forget", ssid])
        reset()
        recheck.restart()
    }
    Timer {
        id: recheck
        interval: 600
        onTriggered: root.poll()
    }
    function openSettings() {
        Quickshell.execDetached(["konsole", "-e", "nmtui"])
        qs.bar.popup = ""
    }

    // Gõ mật khẩu Wi-Fi bằng Unikey thì "aa" thành "â" — ô mật khẩu tạm đưa
    // bộ gõ về `en` (fcitx5 dùng chung trạng thái cho mọi cửa sổ —
    // ShareInputState=All), xong thì trả lại `vn` nếu trước đó đang vn.
    QtObject {
        id: imGuard
        property bool switched: false
        function take() {
            if (!probe.running)
                probe.running = true
        }
        function restore() {
            if (switched) {
                Quickshell.execDetached(["fcitx5-remote", "-o"])
                switched = false
            }
        }
    }
    Process {
        id: probe
        // fcitx5-remote in 2 = đang gõ tiếng Việt, 1 = đang en.
        command: ["sh", "-c", "[ \"$(fcitx5-remote)\" = 2 ] && fcitx5-remote -c && echo off"]
        stdout: StdioCollector {
            onStreamFinished: if (text.trim() === "off") imGuard.switched = true
        }
    }

    // ── Giao diện ──────────────────────────────────────────────────────────
    spacing: 8
    // Esc khi ô mật khẩu vừa thu lại (bàn phím đang ở trang này) = đóng bảng.
    Keys.onEscapePressed: qs.bar.popup = ""

    // Đầu trang: ← Wi-Fi ............ [công tắc]
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
            text: "Wi-Fi"
            color: Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 28
        }

        // Công tắc: bật = nền `selection`, núm `on-selection`; tắt = viền mờ.
        Rectangle {
            id: toggle
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            width: 40
            height: 22
            radius: 11
            color: root.radio ? Theme.c.selection : "transparent"
            border.width: root.radio ? 0 : 1
            border.color: Theme.c.textMuted
            Rectangle {
                width: root.radio ? 14 : 12
                height: width
                radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
                x: root.radio ? parent.width - width - 4 : 5
                color: root.radio ? Theme.c.onSelection : Theme.c.textMuted
                Behavior on x { NumberAnimation { duration: 120 } }
            }
            MouseArea {
                anchors.fill: parent
                anchors.margins: -4
                onClicked: {
                    const on = !root.radio
                    root.gen++
                    root.radio = on
                    // Đặt thẳng giá trị — đừng "đảo" theo trạng thái riêng
                    // của bảng chính (hai bên lệch nhau là bấm tắt ra bật).
                    root.qs.setWifi(on)
                    if (!on) {
                        root.networks = []
                        netModel.clear()
                    }
                    root.reset()
                    recheck.restart()
                    if (on)
                        rescanAfterOn.restart()
                }
            }
        }
        // Vừa bật sóng: card cần vài giây mới quét được.
        Timer {
            id: rescanAfterOn
            interval: 2000
            onTriggered: if (root.radio) root.scan()
        }
    }

    // Danh sách mạng — cuộn được, cao tối đa ~6 mạng.
    Item {
        width: parent.width
        height: Math.min(list.contentHeight, 340)
        visible: root.radio && root.networks.length > 0

        ListView {
            id: list
            anchors.fill: parent
            clip: true
            spacing: 2
            boundsBehavior: Flickable.StopAtBounds
            model: netModel
            delegate: networkRow
        }
    }

    // Không có mạng nào / sóng tắt.
    Text {
        visible: !root.radio || root.networks.length === 0
        width: parent.width
        height: 52
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: !root.radio ? "Wi-Fi is off" : root.loaded && !root.scanning ? "No networks found" : "Searching…"
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

    // nmtui: mạng ẩn, mạng doanh nghiệp, IP tĩnh…
    Rectangle {
        width: parent.width
        height: 34
        radius: 6
        color: moreMouse.containsMouse ? Theme.c.secondary : "transparent"
        Text {
            x: 10
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: "More Wi-Fi settings"
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

    // Nút chữ: chính = nền `selection`, phụ = lớp màu chữ mờ (như ô tắt).
    component TextButton: Rectangle {
        id: btn
        property string label
        property bool primary: false
        signal clicked()
        width: btnText.implicitWidth + 28
        height: 32
        radius: 6
        color: primary ? Theme.c.selection
            : Qt.alpha(Theme.c.text, btnMouse.containsMouse ? 0.14 : 0.08)
        opacity: enabled ? 1 : 0.5
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: btn.primary
            color: Theme.c.onSelection
            opacity: btnMouse.pressed ? 0.16 : btnMouse.containsMouse ? 0.09 : 0
        }
        Text {
            id: btnText
            anchors.centerIn: parent
            text: btn.label
            color: btn.primary ? Theme.c.onSelection : Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 24
        }
        MouseArea {
            id: btnMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: btn.clicked()
        }
    }

    Component {
        id: networkRow

        Rectangle {
            id: row
            // Vai trò của netModel (một mạng trong `wifi list`).
            required property string ssid
            required property int signal
            required property bool secure
            required property bool enterprise
            required property bool active
            required property bool saved

            readonly property bool open: root.expanded === ssid
            readonly property bool busy: root.connecting === ssid
            readonly property bool failed: root.errorFor === ssid
            readonly property bool needsPassword: !active && !enterprise
                && (root.askPassword === ssid || (secure && !saved))

            width: ListView.view.width
            height: 52 + (open ? actions.height + 8 : 0)
            radius: 8
            color: open || rowMouse.containsMouse ? Theme.c.secondary : "transparent"

            onOpenChanged: if (!open) pw.text = ""
            onNeedsPasswordChanged: if (needsPassword && open) pw.forceActiveFocus()

            MouseArea {
                id: rowMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (row.open) {
                        root.expanded = ""
                        root.forceActiveFocus()
                        return
                    }
                    root.expanded = row.ssid
                    root.errorFor = ""
                    root.askPassword = ""
                    if (row.needsPassword)
                        pw.forceActiveFocus()
                }
            }

            // Sóng: chấm + 0–3 vòng (icon status network-wireless-signal-*,
            // 4 mức). Mạng có khoá: ổ khoá nhỏ góc dưới-phải.
            Item {
                x: 12
                y: 16
                width: 20
                height: 20
                SamhieIcon {
                    category: "status"
                    size: 20
                    color: Theme.c.text
                    iconName: "network-wireless-signal-" + (row.signal > 75 ? "excellent"
                        : row.signal > 50 ? "good" : row.signal > 25 ? "weak" : "none")
                }
                SamhieIcon {
                    visible: row.secure
                    x: 11
                    y: 10
                    size: 11
                    iconName: "system-lock-screen"
                    color: Theme.c.text
                }
            }

            // Tên mạng + dòng phụ — Dongle: chiều cao cố định + căn giữa.
            Text {
                x: 42
                y: 6
                width: parent.width - x - 10
                height: 24
                verticalAlignment: Text.AlignVCenter
                text: row.ssid
                elide: Text.ElideRight
                color: row.active ? Theme.c.accentText : Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 26
            }
            Text {
                x: 42
                y: 28
                width: parent.width - x - 10
                height: 20
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                text: row.busy ? "Connecting…"
                    : row.failed ? root.errorText
                    : row.active ? (row.secure ? "Connected, secured" : "Connected")
                    : (row.secure ? (row.enterprise ? "Secured, enterprise" : "Secured") : "Open")
                    + (row.saved ? " · Saved" : "")
                color: row.failed && !row.busy ? Theme.c.danger : Theme.c.textMuted
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 22
            }

            // Phần mở rộng: ô mật khẩu (nếu cần) + nút.
            Column {
                id: actions
                visible: row.open
                x: 42
                y: 52
                width: parent.width - x - 10
                spacing: 8

                Text {
                    visible: row.enterprise && !row.saved && !row.active
                    width: parent.width
                    wrapMode: Text.Wrap
                    text: "This network needs a username. Use More Wi-Fi settings."
                    color: Theme.c.textMuted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 22
                }

                Rectangle {
                    visible: row.needsPassword
                    width: parent.width
                    height: 36
                    radius: 6
                    color: Theme.c.card
                    border.width: 1
                    border.color: pw.activeFocus ? Theme.c.accent : Theme.c.subSecondary

                    TextInput {
                        id: pw
                        x: 10
                        width: parent.width - 20
                        anchors.verticalCenter: parent.verticalCenter
                        clip: true
                        echoMode: TextInput.Password
                        passwordCharacter: "•"
                        inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                        color: Theme.c.text
                        selectionColor: Theme.c.selection
                        selectedTextColor: Theme.c.onSelection
                        font.family: Colors.fonts.ui
                        font.weight: Colors.fonts.uiWeight
                        font.pixelSize: 24
                        onActiveFocusChanged: activeFocus ? imGuard.take() : imGuard.restore()

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            visible: pw.text === ""
                            text: "Password"
                            color: Theme.c.textMuted
                            font: pw.font
                        }

                        function submit() {
                            if (pw.text !== "" && !row.busy) {
                                root.connect(row.ssid, pw.text)
                                pw.text = ""
                            }
                        }
                        Keys.onReturnPressed: submit()
                        Keys.onEnterPressed: submit()
                        // Esc: thu hàng lại; Esc lần nữa (bàn phím ở trang) đóng bảng.
                        Keys.onEscapePressed: {
                            root.expanded = ""
                            root.forceActiveFocus()
                        }
                    }
                }

                Row {
                    anchors.right: parent.right
                    spacing: 8
                    layoutDirection: Qt.RightToLeft

                    TextButton {
                        visible: row.active
                        label: "Disconnect"
                        onClicked: root.disconnect()
                    }
                    TextButton {
                        visible: !row.active && !(row.enterprise && !row.saved)
                        primary: true
                        label: "Connect"
                        enabled: !row.busy && (!row.needsPassword || pw.text !== "")
                        onClicked: {
                            root.connect(row.ssid, row.needsPassword ? pw.text : "")
                            pw.text = ""
                        }
                    }
                    TextButton {
                        visible: row.saved && !row.busy
                        label: "Forget"
                        onClicked: root.forget(row.ssid)
                    }
                    TextButton {
                        visible: row.enterprise && !row.saved && !row.active
                        label: "Open settings"
                        onClicked: root.openSettings()
                    }
                }
            }
        }
    }
}
