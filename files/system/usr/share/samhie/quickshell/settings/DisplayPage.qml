// Trang Display — dáng trang Displays của macOS System Settings:
//
//   ┌──────────────────────────────────────────────────────┐
//   │        ┌────────┐┌──────┐                            │  ← chỉ khi có ≥ 2
//   │        │ DELL   ││ LG   │   kéo để xếp chỗ           │    màn: kéo thả,
//   │        └────────┘└──────┘                            │    tự dính mép
//   └──────────────────────────────────────────────────────┘
//   DELL U2419H
//   │ Resolution                       [ 1920 × 1080   ˅ ] │
//   │ Refresh rate                     [ 60 Hz         ˅ ] │
//   │ Scale                            [ 100%          ˅ ] │
//   │ Rotation                         [ Standard      ˅ ] │
//   │ Use this display                              [●━]  │  ← chỉ khi ≥ 2 màn
//
// Màn đang tắt: một dòng "<tên> — Off" dưới ô xếp chỗ, bấm để chọn.
//
// Đọc: `hyprctl monitors all -j` (cả màn đang tắt). Đổi: chạy ngay
// `hl.monitor({…})` trong Hyprland (Hyprland.dispatch một hàm Lua — đo
// container 2026-10-21: độ phân giải, scale, xoay ăn ngay, không cần nạp lại).
// Đổi độ phân giải / tần số / scale / xoay / tắt màn: hộp thoại "Keep these
// display settings?" giữa mọi màn (ConfirmDialog.qml) đếm 15 s, không bấm
// Keep thì tự quay về như cũ — màn
// đen vì chọn chế độ màn không chạy được thì cứ đợi là xong. Kéo xếp chỗ thì
// lưu luôn (không làm hỏng hình được).
//
// Lưu: ~/.config/hypr/displays.lua — phần RIÊNG của người dùng, chỉ là một
// bảng Lua `return { {output=…, mode=…, …}, … }`. samhie.lua (mặc định trong
// image) đọc file này sau quy tắc màn mặc định; hl.monitor người dùng tự viết
// trong ~/.config/hypr/hyprland.lua vẫn đè lên sau cùng. Màn ghi theo
// `desc:<hãng mẫu số-seri>` (cắm sang cổng khác vẫn nhận), trùng mô tả thì
// ghi theo tên cổng.
import QtQuick
import Quickshell
import Quickshell.Io
import Quickshell.Hyprland
import ".."
import "../colors.js" as Colors

