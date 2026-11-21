// Menu chuột phải của icon dock / ô kết quả tìm app (user 2026-11-21): New
// Window · Pin to Dock / Unpin from Dock. Mỗi màn hình một lớp, chỉ hiện khi
// DockPins.menu thuộc màn này.
//
// Lớp phủ trong suốt CẢ màn hình (như bar/PopupLayer.qml, cùng lý do: không
// dùng PopupWindow + grab — popup kẹt, app mất bàn phím, đo 2026-10-01):
// bấm chỗ trống / Esc là đóng. ExclusionMode.Ignore để lớp phủ cả thanh trên
// → toạ độ trong lớp = toạ độ trong màn hình (DockPins.menu.x/y). Tầng
// Overlay: trên màn tổng quan (Top) và dock desktop.
import QtQuick
import Quickshell
import Quickshell.Wayland
import ".."
import "../colors.js" as Colors

PanelWindow {
    id: layer

    required property var shell
    // Màn hình của lớp này, truyền riêng: so với `screen` của chính cửa sổ thì
    // vòng binding (visible đổi → screen báo đổi → mine tính lại; đo 2026-11-21).
    required property var forScreen
    readonly property var menu: DockPins.menu
    readonly property bool mine: menu !== null && menu.screen === forScreen

    visible: mine
    color: "transparent"
    anchors { top: true; bottom: true; left: true; right: true }
    exclusionMode: ExclusionMode.Ignore
    WlrLayershell.namespace: "samhie-dock-menu"
    WlrLayershell.layer: WlrLayer.Overlay
    WlrLayershell.keyboardFocus: WlrKeyboardFocus.OnDemand

    // Mục menu theo app: [nhãn, việc]
    readonly property var items: {
        if (!mine)
            return []
        const e = menu.entry
        const list = [["New Window", () => { e.execute(); layer.shell.closeOverview() }]]
        if (DockPins.isPinned(e.id))
            list.push(["Unpin from Dock", () => DockPins.unpin(e.id)])
        else
            list.push(["Pin to Dock", () => DockPins.pin(e.id)])
        return list
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
        onPressed: DockPins.closeMenu()
    }
    Item {
        anchors.fill: parent
        focus: layer.mine
        Keys.onEscapePressed: DockPins.closeMenu()
    }

    Rectangle {
        id: card
        visible: layer.mine
        width: 240
        height: list.implicitHeight + 12
        radius: 8
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary
        // Neo: dock → mọc LÊN trên icon (cách 6); lưới tìm app → mọc xuống từ
        // con trỏ. Không lòi ra ngoài màn.
        x: layer.mine ? Math.round(Math.max(6, Math.min(layer.width - width - 6, layer.menu.x - (layer.menu.up ? width / 2 : 0)))) : 0
        y: layer.mine ? Math.round(Math.max(6, Math.min(layer.height - height - 6, layer.menu.up ? layer.menu.y - height - 6 : layer.menu.y))) : 0

        // Hút cú bấm vào chỗ trống trong thẻ — không thì lọt xuống lớp phủ.
        MouseArea { anchors.fill: parent; acceptedButtons: Qt.LeftButton | Qt.RightButton }

        Column {
            id: list
            x: 6
            y: 6
            width: parent.width - 12

            Repeater {
                model: layer.items
                delegate: Rectangle {
                    id: row
                    required property var modelData
                    width: list.width
                    height: 40
                    radius: 6
                    color: rowHover.hovered ? Theme.c.secondary : "transparent"
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: row.modelData[0]
                        color: Theme.c.text
                        font.family: Colors.fonts.ui
                        font.weight: Colors.fonts.uiWeight
                        font.pixelSize: 24  // như menu khay / menu nguồn
                    }
                    HoverHandler { id: rowHover }
                    TapHandler {
                        onTapped: {
                            const f = row.modelData[1]
                            DockPins.closeMenu()
                            f()
                        }
                    }
                }
            }
        }
    }
}
