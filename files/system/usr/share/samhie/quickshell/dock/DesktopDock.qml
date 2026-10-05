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
    // Màn tổng quan cùng màn hình (shell.qml). Lúc mở mà dock này đang hiện
    // (overview.dockFromDesktop) thì dock này Ở YÊN suốt lúc tổng quan mở —
    // tổng quan không vẽ dock riêng. Đổi dock (tắt cái này, hiện cái kia)
    // thì máy ảo lọt một khung không có dock nào (user 2026-10-02, quay
    // khung): hai lớp khác nhau, cái mới vẽ chậm hơn một nhịp. Dock đang ẩn
    // (cửa sổ che) thì tổng quan tự hiện dock của nó, dock này giữ ẩn.
    property var overview: null
    readonly property bool overviewOpen: overview !== null && overview.covering && !overview.dockFromDesktop

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
    // Bottom (DƯỚI cửa sổ): menu chuột phải/tooltip của app là popup của cửa
    // sổ — Hyprland vẽ cùng tầng cửa sổ, dưới Top/Overlay — nên dock tầng
    // Overlay đè lên menu mở sát đáy màn (user 2026-10-05). Hyprland chỉ đẩy
    // popup lên khỏi vùng GIỮ CHỖ (Popup.cpp: logicalBoxMinusReserved), mà
    // dock không giữ chỗ. Dock chỉ hiện khi không cửa sổ nào đè chỗ của nó
    // (covered), nên nằm dưới cửa sổ nhìn vẫn y như cũ.
    // Overlay khi màn tổng quan đang dựng/mở: dock phải nằm TRÊN lớp tổng
    // quan (Top) — xem overviewOpen. Đổi tầng từ lúc nhấn Win (dựng sẵn,
    // lớp tổng quan còn trong suốt) nên kịp trước khi tổng quan hiện.
    WlrLayershell.layer: overview !== null && overview.visible ? WlrLayer.Overlay : WlrLayer.Bottom
    visible: reveal > 0

    Dock {
        id: dock
        y: (1 - root.reveal) * (root.bottomGap + height)
        opacity: root.reveal
        // Đang ở trong tổng quan (dock ở yên — xem overviewOpen): bấm app
        // thì đóng tổng quan như dock riêng của nó.
        onLaunched: root.shell.closeOverview()
    }
}
