// Trang Appearance — dáng trang Appearance + Wallpaper + Lock Screen của
// macOS System Settings gộp lại:
//
//   Appearance       ┌──────┐  ┌──────┐
//                    │ sáng │  │ tối  │
//                    └──────┘  └──────┘
//                     ○ Light   ● Dark
//   ┌────────┐ Wallpaper                        [Use Default] [Choose…]
//   │  ảnh   │ Samhie (default)
//   └────────┘
//   Lock screen after                                [ 5 minutes     ˅ ]
//   Display turns off 30 seconds later
//
// Sáng/tối: shell chính giữ (quicksettings/Settings.qml) → gọi IPC `prefs
// setLightMode`; trang này đổi màu theo settings.json như cả app.
// Hình nền: Choose… mở hộp chọn file KDE (kdialog), CHÉP ảnh về
// ~/.local/share/samhie/wallpaper-<giờ> + symlink `wallpaper` (xoá/dời file
// gốc vẫn còn hình nền; user chốt 2026-10-21), một hình cho cả hai chế độ.
// samhie.lua đọc symlink đó (sau samhie.wallpaper của hyprland.lua) →
// `samhie_wallpaper_refresh()` vẽ lại ngay; màn tổng quan nhận đường dẫn
// mới qua IPC `wallpaper set`.
// Tự khoá: ghi số giây vào ~/.local/state/samhie/lock-after (0 = Never) rồi
// chạy lại hypridle (/usr/libexec/samhie/idle restart). Có
// ~/.config/hypr/hypridle.conf riêng thì ô khoá lại — file đó thắng.
import QtQuick
import Quickshell
import Quickshell.Io
import Quickshell.Hyprland
import Quickshell.Widgets
import "../bar"
import ".."
import "../colors.js" as Colors

