// Cụm workspace (màn hình ảo) góc trái, kiểu GNOME/Ubuntu như bản 1.0:
// workspace đang đứng là viên thuốc dài, các workspace khác là chấm nhỏ.
// Bấm vào cụm là MỞ MÀN TỔNG QUAN như Ubuntu (user 2026-10-01) — nhảy sang
// màn khác thì bấm ô bên trái trong tổng quan. Lăn chuột trên cụm để đi
// tới/lui.
//
// Cả cụm là MỘT nút như "Activities" của Ubuntu (user 2026-10-09): rê chuột
// lên (hoặc màn tổng quan đang mở) thì hiện ô nền bo góc ôm mọi chấm — cùng
// cỡ/màu với ô hover các nút khác trên thanh (cao 26, bo 6, hoverFill).
//
// Workspace ĐỘNG như Ubuntu/GNOME (user chốt 2026-10-01): các workspace
// đang có + LUÔN một màn trống ở cuối, ít nhất 2 chấm (workspaceSlots()
// trong shell.qml).
//
// Lệnh nhảy phải viết kiểu Lua của Hyprland 0.56: kiểu cũ `workspace 3` báo
// lỗi "')' expected" (đo 2026-10-01 bằng hyprctl), mà
// HyprlandWorkspace.activate() của Quickshell 0.2.1 gửi đúng kiểu cũ đó —
// nên tự gửi bằng Hyprland.dispatch().
import QtQuick
import Quickshell
import Quickshell.Hyprland

Item {
    id: root

    required property var bar
    required property var shell

    // Số đo bản 1.0 (gridUnit 18): chấm 0,42 → 8, viên thuốc 1,4 → 26 (25,2
    // làm tròn chẵn cho tâm thẳng pixel), khe 0,28 → 5.
    readonly property int dot: 8
    readonly property int pill: 26
    readonly property int gap: 5
    // Lề trong ô hover hai bên chấm: 9 = lề trên/dưới ((26 − 8) / 2), chấm
    // nằm giữa ô đều bốn phía.
    readonly property int pad: 9

    readonly property HyprlandMonitor monitor: Hyprland.monitorFor(bar.screen)
    // Danh sách chung với màn tổng quan — xem workspaceSlots() trong shell.qml.
    readonly property var shown: shell.workspaceSlots(monitor)

    // Lăn chuột: sang chấm kế/trước trong cụm (kể cả chấm trống ở cuối).
    function step(delta) {
        const at = shown.findIndex(w => w.active)
        const next = shown[at + delta]
        if (next)
            go(next.id)
    }

    function go(target) {
        bar.popup = ""
        Hyprland.dispatch("hl.dsp.focus({ workspace = " + JSON.stringify(target) + " })")
    }

    implicitWidth: row.implicitWidth + 2 * pad
    implicitHeight: bar.height

    HoverHandler { id: clusterHover }

    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        height: 26
        radius: 6
        color: clusterHover.hovered || (root.monitor && root.shell.overviewMonitor === root.monitor.name)
            ? root.bar.hoverFill : "transparent"
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.shell.toggleOverview()
        onWheel: wheel => {
            const d = wheel.angleDelta.y !== 0 ? wheel.angleDelta.y : wheel.angleDelta.x
            root.step(d < 0 ? 1 : -1)
        }
    }

    Row {
        id: row
        x: root.pad
        anchors.verticalCenter: parent.verticalCenter
        spacing: root.gap

        Repeater {
            // Model là SỐ chấm, không phải mảng: mảng mới mỗi lần đổi sẽ dựng
            // lại mọi chấm và mất hiệu ứng viên thuốc giãn ra.
            model: root.shown.length

            delegate: Item {
                id: cell

                required property int index
                readonly property var ws: root.shown[index]
                readonly property bool current: ws ? ws.active : false

                width: mark.width
                height: root.height

                Rectangle {
                    id: mark
                    anchors.centerIn: parent
                    width: cell.current ? root.pill : root.dot
                    height: root.dot
                    radius: height / 2

                    // Chấm khác mờ đi, rê chuột lên CỤM thì rõ hơn; viên thuốc
                    // thì sáng nhẹ như mọi chữ trên thanh (Bar.lift).
                    color: cell.current
                        ? (clusterHover.hovered ? root.bar.hoverColor : root.bar.textColor)
                        : Qt.rgba(root.bar.textColor.r, root.bar.textColor.g, root.bar.textColor.b,
                                  clusterHover.hovered ? 0.55 : 0.32)

                    Behavior on width {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }
                    Behavior on color {
                        ColorAnimation { duration: 100 }
                    }
                }

            }
        }
    }
}
