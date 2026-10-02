// Thiết lập của người dùng mà quick settings đổi — NHỚ qua lần đăng nhập sau,
// ở ~/.local/state/samhie/settings.json (phần riêng của người dùng; image
// không đụng tới). Gồm: Đừng làm phiền (đồng bộ với chuông), Night light,
// chế độ sáng/tối.
//
// Night light: gammastep (kho Fedora) chạy suốt lúc bật — trên Wayland, màu
// ấm chỉ giữ chừng nào chương trình còn giữ "gamma control" của màn hình;
// tắt = dừng tiến trình, màn hình tự về màu thường.
// Sáng/tối: /usr/libexec/samhie/color-mode (app KDE + GTK/Flatpak); giao
// diện Quickshell đọc `lightMode` (giai đoạn 3, mục "Bản sáng").
import QtQuick
import Quickshell
import Quickshell.Io

Scope {
    id: root

    required property var notifs   // Notifications.qml — doNotDisturb

    property bool nightLight: false
    property bool lightMode: false
    readonly property int nightTemperature: 4500   // K — ấm vừa, chữ vẫn đọc rõ

    // Đã đọc xong file (hoặc biết là chưa có) — trước đó đừng ghi đè.
    property bool loaded: false

    function save() {
        if (!loaded)
            return
        store.adapter.doNotDisturb = notifs.doNotDisturb
        store.adapter.nightLight = nightLight
        store.adapter.lightMode = lightMode
        store.writeAdapter()
    }

    FileView {
        id: store
        path: (Quickshell.env("XDG_STATE_HOME") || Quickshell.env("HOME") + "/.local/state") + "/samhie/settings.json"

        onLoaded: {
            root.notifs.doNotDisturb = adapter.doNotDisturb
            root.nightLight = adapter.nightLight
            root.lightMode = adapter.lightMode
            root.loaded = true
            // Áp lại lúc đăng nhập: image cập nhật bảng màu sáng thì file của
            // người dùng nhận bản mới.
            if (root.lightMode)
                colorMode.apply()
        }
        // Lần đầu chưa có file: dùng mặc định, ghi ra ở lần đổi đầu tiên.
        onLoadFailed: root.loaded = true

        JsonAdapter {
            property bool doNotDisturb: false
            property bool nightLight: false
            property bool lightMode: false
        }
    }

    Connections {
        target: root.notifs
        function onDoNotDisturbChanged() { root.save() }
    }
    onNightLightChanged: save()
    onLightModeChanged: {
        save()
        if (loaded)
            colorMode.apply()
    }

    Process {
        id: gammastep
        running: root.loaded && root.nightLight
        // -P: bỏ hiệu chỉnh gamma cũ trước khi đặt; -O: một mức cố định, giữ
        // tới khi tiến trình dừng. setpriv --pdeathsig: Quickshell chết (crash,
        // chạy lại) thì gammastep dừng theo — không thì nó chạy mồ côi, bản
        // mới chạy thêm một cái nữa và tắt Night light không còn ăn (đo trong
        // container 2026-10-02).
        command: ["setpriv", "--pdeathsig", "TERM", "--",
            "gammastep", "-P", "-O", String(root.nightTemperature)]
    }

    Process {
        id: colorMode
        // Bấm liên tục lúc script đang chạy: chạy lại một lần nữa khi xong,
        // với chế độ MỚI NHẤT.
        property bool pending: false
        function apply() {
            if (running) {
                pending = true
                return
            }
            command = ["/usr/libexec/samhie/color-mode", root.lightMode ? "light" : "dark"]
            running = true
        }
        onExited: if (pending) {
            pending = false
            apply()
        }
    }
}
