// Menu nguồn, mở dưới nút nguồn, mép phải thẳng nút. Năm mục như bản 1.0
// (user chốt 2026-09-11): Lock / Sleep / Restart / Shut Down / Log Out.
//
// Lock và Sleep làm ngay (quay lại được, Windows cũng vậy). Ba mục còn lại
// làm mất việc đang mở nên phải bấm HAI lần: lần đầu dòng đổi sang chữ đỏ
// "Click again", lần hai mới chạy; để yên 4 giây thì tự về như cũ.
// (Bản 1.0 có màn hình xác nhận của KDE; Hyprland không có.)
//
// Rộng 180 (user 2026-10-09: 250 thấy dài, vừa khít chữ 140 thì gầy).
// Chữ xác nhận vì vậy rút từ "Click again to shut down" (167px chữ, cần thẻ
// ~233) còn "Click again".
//
// Lệnh: logind cho phép phiên đang ngồi trước máy tự tắt/khởi động lại/ngủ
// mà không hỏi mật khẩu. Đăng xuất = thoát Hyprland, giống phím Win+Shift+M.
//
// Máy cài nhiều hệ điều hành (2026-10-26, máy user: Samhie + Bazzite) thì sau
// Restart có thêm "Restart to <tên>" cho mỗi hệ khác: đặt BootNext của UEFI
// (/usr/libexec/samhie/boot-next qua pkexec, polkit org.samhie.boot-next
// không hỏi mật khẩu) rồi khởi động lại — khỏi bấm F8, khỏi mật khẩu BIOS.
// Danh sách đọc lại mỗi lần mở menu; máy một hệ thì menu y như cũ.
// Thẻ rộng ra theo tên dài nhất (Restart to Windows…), tối thiểu 180.
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Io
import ".."
import "../colors.js" as Colors

PopupMotion {
    id: root

    required property var bar
    readonly property bool open: bar.popup === "power"

    // Hệ khác trong menu khởi động: [{ num: "0001", name: "Bazzite" }].
    property var otherSystems: []

    readonly property var actions: {
        const base = [
            { label: "Lock",      icon: "system-lock-screen", confirm: "",            cmd: ["loginctl", "lock-session"] },
            { label: "Sleep",     icon: "system-suspend",     confirm: "",            cmd: ["systemctl", "suspend"] },
            { label: "Restart",   icon: "system-reboot",      confirm: "Click again", cmd: ["systemctl", "reboot"] },
            { label: "Shut Down", icon: "system-shutdown",    confirm: "Click again", cmd: ["systemctl", "poweroff"] },
            { label: "Log Out",   icon: "system-log-out",     confirm: "Click again", cmd: [] }
        ]
        const others = otherSystems.map(o => ({
            label: "Restart to " + o.name,
            icon: "system-reboot",
            confirm: "Click again",
            cmd: ["sh", "-c", 'pkexec /usr/libexec/samhie/boot-next set "$1" && systemctl reboot', "sh", o.num]
        }))
        return base.slice(0, 3).concat(others, base.slice(3))
    }

    Process {
        id: listSystems
        command: ["/usr/libexec/samhie/boot-next", "list"]
        // Đọc sẵn lúc khởi động: mở menu là đủ dòng ngay, không giật cỡ thẻ
        // giữa hiệu ứng mở; mở lại thì đọc lại (menu khởi động có thể đổi).
        running: true
        stdout: StdioCollector {
            onStreamFinished: {
                root.otherSystems = text.split("\n")
                    .map(l => l.split("\t"))
                    .filter(f => f.length === 2 && /^[0-9A-Fa-f]{4}$/.test(f[0]))
                    .map(f => ({ num: f[0], name: f[1].replace(/ Boot Manager$/, "") }))
            }
        }
    }

    // Dòng đang chờ bấm lần hai (-1 = không có).
    property int armed: -1
    onOpenChanged: {
        armed = -1
        if (open)
            listSystems.running = true
    }
    Timer {
        running: root.armed >= 0
        interval: 4000
        onTriggered: root.armed = -1
    }

    function run(i) {
        const a = actions[i]
        if (a.confirm !== "" && armed !== i) {
            armed = i
            return
        }
        bar.popup = ""
        if (a.cmd.length > 0)
            Quickshell.execDetached(a.cmd)
        else
            Hyprland.dispatch("hl.dsp.exit()")
    }

    // Hiện/ẩn có hiệu ứng (PopupMotion).
    showing: open
    transformOrigin: Item.TopRight
    width: card.width
    height: card.height

    // Nền Card, viền 1px Sub Secondary, bo 8 — như menu của theme SDDM và
    // popup của fcitx5.
    FontMetrics {
        id: labelMetrics
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 24
    }

    Rectangle {
        id: card
        // 66 = lề thẻ 6+6, lề dòng 12, icon 20, khoảng 10, lề phải 12.
        width: Math.max(180, Math.ceil(Math.max(...root.actions.map(a => labelMetrics.advanceWidth(a.label)))) + 66)
        height: list.implicitHeight + 12
        radius: 8
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        // Hút cú bấm vào chỗ trống trong thẻ — không thì nó lọt xuống lớp
        // phủ (PopupLayer) và đóng popup.
        MouseArea { anchors.fill: parent }


        Column {
            id: list
            x: 6
            y: 6
            width: parent.width - 12

            Repeater {
                model: root.actions

                // Dòng như SamhieMenuItem của SDDM (cao 40, icon 20, nền
                // Secondary khi rê chuột) nhưng chữ Dongle 24px thay vì 28.
                delegate: Rectangle {
                    id: row
                    required property var modelData
                    required property int index
                    readonly property bool isArmed: root.armed === index

                    width: list.width
                    height: 40
                    radius: 6
                    color: rowHover.hovered ? Theme.c.secondary : "transparent"

                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10

                        SamhieIcon {
                            anchors.verticalCenter: parent.verticalCenter
                            iconName: row.modelData.icon
                            color: row.isArmed ? Theme.c.danger : Theme.c.text
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: row.isArmed ? row.modelData.confirm : row.modelData.label
                            color: row.isArmed ? Theme.c.danger : Theme.c.text
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pixelSize: 24  // 28 thấy to (user 2026-10-02)
                        }
                    }

                    HoverHandler { id: rowHover }
                    TapHandler { onTapped: root.run(row.index) }
                }
            }
        }
    }
}
