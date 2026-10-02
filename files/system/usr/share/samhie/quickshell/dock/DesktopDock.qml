// Dock trên desktop — mỗi màn hình một cái (user chốt 2026-10-01, trộn Ubuntu
// và Nobara):
//   * HIỆN khi màn ảo đang xem không có cửa sổ nào đè lên chỗ của dock — kiểu
//     "intellihide" của Ubuntu: cửa sổ cao kín màn luôn đè nên dock ẩn; cửa sổ
//     nổi nhỏ kéo xuống chạm dock cũng ẩn, kéo đi thì hiện lại.
//   * ẨN khi màn tổng quan đang mở trên màn hình này (tổng quan có dock riêng
//     ở ĐÚNG chỗ này, nên nhìn như cùng một dock).
//   * Rê chuột xuống đáy KHÔNG làm dock hiện ra.
// Không giữ chỗ (exclusiveZone 0): cửa sổ mới vẫn cao kín màn, dock tự lánh.
//
// Vị trí/cỡ cửa sổ lấy từ lastIpcObject (`at`, `size` — toạ độ logic toàn
// màn hình), chỉ mới sau khi hỏi lại Hyprland: hỏi lại mỗi khi có sự kiện cửa
// sổ/màn ảo. Kéo cửa sổ nổi thì Hyprland không báo gì → lúc màn ảo có cửa sổ
// nổi thì hỏi thêm mỗi 500 ms.
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Wayland

PanelWindow {
    id: root

    required property var shell
    readonly property HyprlandMonitor monitor: Hyprland.monitorFor(screen)
    readonly property HyprlandWorkspace workspace: monitor ? monitor.activeWorkspace : null
    // Màn tổng quan cùng màn hình (shell.qml). Theo lúc tổng quan THẬT SỰ
    // hiện (covering), không theo lúc được gọi mở — còn chờ hình chụp mà dock
    // đã tắt thì chớp (user 2026-10-02).
    property var overview: null
    readonly property bool overviewOpen: overview !== null && overview.covering

    readonly property int bottomGap: 8

    // Vùng dock trên màn hình, toạ độ logic toàn màn hình (cùng hệ với `at`).
    readonly property rect dockRect: Qt.rect(
        screen.x + (screen.width - dock.width) / 2,
        screen.y + screen.height - bottomGap - dock.height,
        dock.width, dock.height)

    // Cửa sổ của màn ảo đang xem (đã có vị trí từ Hyprland).
    readonly property var windows: (workspace ? workspace.toplevels.values : [])
        .map(t => t.lastIpcObject)
        .filter(o => o && o.at !== undefined && o.size !== undefined)

    readonly property bool covered: windows.some(o => (o.fullscreen ?? 0) > 0
        || (o.at[0] < dockRect.x + dockRect.width && o.at[0] + o.size[0] > dockRect.x
            && o.at[1] < dockRect.y + dockRect.height && o.at[1] + o.size[1] > dockRect.y))
    readonly property bool hasFloating: windows.some(o => o.floating)

    readonly property bool shown: !covered && !overviewOpen

    // Hỏi lại Hyprland, gộp các sự kiện tới cùng lúc thành một lần.
    Timer {
        id: refresh
        interval: 50
        onTriggered: Hyprland.refreshToplevels()
    }
    Connections {
        target: Hyprland
        function onRawEvent(event) {
            switch (event.name) {
            case "openwindow": case "closewindow": case "movewindowv2":
            case "changefloatingmode": case "fullscreen": case "workspacev2":
            case "focusedmonv2": case "activewindowv2": case "moveworkspacev2":
                refresh.restart()
            }
        }
    }
    Timer {
        interval: 500
        repeat: true
        running: root.hasFloating && !root.overviewOpen
        onTriggered: Hyprland.refreshToplevels()
    }
    Component.onCompleted: {
        Hyprland.refreshToplevels()
        Qt.callLater(apply)
    }

    // Hiện/ẩn: `reveal` 1 = hiện, 0 = ẩn; dock trượt xuống + mờ theo nó, cửa sổ
    // tắt hẳn (không nhận chuột) khi về 0. Đổi do cửa sổ → trượt 180 ms. Đổi do
    // mở/đóng tổng quan → đổi NGAY: tổng quan có dock riêng ở đúng chỗ này.
    // Quyết định để sau một nhịp (callLater) vì không biết Qt gọi
    // onShownChanged hay onOverviewOpenChanged trước.
    property real reveal: 0
    property double overviewChangedAt: 0
    onOverviewOpenChanged: overviewChangedAt = Date.now()
    onShownChanged: Qt.callLater(apply)
    function apply() {
        const target = shown ? 1 : 0
        if (Date.now() - overviewChangedAt < 100) {
            anim.stop()
            reveal = target
        } else if (reveal !== target) {
            anim.to = target
            anim.restart()
        }
    }
    NumberAnimation {
        id: anim
        target: root
        property: "reveal"
        duration: 180
        easing.type: Easing.OutCubic
    }

    anchors.bottom: true
    margins.bottom: bottomGap
    implicitWidth: dock.width
    implicitHeight: dock.height
    color: "transparent"
    exclusiveZone: 0
    WlrLayershell.namespace: "samhie-dock"
    WlrLayershell.layer: WlrLayer.Top
    visible: reveal > 0

    Dock {
        id: dock
        y: (1 - root.reveal) * (root.bottomGap + height)
        opacity: root.reveal
    }
}
