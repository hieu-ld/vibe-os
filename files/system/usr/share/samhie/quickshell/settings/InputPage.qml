// Trang Mouse & Keyboard (user chốt 2026-10-22: một trang, ba nhóm, không
// thêm tuỳ chọn ít dùng):
//
//   Mouse      Pointer speed ───●───   Natural scrolling [ ]   Scroll speed ───●───
//   Trackpad   Tap to click [●]        Natural scrolling [●]   (chỉ khi có touchpad)
//   Keyboard   Key repeat delay ───●───   Key repeat rate ───●───
//              Input Method                              [Configure…]
//                                                   [Restore Defaults]
//
// Đọc: `hyprctl getoption` — giá trị Hyprland ĐANG dùng. Đổi: ghi
// ~/.config/hypr/input.lua (phần RIÊNG, chỉ những khoá người dùng đã chỉnh)
// rồi gọi samhie_input_apply() của samhie.lua — hàm đó áp lại TỪ mặc định
// + file, nên Restore Defaults chỉ cần xoá file. Mặc định nằm ở samhie.lua,
// trang này không giữ bản sao.
// Bộ gõ: fcitx5-configtool (Telex/VNI, phím chuyển) — không làm lại.
import QtQuick
import Quickshell
import Quickshell.Io
import Quickshell.Hyprland
import ".."