Column {
    id: root

    property string title

    readonly property string home: Quickshell.env("HOME")
    readonly property string dataDir: (Quickshell.env("XDG_DATA_HOME") || home + "/.local/share") + "/samhie"
    readonly property string stateDir: (Quickshell.env("XDG_STATE_HOME") || home + "/.local/state") + "/samhie"
    readonly property string shellDir: Quickshell.env("SAMHIE_SHELL") || "/usr/share/samhie/quickshell"
    readonly property string defaultDark: "/usr/share/backgrounds/samhie/samhie.png"
    readonly property string defaultLight: "/usr/share/backgrounds/samhie/samhie-light.png"

    function ipc(args) {
        Quickshell.execDetached(["quickshell", "ipc", "-p", shellDir, "call"].concat(args))
    }

    // ── Sáng / tối ─────────────────────────────────────────────────────────
    function setLight(on) {
        if (on !== Theme.light)
            ipc(["prefs", "setLightMode", String(on)])
    }

    // ── Hình nền ───────────────────────────────────────────────────────────
    property string custom: ""       // đường dẫn thật của hình đã chọn, "" = mặc định
    property string customName: ""   // tên file gốc (wallpaper.name)
    // samhie.wallpaper đặt trong hyprland.lua (shell chạy với SAMHIE_WALLPAPER
    // khác mặc định và khác hình của Settings) → nút khoá lại, nói rõ lý do.
    readonly property string envDark: Quickshell.env("SAMHIE_WALLPAPER") || ""
    readonly property bool fromLua: envDark !== "" && envDark !== defaultDark && !envDark.startsWith(dataDir + "/")
    readonly property string shown: fromLua ? envDark : custom !== "" ? custom : Theme.light ? defaultLight : defaultDark

    Process {
        id: wallQuery
        command: ["sh", "-c", 'readlink -e "$1/wallpaper"; echo; cat "$1/wallpaper.name" 2>/dev/null', "sh", root.dataDir]
        running: true
        stdout: StdioCollector {
            onStreamFinished: {
                const lines = text.split("\n")
                root.custom = lines[0] ?? ""
                root.customName = lines[2] ?? ""
            }
        }
    }
    // Chép: bản mới tên theo giờ (ảnh cũ trong bộ nhớ đệm của Qt không bị
    // dùng nhầm), symlink `wallpaper` trỏ sang, xoá bản cũ.
    Process {
        id: wallCopy
        property string source
        command: ["sh", "-c", '
            dir=$1; src=$2; new=wallpaper-$(date +%s)
            mkdir -p "$dir" && cp -- "$src" "$dir/$new.tmp" && mv "$dir/$new.tmp" "$dir/$new" &&
            ln -sfn "$new" "$dir/wallpaper" && printf %s "${src##*/}" > "$dir/wallpaper.name" &&
            find "$dir" -maxdepth 1 -name "wallpaper-*" ! -name "$new" -delete',
            "sh", root.dataDir, source]
        stderr: StdioCollector {
            onStreamFinished: if (text !== "") console.warn("wallpaper:", text)
        }
        onExited: root.wallpaperChanged()
    }
    Process {
        id: wallReset
        command: ["sh", "-c", 'rm -f "$1/wallpaper" "$1/wallpaper.name" "$1"/wallpaper-*', "sh", root.dataDir]
        onExited: root.wallpaperChanged()
    }
    function wallpaperChanged() {
        Hyprland.dispatch("function() samhie_wallpaper_refresh() end")
        wallAfter.running = true
    }
    // Đọc lại đường dẫn thật rồi báo màn tổng quan.
    Process {
        id: wallAfter
        command: ["readlink", "-e", root.dataDir + "/wallpaper"]
        stdout: StdioCollector {
            onStreamFinished: {
                const real = text.trim()
                root.ipc(["wallpaper", "set", real || root.defaultDark, real || root.defaultLight])
                wallQuery.running = true
            }
        }
    }
    // Hộp chọn file KDE (kdialog — cùng hộp đã chỉnh gọn của Dolphin/Kate,
    // style mục 44). FileDialog của QtQuick.Dialogs không gọi được hộp KDE
    // trong Quickshell: ra hộp QML dựng sẵn, không ô gõ đường dẫn (container
    // 2026-10-21). In ra đường dẫn đã chọn; huỷ = mã lỗi, không in gì.
    Process {
        id: picker
        command: ["kdialog", "--title", "Choose a Wallpaper",
                  "--getopenfilename", root.home + "/Pictures", "*.png *.jpg *.jpeg|Images"]
        stdout: StdioCollector {
            onStreamFinished: {
                const path = text.trim()
                if (path === "")
                    return
                wallCopy.source = path
                wallCopy.running = true
            }
        }
    }

    // ── Tự khoá ────────────────────────────────────────────────────────────
    property int lockAfter: 300
    property bool userIdle: false    // ~/.config/hypr/hypridle.conf riêng
    Process {
        id: lockQuery
        command: ["sh", "-c", 'cat "$1/lock-after" 2>/dev/null; echo; test -f "$2" && echo user',
                  "sh", root.stateDir, (Quickshell.env("XDG_CONFIG_HOME") || root.home + "/.config") + "/hypr/hypridle.conf"]
        running: true
        stdout: StdioCollector {
            onStreamFinished: {
                const lines = text.split("\n")
                const n = parseInt(lines[0])
                root.lockAfter = Number.isInteger(n) && n >= 0 ? n : 300
                root.userIdle = lines.includes("user")
            }
        }
    }
    Process {
        id: lockSave
        property int secs
        command: ["sh", "-c", 'mkdir -p "$1" && printf %s "$2" > "$1/lock-after"', "sh", root.stateDir, String(secs)]
        onExited: Hyprland.dispatch('hl.dsp.exec_cmd("/usr/libexec/samhie/idle restart")')
    }
    function setLockAfter(s) {
        lockAfter = s
        lockSave.secs = s
        lockSave.running = true
    }
    readonly property var lockOptions: [
        { label: "1 minute", value: 60 }, { label: "2 minutes", value: 120 },
        { label: "5 minutes", value: 300 }, { label: "10 minutes", value: 600 },
        { label: "15 minutes", value: 900 }, { label: "30 minutes", value: 1800 },
        { label: "1 hour", value: 3600 }, { label: "Never", value: 0 },
    ]

    width: parent ? parent.width : 0
    spacing: 20

    // Ô xem trước một chế độ: thanh trên + cửa sổ thu nhỏ, đúng màu bảng.
    component ModeTile: Item {
        id: tile
        required property bool light
        readonly property var p: light ? Colors.light : Colors.dark
        readonly property bool current: Theme.light === light
        width: 136
        height: 116

        Rectangle {
            id: frame
            width: parent.width
            height: 86
            radius: 8
            clip: true
            color: tile.p.overview
            border.width: 1
            border.color: Theme.c.subSecondary
            Rectangle {      // thanh trên
                x: 1; y: 1
                width: parent.width - 2
                height: 8
                color: tile.p.bar
                radius: 7
            }
            Rectangle {      // cửa sổ
                x: 18; y: 20
                width: 100; height: 56
                radius: 5
                color: tile.p.card
                border.width: 1
                border.color: tile.p.windowBorder
                Rectangle { x: 8; y: 8; width: 26; height: 40; radius: 3; color: tile.p.secondary }
                Rectangle { x: 42; y: 10; width: 46; height: 5; radius: 2; color: tile.p.text; opacity: 0.8 }
                Rectangle { x: 42; y: 20; width: 34; height: 5; radius: 2; color: tile.p.textMuted; opacity: 0.8 }
                Rectangle { x: 42; y: 34; width: 22; height: 9; radius: 3; color: tile.p.selection }
            }
        }
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            y: frame.height + 10
            spacing: 8
            // Nút tròn kiểu radio: chọn = nền `selection` + chấm; không viền accent.
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 16; height: 16; radius: 8
                color: tile.current ? Theme.c.selection : "transparent"
                border.width: tile.current ? 0 : 1
                border.color: Theme.c.textMuted
                Rectangle {
                    visible: tile.current
                    anchors.centerIn: parent
                    width: 6; height: 6; radius: 3
                    color: Theme.c.onSelection
                }
            }
            Text {
                text: tile.light ? "Light" : "Dark"
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 24
            }
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.setLight(tile.light)
        }
    }

    Group {
        Item {
            width: parent.width
            height: 148
            Text {
                x: 16
                y: 16 + 43 - height / 2   // giữa ô xem trước (cao 86)
                text: "Appearance"
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 26
            }
            Row {
                anchors.right: parent.right
                anchors.rightMargin: 16
                y: 16
                spacing: 20
                ModeTile { light: true }
                ModeTile { light: false }
            }
        }
    }

    Group {
        Item {
            width: parent.width
            height: 132
            ClippingRectangle {
                x: 16
                y: 16
                width: 160
                height: 100
                radius: 8
                color: Theme.c.subSecondary
                Image {
                    anchors.fill: parent
                    source: root.shown !== "" ? "file://" + root.shown : ""
                    sourceSize: Qt.size(320, 200)
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                }
            }
            Column {
                x: 196
                width: wallButtons.x - x - 12
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    text: "Wallpaper"
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 26
                }
                Text {
                    width: parent.width
                    elide: Text.ElideMiddle
                    text: root.fromLua ? "Set in ~/.config/hypr/hyprland.lua"
                        : root.custom !== "" ? (root.customName || "Custom picture") : "Samhie (default)"
                    color: Theme.c.textMuted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 22
                }
            }
            Row {
                id: wallButtons
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8
                Button {
                    visible: root.custom !== "" && !root.fromLua
                    label: "Use Default"
                    enabled: !wallReset.running
                    onClicked: wallReset.running = true
                }
                Button {
                    label: "Choose…"
                    enabled: !root.fromLua && !wallCopy.running && !picker.running
                    onClicked: picker.running = true
                }
            }
        }
    }

    Group {
        SettingRow {
            label: "Lock screen after"
            sub: root.userIdle ? "Set in ~/.config/hypr/hypridle.conf"
                : root.lockAfter === 0 ? "Still locks when the computer sleeps"
                : "The display turns off 30 seconds later"
            labelWidth: width * 0.6
            Dropdown {
                enabled: !root.userIdle
                options: root.lockOptions
                current: root.lockAfter
                onPicked: v => root.setLockAfter(v)
            }
        }
    }
}