Column {
    id: root

    property string title

    property var monitors: []
    property string selected: ""
    readonly property var mon: monitors.find(m => m.name === selected) ?? monitors[0] ?? null
    readonly property var enabledMonitors: monitors.filter(m => !m.disabled)

    // ── Đọc ────────────────────────────────────────────────────────────────
    Process {
        id: query
        command: ["hyprctl", "monitors", "all", "-j"]
        stdout: StdioCollector {
            onStreamFinished: {
                let list
                try {
                    list = JSON.parse(text)
                } catch (e) {
                    return
                }
                list.sort((a, b) => a.x - b.x || a.y - b.y)
                root.monitors = list
                if (!list.some(m => m.name === root.selected)) {
                    const f = list.find(m => m.focused) ?? list[0]
                    root.selected = f ? f.name : ""
                }
            }
        }
    }
    function refresh() {
        query.running = false
        query.running = true
    }
    Component.onCompleted: refresh()
    Timer {
        id: refreshSoon
        interval: 400
        onTriggered: root.refresh()
    }
    Connections {
        target: Hyprland
        function onRawEvent(event) {
            if (event.name.startsWith("monitoradded") || event.name.startsWith("monitorremoved")
                    || event.name === "configreloaded")
                refreshSoon.restart()
        }
    }

    // ── Quy tắc màn (bảng hl.monitor) ──────────────────────────────────────
    function rateText(r) {
        return String(Number(r.toFixed(2)))
    }
    function modeOf(m) {
        return m.width + "x" + m.height + "@" + rateText(m.refreshRate)
    }
    // Quy tắc tả đúng trạng thái đang chạy của một màn.
    function specOf(m) {
        if (m.disabled)
            return { output: m.name, disabled: true }
        return { output: m.name, mode: modeOf(m), position: m.x + "x" + m.y,
                 scale: m.scale, transform: m.transform }
    }
    // Cỡ logic (sau scale, xoay) của một quy tắc.
    function logical(s) {
        const wh = s.mode.split("@")[0].split("x").map(Number)
        const rot = s.transform % 2 === 1
        return { w: Math.round((rot ? wh[1] : wh[0]) / s.scale),
                 h: Math.round((rot ? wh[0] : wh[1]) / s.scale) }
    }
    function luaValue(v) {
        if (typeof v === "string")
            return '"' + v.replace(/[\\"]/g, "\\$&").replace(/\n/g, "\\n") + '"'
        return String(v)
    }
    function luaTable(s) {
        return "{ " + Object.keys(s).map(k => k + " = " + luaValue(s[k])).join(", ") + " }"
    }
    function run(spec) {
        Hyprland.dispatch("function() hl.monitor(" + luaTable(spec) + ") end")
    }

    // ── Đổi + xác nhận ─────────────────────────────────────────────────────
    property var undo: null         // quy tắc mọi màn trước lần đổi đang chờ Keep
    property int countdown: 0

    // Đổi một màn (patch: mode / scale / transform / disabled). Màn khác nằm
    // bên phải / bên dưới nó dịch theo phần cỡ đổi, để không hở / chồng.
    function change(m, patch) {
        if (undo !== null)
            return
        undo = monitors.map(specOf)
        if (patch.disabled !== undefined) {
            run(patch.disabled ? { output: m.name, disabled: true }
                : { output: m.name, mode: "preferred", position: "auto", scale: "auto" })
        } else {
            const before = specOf(m)
            const after = Object.assign({}, before, patch)
            const a = logical(before), b = logical(after)
            run(after)
            for (const o of enabledMonitors) {
                if (o.name === m.name)
                    continue
                const dx = o.x >= m.x + a.w ? b.w - a.w : 0
                const dy = o.y >= m.y + a.h ? b.h - a.h : 0
                if (dx !== 0 || dy !== 0)
                    run(Object.assign(specOf(o), { position: (o.x + dx) + "x" + (o.y + dy) }))
            }
        }
        countdown = 15
        tick.start()
        refreshSoon.restart()
    }
    function keep() {
        tick.stop()
        undo = null
        save()
    }
    function revert() {
        tick.stop()
        if (undo !== null)
            for (const s of undo)
                run(s)
        undo = null
        refreshSoon.restart()
    }
    Timer {
        id: tick
        interval: 1000
        repeat: true
        onTriggered: {
            root.countdown--
            if (root.countdown <= 0)
                root.revert()
        }
    }
    // Đổi trang / đóng app lúc đang chờ: coi như Revert (chưa bấm Keep).
    Component.onDestruction: if (undo !== null) revert()

    // ── Lưu ~/.config/hypr/displays.lua ────────────────────────────────────
    readonly property string file: (Quickshell.env("XDG_CONFIG_HOME") || Quickshell.env("HOME") + "/.config")
        + "/hypr/displays.lua"

    function descOf(m) {
        // Phòng khi mô tả có kèm " (CỔNG)" ở cuối — `desc:` không có phần đó.
        return (m.description || "").replace(" (" + m.name + ")", "").trim()
    }
    function selectorOf(m) {
        const d = descOf(m)
        const unique = d !== "" && monitors.filter(o => descOf(o) === d).length === 1
        return unique ? "desc:" + d : m.name
    }
    function save() {
        // Đọc lại trạng thái thật rồi mới ghi (Hyprland có thể đã chỉnh scale).
        saveAfterRead.running = true
    }
    Process {
        id: saveAfterRead
        command: ["hyprctl", "monitors", "all", "-j"]
        stdout: StdioCollector {
            onStreamFinished: {
                let list
                try {
                    list = JSON.parse(text)
                } catch (e) {
                    return
                }
                root.monitors = list
                const lines = list.map(m => "    " + root.luaTable(
                    Object.assign(root.specOf(m), { output: root.selectorOf(m) })) + ",")
                const content = "-- Màn hình — app Settings (trang Display) ghi file này mỗi lần bấm Keep\n"
                    + "-- hoặc kéo xếp chỗ màn. samhie.lua đọc nó sau quy tắc màn mặc định;\n"
                    + "-- hl.monitor viết trong hyprland.lua vẫn đè lên. Xoá file = về mặc định.\n"
                    + "return {\n" + lines.join("\n") + "\n}\n"
                writer.command = ["sh", "-c", 'mkdir -p "${1%/*}" && printf %s "$2" > "$1.tmp" && mv "$1.tmp" "$1"',
                                  "sh", root.file, content]
                writer.running = true
            }
        }
    }
    Process {
        id: writer
        stderr: StdioCollector {
            onStreamFinished: if (text !== "") console.warn("displays.lua:", text)
        }
    }

    // ── Lựa chọn cho màn đang chọn ─────────────────────────────────────────
    readonly property var modes: {
        if (!mon)
            return []
        const list = (mon.availableModes ?? []).map(s => {
            const m = s.replace("Hz", "").split("@")
            const wh = m[0].split("x").map(Number)
            return { w: wh[0], h: wh[1], rate: Number(m[1]) }
        })
        if (!list.some(x => x.w === mon.width && x.h === mon.height))
            list.push({ w: mon.width, h: mon.height, rate: mon.refreshRate })
        return list
    }
    readonly property var resolutions: {
        const seen = []
        for (const x of modes.slice().sort((a, b) => b.w * b.h - a.w * a.h || b.w - a.w))
            if (!seen.some(s => s.w === x.w && s.h === x.h))
                seen.push({ w: x.w, h: x.h })
        return seen.map(s => ({ label: s.w + " × " + s.h, value: s.w + "x" + s.h }))
    }
    readonly property var rates: {
        if (!mon)
            return []
        const out = []
        for (const x of modes)
            if (x.w === mon.width && x.h === mon.height && !out.includes(rateText(x.rate)))
                out.push(rateText(x.rate))
        if (!out.includes(rateText(mon.refreshRate)))
            out.push(rateText(mon.refreshRate))
        return out.sort((a, b) => Number(b) - Number(a)).map(r => ({ label: r + " Hz", value: r }))
    }
    // Scale: chỉ những mức cho cỡ logic tròn số (Hyprland tự chỉnh mức lẻ và
    // báo lỗi đỏ trên màn), bề ngang logic ≥ 1024 (nhỏ hơn thì app chật).
    readonly property var scales: {
        if (!mon)
            return []
        const out = []
        for (const s of [1, 1.25, 4 / 3, 1.5, 1.6, 5 / 3, 1.75, 2, 2.25, 2.5, 3]) {
            const w = mon.width / s, h = mon.height / s
            const round = Math.abs(w - Math.round(w)) < 0.01 && Math.abs(h - Math.round(h)) < 0.01
            if (round && (s === 1 || Math.min(w, h) >= 720 && Math.max(w, h) >= 1024))
                out.push(Number(s.toFixed(6)))
        }
        const cur = Number(mon.scale.toFixed(6))
        if (!out.includes(cur))
            out.push(cur)
        return out.sort((a, b) => a - b).map(s => ({ label: Math.round(s * 100) + "%", value: s }))
    }
    readonly property var rotations: [
        { label: "Standard", value: 0 }, { label: "90°", value: 1 },
        { label: "180°", value: 2 }, { label: "270°", value: 3 },
    ]

    function pickResolution(v) {
        const wh = v.split("x").map(Number)
        const fit = modes.filter(x => x.w === wh[0] && x.h === wh[1])
        // Giữ tần số đang dùng nếu độ phân giải mới có, không thì lấy cao nhất.
        const same = fit.find(x => rateText(x.rate) === rateText(mon.refreshRate))
        const rate = same ? same.rate : Math.max(...fit.map(x => x.rate))
        // Scale cũ có thể không hợp độ phân giải mới → về 100%.
        const w = wh[0] / mon.scale, h = wh[1] / mon.scale
        const scale = Math.abs(w - Math.round(w)) < 0.01 && Math.abs(h - Math.round(h)) < 0.01 ? mon.scale : 1
        change(mon, { mode: v + "@" + rateText(rate), scale: scale })
    }

    function nameOf(m) {
        return m.model || m.make || m.name
    }

    width: parent ? parent.width : 0
    spacing: 20

    // ── Xác nhận ───────────────────────────────────────────────────────────
    // Hộp thoại giữa MỌI màn (user 2026-10-21: khung trong trang khó thấy /
    // khó bấm) — chế độ vừa chọn làm một màn đen thì vẫn bấm ở màn kia.
    Variants {
        model: root.undo !== null ? Quickshell.screens : []
        ConfirmDialog {
            countdown: root.countdown
            onKept: root.keep()
            onReverted: root.revert()
        }
    }

    // ── Xếp chỗ (≥ 2 màn) ─────────────────────────────────────────────────
    // Màn đang tắt không có chỗ trong bố cục → một dòng bên dưới để chọn
    // (rồi bật lại bằng "Use this display").
    Group {
        visible: root.monitors.length > 1
        Arrangement {
            width: parent.width
            height: 220
            monitors: root.enabledMonitors
            selected: root.mon ? root.mon.name : ""
            locked: root.undo !== null
            onPicked: name => root.selected = name
            onMoved: positions => {
                for (const m of root.enabledMonitors)
                    root.run(Object.assign(root.specOf(m), { position: positions[m.name] }))
                root.save()
                refreshSoon.restart()
            }
        }
        Repeater {
            model: root.monitors.filter(m => m.disabled)
            ChoiceRow {
                required property var modelData
                label: root.nameOf(modelData) + " — Off"
                iconName: "video-display"
                selected: root.mon !== null && root.mon.name === modelData.name
                onClicked: root.selected = modelData.name
            }
        }
    }

    // ── Màn đang chọn ──────────────────────────────────────────────────────
    Group {
        visible: root.mon !== null
        title: root.mon ? root.nameOf(root.mon) + (root.monitors.length > 1 ? " (" + root.mon.name + ")" : "") : ""
        SettingRow {
            visible: root.mon !== null && !root.mon.disabled
            label: "Resolution"
            Dropdown {
                enabled: root.undo === null
                options: root.resolutions
                current: root.mon ? root.mon.width + "x" + root.mon.height : ""
                onPicked: v => root.pickResolution(v)
            }
        }
        SettingRow {
            visible: root.mon !== null && !root.mon.disabled
            label: "Refresh rate"
            Dropdown {
                enabled: root.undo === null
                options: root.rates
                current: root.mon ? root.rateText(root.mon.refreshRate) : ""
                onPicked: v => root.change(root.mon, { mode: root.mon.width + "x" + root.mon.height + "@" + v })
            }
        }
        SettingRow {
            visible: root.mon !== null && !root.mon.disabled
            label: "Scale"
            Dropdown {
                enabled: root.undo === null
                options: root.scales
                current: root.mon ? Number(root.mon.scale.toFixed(6)) : 1
                onPicked: v => root.change(root.mon, { scale: v })
            }
        }
        SettingRow {
            visible: root.mon !== null && !root.mon.disabled
            label: "Rotation"
            Dropdown {
                enabled: root.undo === null
                options: root.rotations
                current: root.mon ? root.mon.transform % 4 : 0
                onPicked: v => root.change(root.mon, { transform: v })
            }
        }
        SettingRow {
            id: useRow
            visible: root.monitors.length > 1
            label: "Use this display"
            // Không cho tắt màn cuối cùng đang bật.
            readonly property bool last: root.mon !== null && !root.mon.disabled && root.enabledMonitors.length === 1
            sub: last ? "This is the only display that is on" : ""
            Switch {
                opacity: useRow.last || root.undo !== null ? 0.4 : 1
                checked: root.mon !== null && !root.mon.disabled
                onToggled: on => {
                    if (!useRow.last && root.undo === null)
                        root.change(root.mon, { disabled: !on })
                }
            }
        }
    }
}