Column {
    id: root

    property string title

    width: parent ? parent.width : 0
    spacing: 20

    // ── Giá trị đang dùng ──────────────────────────────────────────────────
    property real sensitivity: 0
    property bool naturalScroll: false
    property real scrollFactor: 1
    property int repeatDelay: 600
    property int repeatRate: 25
    property bool padNatural: true
    property bool padTap: true
    property bool hasTouchpad: false

    readonly property var options: ({
        sensitivity: "input:sensitivity",
        naturalScroll: "input:natural_scroll",
        scrollFactor: "input:scroll_factor",
        repeatDelay: "input:repeat_delay",
        repeatRate: "input:repeat_rate",
        padNatural: "input:touchpad:natural_scroll",
        padTap: "input:touchpad:tap_to_click",
    })

    // Mỗi dòng một JSON: {"option": …, "float"|"int"|"bool": …}.
    Process {
        id: query
        running: true
        command: ["sh", "-c", 'for o; do hyprctl getoption "$o" -j | tr -d "\\n"; echo; done; hyprctl devices -j | tr -d "\\n"']
            .concat(["sh"]).concat(Object.values(root.options))
        stdout: StdioCollector {
            onStreamFinished: {
                const names = Object.keys(root.options)
                for (const line of text.trim().split("\n")) {
                    let o
                    try {
                        o = JSON.parse(line)
                    } catch (e) {
                        continue
                    }
                    if (o.mice) {
                        // libinput đặt tên theo phần cứng: "… Touchpad".
                        root.hasTouchpad = o.mice.some(m => /touchpad|trackpad/i.test(m.name))
                        continue
                    }
                    const key = names.find(k => root.options[k] === o.option)
                    if (key)
                        root[key] = o.float ?? o.int ?? o.bool
                }
            }
        }
    }

    // ── Lưu ~/.config/hypr/input.lua ───────────────────────────────────────
    readonly property string file: (Quickshell.env("XDG_CONFIG_HOME") || Quickshell.env("HOME") + "/.config")
        + "/hypr/input.lua"
    // Khoá người dùng đã chỉnh (tên khoá Hyprland), đọc từ file lúc mở trang.
    property var custom: ({})
    property var customPad: ({})
    readonly property bool customized: Object.keys(custom).length + Object.keys(customPad).length > 0

    // File do chính trang này ghi (dạng cố định: `khoá = giá trị`, touchpad
    // lồng một tầng) — đọc lại bằng regex, không cần trình đọc Lua.
    FileView {
        path: root.file
        // Chưa có file (chưa chỉnh gì) là bình thường — đừng cảnh báo.
        printErrors: false
        onLoaded: {
            const t = text().replace(/--[^\n]*/g, "")
            const parse = s => {
                // V4 của Qt chưa có String.matchAll — lặp exec().
                const out = {}
                const re = /(\w+)\s*=\s*(true|false|-?[\d.]+)/g
                let m
                while ((m = re.exec(s)) !== null)
                    out[m[1]] = m[2] === "true" ? true : m[2] === "false" ? false : Number(m[2])
                return out
            }
            const pad = t.match(/touchpad\s*=\s*\{([^}]*)\}/)
            root.customPad = pad ? parse(pad[1]) : {}
            root.custom = parse(pad ? t.replace(pad[0], "") : t)
        }
    }

    function set(key, value) {
        const c = Object.assign({}, custom)
        c[key] = value
        custom = c
        saveTimer.restart()
    }
    function setPad(key, value) {
        const c = Object.assign({}, customPad)
        c[key] = value
        customPad = c
        saveTimer.restart()
    }

    function luaFields(o) {
        return Object.keys(o).map(k => k + " = " + o[k])
    }
    // Kéo thanh trượt: ghi + áp dụng khi tay dừng một nhịp.
    Timer {
        id: saveTimer
        interval: 150
        onTriggered: {
            const lines = root.luaFields(root.custom).map(l => "    " + l + ",")
            if (Object.keys(root.customPad).length > 0)
                lines.push("    touchpad = { " + root.luaFields(root.customPad).join(", ") + " },")
            const content = "-- Chuột + bàn phím — app Settings (trang Mouse & Keyboard) ghi file này,\n"
                + "-- chỉ những gì khác mặc định. samhie.lua đọc nó; input trong hyprland.lua\n"
                + "-- vẫn đè lên. Xoá file = về mặc định.\n"
                + "return {\n" + lines.join("\n") + "\n}\n"
            writer.run(["sh", "-c", 'mkdir -p "${1%/*}" && printf %s "$2" > "$1.tmp" && mv "$1.tmp" "$1"',
                        "sh", root.file, content])
        }
    }
    function restoreDefaults() {
        saveTimer.stop()
        custom = {}
        customPad = {}
        writer.run(["rm", "-f", root.file])
    }
    // Ghi/xoá xong → Hyprland áp lại, rồi đọc lại giá trị thật. Đang chạy mà
    // có lệnh mới: chạy tiếp lệnh mới nhất khi xong.
    Process {
        id: writer
        property var next: null
        function run(cmd) {
            if (running) {
                next = cmd
                return
            }
            command = cmd
            running = true
        }
        stderr: StdioCollector {
            onStreamFinished: if (text !== "") console.warn("input.lua:", text)
        }
        onExited: {
            if (next !== null) {
                const cmd = next
                next = null
                run(cmd)
                return
            }
            Hyprland.dispatch("function() samhie_input_apply() end")
            query.running = true
        }
    }

    // ── Thanh trượt ↔ giá trị (0…1) ───────────────────────────────────────
    function clamp(v) {
        return Math.max(0, Math.min(1, v))
    }
    // Làm tròn theo bước, bỏ đuôi số thực (0.6000000000000001 → 0.6).
    function round(v, step) {
        return Number((Math.round(v / step) * step).toFixed(2))
    }
    // Tốc độ con trỏ: -1…1, giữa = 0 (mặc định).
    // Tốc độ cuộn: ×0,25…×4 theo thang log, giữa = ×1.
    // Trễ lặp phím: 1000 → 150 ms (phải = ngắn, như macOS), 600 ≈ giữa.
    // Tốc độ lặp: 5…50 lần/giây.

    Group {
        title: "Mouse"
        SettingRow {
            label: "Pointer speed"
            Slider {
                width: 220
                value: (root.sensitivity + 1) / 2
                onMoved: v => {
                    root.sensitivity = root.round(v * 2 - 1, 0.05)
                    root.set("sensitivity", root.sensitivity)
                }
            }
        }
        SettingRow {
            label: "Natural scrolling"
            Switch {
                checked: root.naturalScroll
                onToggled: on => {
                    root.naturalScroll = on
                    root.set("natural_scroll", on)
                }
            }
        }
        SettingRow {
            label: "Scroll speed"
            Slider {
                width: 220
                value: root.clamp(Math.log2(root.scrollFactor) / 4 + 0.5)
                onMoved: v => {
                    root.scrollFactor = root.round(Math.pow(2, (v - 0.5) * 4), 0.05)
                    root.set("scroll_factor", root.scrollFactor)
                }
            }
        }
    }

    Group {
        title: "Trackpad"
        visible: root.hasTouchpad
        SettingRow {
            label: "Tap to click"
            Switch {
                checked: root.padTap
                onToggled: on => {
                    root.padTap = on
                    root.setPad("tap_to_click", on)
                }
            }
        }
        SettingRow {
            label: "Natural scrolling"
            sub: "Content follows your fingers"
            labelWidth: width * 0.7
            Switch {
                checked: root.padNatural
                onToggled: on => {
                    root.padNatural = on
                    root.setPad("natural_scroll", on)
                }
            }
        }
    }

    Group {
        title: "Keyboard"
        SettingRow {
            label: "Key repeat delay"
            Slider {
                width: 220
                value: root.clamp((1000 - root.repeatDelay) / 850)
                onMoved: v => {
                    root.repeatDelay = root.round(1000 - v * 850, 10)
                    root.set("repeat_delay", root.repeatDelay)
                }
            }
        }
        SettingRow {
            label: "Key repeat rate"
            Slider {
                width: 220
                value: root.clamp((root.repeatRate - 5) / 45)
                onMoved: v => {
                    root.repeatRate = Math.round(5 + v * 45)
                    root.set("repeat_rate", root.repeatRate)
                }
            }
        }
        SettingRow {
            label: "Input method"
            sub: "Vietnamese typing (Telex, VNI…)"
            labelWidth: width * 0.7
            Button {
                label: "Configure…"
                onClicked: Quickshell.execDetached(["fcitx5-configtool"])
            }
        }
    }

    Item {
        width: parent.width
        height: restore.height
        Button {
            id: restore
            anchors.right: parent.right
            label: "Restore Defaults"
            enabled: root.customized
            onClicked: root.restoreDefaults()
        }
    }
}
