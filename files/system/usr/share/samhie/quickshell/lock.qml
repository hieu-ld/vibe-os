// Màn hình khoá Samhie (thay hyprlock — giai đoạn 3).
//
// /usr/libexec/samhie/lock chạy file này bằng `quickshell -p`, MỘT tiến trình
// riêng, tách khỏi thanh trên: thanh trên có lỗi hay nạp lại cũng không đụng
// tới màn hình khoá. hypridle gọi script đó mỗi khi `loginctl lock-session`
// (Win+L, nút Lock, 5 phút không dùng, trước khi máy ngủ).
//
// Khoá bằng giao thức ext-session-lock của Wayland (WlSessionLock): Hyprland
// che MỌI màn hình và chỉ mở lại khi chính tiến trình này báo mở khoá. Tiến
// trình chết giữa chừng thì màn hình VẪN khoá (Hyprland tô kín một màu) —
// không bao giờ lộ desktop. Mật khẩu kiểm qua PAM (/etc/pam.d/samhie-lock).
//
// Bố cục user chốt 2026-10-01: gọn như hyprlock — giờ to + ngày tiếng Việt,
// một ô mật khẩu có dấu nháy nhấp nháy (hyprlock không có — lý do làm lại),
// nền là hình nền mờ sẵn giống màn hình đăng nhập SDDM.
//
// Theo chế độ sáng/tối (user 2026-10-21; trước đó luôn tối): đọc `lightMode`
// trong ~/.local/state/samhie/settings.json lúc khoá → bảng màu + lớp phủ
// sáng hay tối. Nền: bản mờ của hình người dùng chọn trong Settings
// (~/.local/share/samhie/wallpaper-blur.png, /usr/libexec/samhie/wallpaper
// làm), không có thì bản mờ mặc định của chế độ đó (samhie-blur.png /
// samhie-light-blur.png). Màn đăng nhập SDDM vẫn luôn tối.
import QtQuick
import QtQuick.Controls.Basic
import Quickshell
import Quickshell.Io
import Quickshell.Services.Pam
import Quickshell.Wayland
import "colors.js" as Colors

