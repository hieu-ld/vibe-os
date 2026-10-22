// Lớp trong suốt phủ màn hình (TRỪ thanh trên) khi có popup mở; popup
// (lịch, menu nguồn, menu khay) vẽ ngay trong lớp này, sát dưới thanh.
// Bấm vào chỗ trống của lớp hoặc Esc là đóng.
//
// Vì sao không dùng PopupWindow + HyprlandFocusGrab (cách đầu tiên): popup
// của Quickshell không nhận phím, thanh phải đổi sang giữ bàn phím, và
// trong máy ảo (2026-10-01) bấm ra ngoài lẫn Esc đều không đóng được — popup
// kẹt, kéo theo app mất bàn phím. Lớp phủ thì có sẵn cú bấm và bàn phím của
// chính nó, không phụ thuộc grab của Hyprland. Cái giá: cú bấm ra ngoài để
// đóng popup không lọt xuống app bên dưới (menu của nhiều hệ điều hành cũng
// vậy).
import QtQuick
import Quickshell
import Quickshell.Wayland
import "../quicksettings"

PanelWindow {
    id: layer

    required property var bar
    readonly property int gap: 6   // khe giữa đáy thanh và popup

    screen: bar.screen
    // Đóng xong vẫn hiện thêm một nhịp cho popup kịp mờ đi (PopupMotion,
    // 120 ms). Trong nhịp đó lớp vẫn nuốt cú bấm — không đáng kể.
    // `held` chỉ đổi ở đây: viết `visible: popup !== "" || closing` thì binding
    // tắt lớp TRƯỚC khi handler kịp bật `closing` → lớp tắt rồi hiện lại một
    // khung (đo container 2026-10-22: lịch nháy mất một khung lúc đóng).
    visible: held
    property bool held: false
    Connections {
        target: layer.bar
        function onPopupChanged() {
            if (layer.bar.popup !== "") {
                closingTimer.stop()
                layer.held = true
            } else {
                closingTimer.restart()
            }
        }
    }
    Timer {
        id: closingTimer
        interval: 140
        onTriggered: layer.held = false
    }
    color: "transparent"

    anchors { top: true; bottom: true; left: true; right: true }
    // Vùng dành riêng 0: Hyprland đặt lớp này NÉ vùng của thanh (exclusive
    // zone của layer-shell), nên thanh vẫn bấm được — bấm nút khác trên thanh
    // là đổi popup, bấm chỗ trống trên thanh là đóng (Bar.qml).
    exclusionMode: ExclusionMode.Normal
    exclusiveZone: 0
    WlrLayershell.namespace: "samhie-popup"
    // OnDemand, KHÔNG Exclusive: layer Exclusive làm Hyprland chặn cả cú bấm
    // vào thanh (đo 2026-10-01: popup đang mở, bấm đồng hồ/nút nguồn không
    // ăn). OnDemand vẫn nhận phím ngay khi hiện (Esc đóng, ←/→ lật tháng —
    // đo trong container) và trả bàn phím cho app khi đóng.
    WlrLayershell.keyboardFocus: WlrKeyboardFocus.OnDemand

    onVisibleChanged: if (visible) keys.forceActiveFocus()

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onPressed: layer.bar.popup = ""
    }

    Item {
        id: keys
        focus: true
        Keys.onEscapePressed: layer.bar.popup = ""
        Keys.onLeftPressed: if (layer.bar.popup === "calendar") calendar.stepMonth(-1)
        Keys.onRightPressed: if (layer.bar.popup === "calendar") calendar.stepMonth(1)
    }

    // Giữa màn hình, ngay dưới đồng hồ.
    CalendarPopup {
        id: calendar
        bar: layer.bar
        x: Math.round((layer.width - width) / 2)
        y: layer.gap
    }

    // Mép phải thẳng lề phải của cụm nút trên thanh.
    QuickSettings {
        bar: layer.bar
        x: layer.width - width - 6
        y: layer.gap
    }

    // Mép phải thẳng lề phải của cụm nút trên thanh.
    NotificationCenter {
        bar: layer.bar
        x: layer.width - width - 6
        y: layer.gap
    }

    // Mép phải thẳng lề phải của cụm nút trên thanh.
    PowerMenu {
        bar: layer.bar
        x: layer.width - width - 6
        y: layer.gap
    }

    // Mép trái thẳng ô tên app trên thanh.
    WindowList {
        bar: layer.bar
        x: layer.bar.windowListX
        y: layer.gap
    }

    // Mép phải thẳng mép phải cụm CPU/RAM vừa bấm.
    ClaudePopup {
        bar: layer.bar
        x: Math.max(6, Math.round(layer.bar.claudeX - width))
        y: layer.gap
    }

    // Giữa icon vừa bấm, không tràn khỏi màn hình.
    TrayMenu {
        bar: layer.bar
        x: Math.max(6, Math.min(layer.width - width - 6, Math.round(layer.bar.trayX - width / 2)))
        y: layer.gap
    }
}
