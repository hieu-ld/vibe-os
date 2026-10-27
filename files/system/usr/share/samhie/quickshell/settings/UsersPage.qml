// Trang Users — CHỈ tài khoản đang dùng (user chốt 2026-10-27; tạo/xoá tài
// khoản khác, quyền admin: không làm). Lệnh: /usr/libexec/samhie/account.
//   Đầu trang  vòng tròn chữ cái đầu (như màn đăng nhập — sddm Main.qml) +
//              tên hiển thị + tên đăng nhập
//   Name       tên hiển thị (màn đăng nhập hiện tên này); Save mới lưu
//   Password   bấm Change… mới mở ba ô; đổi bằng `passwd` (két mật khẩu
//              đổi theo — xem helper). Mật khẩu ổ / PIN TPM KHÔNG đổi ở đây.
import QtQuick
import Quickshell
import Quickshell.Io
import ".."
import "../colors.js" as Colors

Column {
    id: root

    property string title
    readonly property string helper: "/usr/libexec/samhie/account"
    property var info: ({})

    width: parent ? parent.width : 0
    spacing: 24

    Process {
        id: load
        running: true
        command: [root.helper, "info"]
        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    root.info = JSON.parse(text)
                    nameField.text = root.info.name ?? ""
                } catch (e) {
                    console.warn("account info:", text)
                }
            }
        }
    }

    // ── Tên hiển thị ───────────────────────────────────────────────────────
    property string nameError: ""
    readonly property bool nameChanged: nameField.text.trim() !== "" && nameField.text.trim() !== (info.name ?? "")
    function saveName() {
        if (!nameChanged || nameAct.running)
            return
        nameError = ""
        nameAct.command = [helper, "set-name", nameField.text.trim()]
        nameAct.running = true
    }
    Process {
        id: nameAct
        stdout: StdioCollector { id: nameOut }
        onExited: code => {
            root.nameError = code === 0 ? "" : (nameOut.text.trim() || "Couldn't change the name")
            load.running = true
        }
    }

    // ── Mật khẩu ───────────────────────────────────────────────────────────
    property bool changing: false
    property string passError: ""
    property bool passDone: false
    readonly property bool passReady: current.text !== "" && fresh.text !== "" && fresh.text === repeat.text
    function openPassword(on) {
        changing = on
        passError = ""
        passDone = false
        current.text = fresh.text = repeat.text = ""
        if (on)
            Qt.callLater(() => current.input.forceActiveFocus())
    }
    function changePassword() {
        if (!passReady || passAct.running)
            return
        passError = ""
        passAct.body = current.text + "\n" + fresh.text + "\n"
        passAct.running = true
    }
    Process {
        id: passAct
        property string body
        command: [root.helper, "password"]
        stdinEnabled: true
        onStarted: {
            write(body)
            body = ""
            stdinEnabled = false
        }
        stdout: StdioCollector { id: passOut }
        onExited: code => {
            stdinEnabled = true
            if (code === 0) {
                root.openPassword(false)
                root.passDone = true
                return
            }
            root.passError = passOut.text.trim() || "Couldn't change the password"
            // Sai mật khẩu hiện tại → xoá ô đó; mật khẩu mới bị chê → xoá hai ô mới.
            if (code === 3) {
                current.text = ""
                current.input.forceActiveFocus()
            } else if (code === 4) {
                fresh.text = repeat.text = ""
                fresh.input.forceActiveFocus()
            }
        }
    }

    // Gõ mật khẩu bằng Unikey thì "aa" thành "â", mà màn đăng nhập/khoá
    // không có Unikey → ô mật khẩu tạm đưa bộ gõ về `en`, như ô mật khẩu
    // Wi-Fi (quicksettings/WifiPage.qml). Chuyển giữa ba ô: activeFocus có
    // lúc cùng false → xét sau (callLater) để không bật/tắt bộ gõ liên tục.
    readonly property bool passFocus: current.input.activeFocus || fresh.input.activeFocus || repeat.input.activeFocus
    onPassFocusChanged: Qt.callLater(syncIm)
    function syncIm() {
        if (passFocus) {
            if (!probe.running && !imSwitched)
                probe.running = true
        } else
            restoreIm()
    }
    property bool imSwitched: false
    function restoreIm() {
        if (imSwitched) {
            Quickshell.execDetached(["fcitx5-remote", "-o"])
            imSwitched = false
        }
    }
    Component.onDestruction: restoreIm()
    Process {
        id: probe
        // fcitx5-remote in 2 = đang gõ tiếng Việt, 1 = đang en.
        command: ["sh", "-c", "[ \"$(fcitx5-remote)\" = 2 ] && fcitx5-remote -c && echo off"]
        stdout: StdioCollector {
            onStreamFinished: if (text.trim() === "off") root.imSwitched = true
        }
    }

    // ── Giao diện ──────────────────────────────────────────────────────────
    Column {
        width: parent.width
        spacing: 4
        topPadding: 8

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 96
            height: 96
            radius: width / 2
            color: Theme.c.secondary
            border.width: 1
            border.color: Theme.c.subSecondary

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 4
                text: ((root.info.name || root.info.user) ?? "").charAt(0).toUpperCase()
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Font.Light
                font.pixelSize: 74
            }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.info.name || root.info.user || ""
            color: Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 40
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: (root.info.name ?? "") !== "" && root.info.name !== root.info.user
            text: root.info.user ?? ""
            color: Theme.c.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 24
        }
    }

    Group {
        SettingRow {
            label: "Name"
            sub: root.nameError !== "" ? root.nameError : "Shown on the login screen"
            subColor: root.nameError !== "" ? Theme.c.danger : Theme.c.textMuted
            Row {
                spacing: 8
                TextField {
                    id: nameField
                    placeholder: "Your name"
                    onAccepted: root.saveName()
                }
                Button {
                    label: "Save"
                    enabled: root.nameChanged && !nameAct.running
                    onClicked: root.saveName()
                }
            }
        }
        InfoRow { label: "Username"; value: root.info.user ?? "" }
    }

    Group {
        title: "Password"
        SettingRow {
            label: "Login password"
            sub: root.passDone ? "Password changed" : "Your keyring of saved passwords changes with it"
            labelWidth: width - 160
            Button {
                label: root.changing ? "Cancel" : "Change…"
                enabled: !passAct.running
                onClicked: root.openPassword(!root.changing)
            }
        }
        SettingRow {
            visible: root.changing
            label: "Current password"
            TextField {
                id: current
                width: 280
                password: true
                onAccepted: fresh.input.forceActiveFocus()
            }
        }
        SettingRow {
            visible: root.changing
            label: "New password"
            TextField {
                id: fresh
                width: 280
                password: true
                onAccepted: repeat.input.forceActiveFocus()
            }
        }
        SettingRow {
            visible: root.changing
            label: "Repeat new password"
            sub: repeat.text !== "" && fresh.text !== repeat.text ? "The new passwords don't match" : ""
            subColor: Theme.c.danger
            TextField {
                id: repeat
                width: 280
                password: true
                onAccepted: root.changePassword()
            }
        }
        // Lý do lỗi (pwquality nói bằng tiếng Anh: "The password is shorter
        // than 8 characters"…) + nút đổi.
        SettingRow {
            visible: root.changing
            label: passAct.running ? "Changing…" : ""
            sub: passAct.running ? "" : root.passError
            subColor: Theme.c.danger
            labelWidth: width - 200
            Button {
                label: "Change Password"
                primary: true
                enabled: root.passReady && !passAct.running
                onClicked: root.changePassword()
            }
        }
    }

    Text {
        x: 8
        width: parent.width - 16
        wrapMode: Text.Wrap
        text: "The disk password and PIN asked for at startup are not changed here."
        color: Theme.c.textMuted
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 22
    }
}