ShellRoot {
    id: root

    property bool light: false
    FileView {
        path: (Quickshell.env("XDG_STATE_HOME") || Quickshell.env("HOME") + "/.local/state") + "/samhie/settings.json"
        // Đọc chặn (vài chục byte): khung đầu tiên đã đúng chế độ.
        blockLoading: true
        onLoaded: root.light = adapter.lightMode
        JsonAdapter {
            property bool lightMode: false
        }
    }
    readonly property var c: light ? Colors.light : Colors.dark
    readonly property string customBackground: (Quickshell.env("XDG_DATA_HOME") || Quickshell.env("HOME") + "/.local/share")
        + "/samhie/wallpaper-blur.png"
    readonly property string defaultBackground: light ? "/usr/share/backgrounds/samhie/samhie-light-blur.png"
        : "/usr/share/backgrounds/samhie/samhie-blur.png"

    // Trạng thái dùng chung cho mọi màn hình: gõ ở màn nào thì ô ở các màn
    // khác cũng hiện cùng số chấm.
    property string password: ""
    property bool busy: false
    property bool failed: false
    property string pamError: ""
    property bool capsLock: false

    property date now: new Date()
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: root.now = new Date()
    }

    // "Thứ 2, 28/09/2026" / "Chủ nhật, 04/10/2026" — giống SDDM và lịch trên
    // thanh. Tự ghép vì máy chạy locale en_US.
    function vietnameseDate(d) {
        const day = d.getDay() // 0 = Chủ nhật
        const name = day === 0 ? "Chủ nhật" : "Thứ " + (day + 1)
        return name + ", " + Qt.formatDate(d, "dd/MM/yyyy")
    }

    function tryUnlock() {
        if (busy || password === "")
            return
        busy = true
        failed = false
        pamError = ""
        if (!pam.start()) {
            busy = false
            pamError = "Can't check the password"
        }
    }

    PamContext {
        id: pam
        // auth include login — y như /etc/pam.d/hyprlock của Fedora.
        config: "samhie-lock"

        // pam_unix hỏi "Password:" — trả lời bằng chữ đã gõ. Câu hỏi khác
        // (không có trong cấu hình hiện tại) cũng nhận mật khẩu, như hyprlock.
        onPamMessage: {
            if (responseRequired)
                respond(root.password)
            else if (messageIsError)
                root.pamError = message
        }

        onCompleted: result => {
            root.busy = false
            if (result === PamResult.Success) {
                root.password = ""
                sessionLock.locked = false
                return
            }
            root.password = ""
            root.failed = true
        }
    }

    // Caps Lock: Quickshell không đọc được đèn bàn phím → hỏi Hyprland. Lấy
    // "bàn phím nào cũng được": bàn phím ảo của fcitx5 có thể giành chữ "main"
    // (đo trong container). Chạy lúc mở và mỗi lần bấm phím Caps Lock.
    Process {
        id: capsQuery
        command: ["hyprctl", "devices", "-j"]
        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    root.capsLock = JSON.parse(text).keyboards.some(k => k.capsLock)
                } catch (e) {
                    root.capsLock = false
                }
            }
        }
    }
    function checkCapsLock() {
        if (!capsQuery.running)
            capsQuery.running = true
    }
    Component.onCompleted: checkCapsLock()

    // Thoát SAU khi đã mở khoá: chờ một nhịp để lệnh "unlock" kịp gửi tới
    // Hyprland trước khi tiến trình đóng kết nối.
    Timer {
        id: quit
        interval: 200
        onTriggered: Qt.quit()
    }

    WlSessionLock {
        id: sessionLock
        locked: true

        onLockedChanged: if (!locked) quit.start()
        // Ghi vào log (`quickshell log -p …/lock.qml`): Hyprland đã xác nhận khoá.
        onSecureChanged: if (secure) console.info("samhie-lock: locked")

        WlSessionLockSurface {
            id: surface
            color: root.c.bar

            Image {
                anchors.fill: parent
                // Thử bản mờ của hình tự chọn trước; không có (Image.Error) thì
                // bản mặc định. File cục bộ nạp đồng bộ → không chớp.
                property bool fallback: false
                source: "file://" + (fallback ? root.defaultBackground : root.customBackground)
                onStatusChanged: if (status === Image.Error) fallback = true
                fillMode: Image.PreserveAspectCrop
                // Ảnh đã mờ sẵn ở 960x540; smooth để phóng lên không ra ô vuông.
                smooth: true
            }
            // Tối đi một nửa như SDDM để chữ trắng đọc được (chế độ sáng: sáng
            // đi một nửa, chữ tối — `bar` của bảng sáng).
            Rectangle {
                anchors.fill: parent
                color: root.c.bar
                opacity: 0.5
            }

            // Bấm chỗ trống thì trả bàn phím về ô mật khẩu.
            MouseArea {
                anchors.fill: parent
                onPressed: field.forceActiveFocus()
            }

            // ── Giờ + ngày (cùng cỡ với SDDM) ─────────────────────────────
            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.verticalCenter
                anchors.bottomMargin: 56
                // Dongle có khoảng trên/dưới chữ rất lớn — kéo ngày sát lên giờ.
                spacing: -56

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: Qt.formatTime(root.now, "HH:mm")
                    color: root.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 168
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.vietnameseDate(root.now)
                    color: root.c.textMuted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 40
                }
            }

            // ── Ô mật khẩu (cùng kiểu ô của SDDM: 320x52, bo 12, viền 1px) ──
            TextField {
                id: field
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.verticalCenter
                width: 320
                height: 52
                leftPadding: 18
                rightPadding: 18
                verticalAlignment: TextInput.AlignVCenter
                focus: true
                enabled: !root.busy

                echoMode: TextInput.Password
                passwordCharacter: "●"
                placeholderText: "Password"
                color: root.c.text
                placeholderTextColor: root.c.textMuted
                selectionColor: root.c.selection
                selectedTextColor: root.c.onSelection
                // Dấu chấm của Dongle chỉ cao 3px (đo ở SDDM) → đang có chữ thì
                // dùng Noto Sans; chữ mờ "Password" vẫn Dongle.
                readonly property bool dots: text !== ""
                font.family: dots ? "Noto Sans" : Colors.fonts.ui
                font.weight: dots ? Font.Normal : Colors.fonts.uiWeight
                font.pixelSize: dots ? 14 : 30
                font.letterSpacing: dots ? 4 : 0

                // Hai chiều với root.password: màn khác gõ thì ô này cũng đổi.
                text: root.password
                onTextEdited: {
                    root.password = text
                    root.failed = false
                    root.pamError = ""
                }
                onAccepted: root.tryUnlock()
                Keys.onEscapePressed: root.password = ""
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_CapsLock)
                        root.checkCapsLock()
                }
                // Hết "đang kiểm" thì giữ bàn phím ở ô (enabled=false làm mất focus).
                onEnabledChanged: if (enabled) forceActiveFocus()

                // Dấu nháy tự vẽ: cao cố định 20px, giữa ô. Dấu nháy có sẵn cao
                // theo dòng chữ — ô trống (Dongle) thì nó cao 40px, có chữ (Noto
                // Sans) còn 20px (đo trong container). Gõ phím thì hiện lại ngay.
                // Qt tự đặt x, y, chiều cao của delegate theo dòng chữ → vạch
                // 20px là con bên trong, dời về giữa ô.
                cursorDelegate: Item {
                    id: caret
                    property bool on: true
                    width: 1
                    visible: field.cursorVisible && on

                    Rectangle {
                        width: 1
                        height: 20
                        y: Math.round((field.height - height) / 2 - caret.y)
                        color: root.c.text
                    }

                    Timer {
                        id: blink
                        interval: 500
                        repeat: true
                        running: field.cursorVisible
                        onTriggered: caret.on = !caret.on
                    }
                    Connections {
                        target: field
                        function onCursorPositionChanged() {
                            caret.on = true
                            blink.restart()
                        }
                    }
                }

                // Viền: cyan lúc đang kiểm, đỏ khi sai — như SDDM.
                background: Rectangle {
                    radius: 12
                    color: root.c.card
                    border.width: 1
                    border.color: root.busy ? root.c.accent : root.failed ? root.c.danger : root.c.subSecondary
                }

                SequentialAnimation {
                    id: shake
                    NumberAnimation { target: field; property: "anchors.horizontalCenterOffset"; to: -10; duration: 50 }
                    NumberAnimation { target: field; property: "anchors.horizontalCenterOffset"; to: 10; duration: 80 }
                    NumberAnimation { target: field; property: "anchors.horizontalCenterOffset"; to: -6; duration: 70 }
                    NumberAnimation { target: field; property: "anchors.horizontalCenterOffset"; to: 0; duration: 60 }
                }
                Connections {
                    target: root
                    function onFailedChanged() {
                        if (root.failed)
                            shake.restart()
                    }
                }
            }

            // Một dòng báo dưới ô: sai mật khẩu / lỗi PAM (đỏ) hoặc Caps Lock (xám).
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: field.bottom
                anchors.topMargin: 12
                text: root.pamError !== "" ? root.pamError
                    : root.failed ? "Incorrect password"
                    : root.capsLock ? "Caps Lock is on" : ""
                color: root.pamError !== "" || root.failed ? root.c.danger : root.c.textMuted
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 28
            }
        }
    }
}
