// Một thanh trên cho một màn hình. Cao 30px — bản 1.0 là 32 (Figma khung
// 1920x1200), user thấy hơi cao (2026-10-01). Nền `bar`, chữ/icon chỉ dùng
// vai trò bar-* (xem CLAUDE.md).
import QtQuick
import Quickshell
import Quickshell.Wayland
import ".."
import "../colors.js" as Colors

PanelWindow {
    id: bar

    required property var shell   // shell.qml — trạng thái chung (màn tổng quan)

    // Popup đang mở: "", "calendar", "power", "tray", "quick", "notifications"
    // hoặc "windows". Mỗi lúc chỉ một cái;
    // vẽ trong PopupLayer — bấm ra ngoài hoặc Esc là đóng.
    property string popup: ""
    function toggle(name) {
        popup = popup === name ? "" : name
    }
    // Popup của thanh và màn tổng quan cùng tầng layer — không cho mở cùng
    // lúc kẻo cái này đè mất cái kia.
    onPopupChanged: if (popup !== "") shell.closeOverview()
    // Popup thông báo (tầng Overlay) tạm ẩn khi thanh nào đang mở popup.
    readonly property bool hasPopup: popup !== ""
    onHasPopupChanged: shell.notifs.barPopups += hasPopup ? 1 : -1
    Connections {
        target: bar.shell
        function onOverviewMonitorChanged() {
            if (bar.shell.overviewMonitor !== "")
                bar.popup = ""
        }
    }

    // Mép trái ô tên app (WindowControls) — danh sách cửa sổ xổ ngay dưới.
    property real windowListX: 0

    // Menu khay đang mở cho app nào, và tâm ngang của icon app đó trên thanh.
    property var trayItem: null
    property real trayX: 0
    function openTrayMenu(item, slot) {
        if (popup === "tray" && trayItem === item) {
            popup = ""
            return
        }
        trayItem = item
        trayX = slot.mapToItem(null, slot.width / 2, 0).x
        popup = "tray"
    }

    // Rê chuột lên chữ/icon trên thanh thì SÁNG NHẸ — phép tính KDE làm với
    // icon khay (shader icon.frag của Kirigami, highlight 0.7), bản 1.0 chép
    // y hệt cho đồng hồ và cụm workspace: mỗi kênh mũ 1 / (2·0,7 + 0,5).
    // #e8e8e8 lên ~#f2f2f2. Thanh sáng (chữ tối): mũ nghịch đảo — ĐẬM thêm.
    function lift(c) {
        const g = Theme.light ? 2 * 0.7 + 0.5 : 1 / (2 * 0.7 + 0.5)
        return Qt.rgba(Math.pow(c.r, g), Math.pow(c.g, g), Math.pow(c.b, g), c.a)
    }
    readonly property color textColor: Theme.c.barText
    readonly property color hoverColor: lift(textColor)
    // Nền bo góc mờ khi rê chuột lên nút (nút nguồn, icon khay) — như bản 1.0.
    readonly property color hoverFill: Qt.rgba(textColor.r, textColor.g, textColor.b, 0.12)

    anchors { top: true; left: true; right: true }
    implicitHeight: 30
    color: Theme.c.bar
    WlrLayershell.namespace: "samhie-bar"

    // Bấm vào chỗ trống trên thanh thì đóng popup và màn tổng quan. Nằm DƯỚI
    // các cụm (khai báo trước) nên không nuốt cú bấm của chúng.
    MouseArea {
        anchors.fill: parent
        onClicked: {
            bar.popup = ""
            bar.shell.closeOverview()
        }
    }

    Workspaces {
        id: workspaces
        bar: bar
        shell: bar.shell
        anchors.left: parent.left
        // Mép ô hover. 8 + lề trong 9 → viên thuốc ở x 17 như trước khi có ô
        // (trước là 16,5 — mỗi chấm giữa ô rộng hơn nó 5px — nay thẳng pixel).
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
    }

    // Cửa sổ đang focus: tên app (bấm ra danh sách cửa sổ) + nút thu nhỏ,
    // phóng to, đóng (chỉ hiện khi rê chuột).
    WindowControls {
        bar: bar
        anchors.left: workspaces.right
        // Ô tên app giữ nguyên x như trước khi có ô hover (đo container: 74).
        anchors.leftMargin: 9
        anchors.verticalCenter: parent.verticalCenter
    }

    Clock {
        id: clock
        bar: bar
        anchors.centerIn: parent
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        Tray { bar: bar }

        QuickButton { bar: bar }

        BellButton { bar: bar }

        PowerButton { bar: bar }
    }

    PopupLayer { bar: bar }
}
