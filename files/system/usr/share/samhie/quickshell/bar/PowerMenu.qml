// Menu nguồn, mở dưới nút nguồn, mép phải thẳng nút. Năm mục như bản 1.0
// (user chốt 2026-09-11): Lock / Sleep / Restart / Shut Down / Log Out.
//
// Lock và Sleep làm ngay (quay lại được, Windows cũng vậy). Ba mục còn lại
// làm mất việc đang mở nên phải bấm HAI lần: lần đầu dòng đổi sang chữ đỏ
// "Click again to …", lần hai mới chạy; để yên 4 giây thì tự về như cũ.
// (Bản 1.0 có màn hình xác nhận của KDE; Hyprland không có.)
//
// Lệnh: logind cho phép phiên đang ngồi trước máy tự tắt/khởi động lại/ngủ
// mà không hỏi mật khẩu. Đăng xuất = thoát Hyprland, giống phím Win+Shift+M.
import QtQuick
import Quickshell
import Quickshell.Hyprland
import "../colors.js" as Colors

Item {
    id: root

    required property var bar
    readonly property bool open: bar.popup === "power"

    readonly property var actions: [
        { label: "Lock",      icon: "system-lock-screen", confirm: "",                         cmd: ["loginctl", "lock-session"] },
        { label: "Sleep",     icon: "system-suspend",     confirm: "",                         cmd: ["systemctl", "suspend"] },
        { label: "Restart",   icon: "system-reboot",      confirm: "Click again to restart",   cmd: ["systemctl", "reboot"] },
        { label: "Shut Down", icon: "system-shutdown",    confirm: "Click again to shut down", cmd: ["systemctl", "poweroff"] },
        { label: "Log Out",   icon: "system-log-out",     confirm: "Click again to log out",   cmd: [] }
    ]

    // Dòng đang chờ bấm lần hai (-1 = không có).
    property int armed: -1
    onOpenChanged: armed = -1
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

    visible: open
    width: card.width
    height: card.height

    // Nền Card, viền 1px Sub Secondary, bo 8 — như menu của theme SDDM và
    // popup của fcitx5.
    Rectangle {
        id: card
        width: 250
        height: list.implicitHeight + 12
        radius: 8
        color: Colors.dark.card
        border.width: 1
        border.color: Colors.dark.subSecondary

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
                    color: rowHover.hovered ? Colors.dark.secondary : "transparent"

                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10

                        SamhieIcon {
                            anchors.verticalCenter: parent.verticalCenter
                            iconName: row.modelData.icon
                            color: row.isArmed ? Colors.dark.danger : Colors.dark.text
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: row.isArmed ? row.modelData.confirm : row.modelData.label
                            color: row.isArmed ? Colors.dark.danger : Colors.dark.text
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
