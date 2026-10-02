// Màn tổng quan kiểu Ubuntu — theo bản vẽ của user (2026-10-01, khung
// 1920×1200, số đo trong docs/PLAN.md):
//   - nền `card` phủ màn hình (trừ thanh trên — thanh vẫn bấm được),
//   - ô tìm kiếm 320×40 cách thanh 24px: gõ là lọc app, kết quả thay chỗ
//     khung lớn (AppResults), Enter mở app,
//   - khung lớn = màn ảo đang xem; cửa sổ thật DÀN THÀNH HÀNG như GNOME;
//     bấm cửa sổ → nhảy tới nó; bấm chỗ trống → sang màn đó; nút X khi rê
//     chuột → đóng cửa sổ. Cỡ/khoảng cách theo ảnh Ubuntu của user (vòng 2,
//     2026-10-01 — bản đầu 0,8 lần, sát ô tìm kiếm, user thấy chật): tối đa
//     0,75 lần màn hình, cách ô tìm kiếm 48, cách dock 8,6% chiều cao màn,
//   - cột trái: các màn ảo (ô 0,076 lần màn hình) — CÙNG danh sách với chấm
//     trên thanh (workspaceSlots(): các màn đang có + một màn trống ở cuối);
//     bấm là khung lớn đổi sang màn đó, vẫn ở trong tổng quan. Ô "+" của bản
//     vẽ đã bỏ: màn trống ở cuối chính là chỗ mở màn mới (user chốt).
//   - dock dưới đáy (dock/Dock.qml — cùng dock với desktop).
// Esc: xoá chữ đang gõ, gõ trống rồi thì đóng. Mở/đóng: shell.qml.
//
// Hiệu ứng mở/đóng kiểu Ubuntu (user 2026-10-02: bản fade của Hyprland "hiện
// ra đột ngột"): `progress` 0 → 1. Khung lớn đi từ ĐÚNG chỗ màn hình thật
// (toàn màn, cửa sổ ở vị trí thật — nhìn y như desktop) thu về khung giữa,
// cửa sổ dàn ra hàng; nền, ô tìm kiếm, cột trái, dock hiện dần. Đóng thì
// chạy ngược rồi mới tắt lớp. Bấm Win liên tục: hiệu ứng đảo chiều từ chỗ
// đang chạy — không mất nhịp. Hyprland KHÔNG làm hiệu ứng cho lớp này nữa
// (layer rule no_anim trong samhie.lua).
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Wayland
import "../bar"
import "../dock"
import ".."
import "../colors.js" as Colors

PanelWindow {
    id: root

    required property var shell
    // Dock desktop cùng màn hình (shell.qml) — xem dockFromDesktop.
    property var desktopDock: null
    readonly property HyprlandMonitor monitor: Hyprland.monitorFor(screen)
    readonly property bool open: monitor !== null && shell.overviewMonitor === monitor.name
    // NHẤN Win (chưa thả): dựng sẵn ở lớp vô hình — không nhận chuột/phím —
    // để lúc thả hiệu ứng chạy ngay, khỏi chờ chụp hình cửa sổ (user
    // 2026-10-02: nhịp mở trễ hơn nhịp đóng). Tổ hợp Win+phím thì không có
    // lệnh mở; shell.qml bỏ phần dựng sẵn sau 1,5 s.
    readonly property bool preparing: monitor !== null && shell.overviewPrepare === monitor.name && !open
    // Tổng quan đang THẬT SỰ hiện (đã chạy hiệu ứng). Dock desktop tắt theo
    // cái này chứ không theo `open`: tắt lúc còn chờ hình chụp thì dock chớp
    // (user 2026-10-02).
    // Chờ hiệu ứng chạy được một đoạn: dock desktop và dock tổng quan nằm
    // trên hai lớp khác nhau, tắt cái này đúng lúc hiện cái kia thì lọt một
    // khung không có dock (đo trong container).
    readonly property bool covering: open && started && progress >= 0.2
    // Lúc mở, dock desktop đang hiện → nó ở yên, tổng quan không vẽ dock
    // riêng; không thì dock của tổng quan hiện dần theo hiệu ứng.
    property bool dockFromDesktop: false

    // Hình nền: shell.qml được chạy với SAMHIE_WALLPAPER = samhie.wallpaper
    // (samhie.lua). Đổi hình nền lúc đang chạy thì phải chạy lại Quickshell.
    readonly property string wallpaper: "file://" + (Quickshell.env("SAMHIE_WALLPAPER") || "/usr/share/backgrounds/samhie/samhie.png")

    // Màn ảo đang xem trong khung lớn.
    property int viewWs: 1

    function close() {
        shell.closeOverview()
    }
    function goWorkspace(id) {
        Hyprland.dispatch("hl.dsp.focus({ workspace = " + id + " })")
        close()
    }

    // Hỏi lại Hyprland (lastIpcObject — vị trí/cỡ cửa sổ — chỉ mới sau khi
    // hỏi), chọn màn ảo cho khung lớn. Một lần cho mỗi lần dựng.
    property bool prepared: false
    function prepare() {
        if (prepared)
            return
        prepared = true
        Hyprland.refreshToplevels()
        Hyprland.refreshWorkspaces()
        viewWs = monitor.activeWorkspace ? monitor.activeWorkspace.id : 1
    }
    onPreparingChanged: if (preparing && progress === 0) prepare()
    onVisibleChanged: if (!visible) prepared = false

    onOpenChanged: {
        if (open && progress === 0) {
            prepare()
            // Mở từ desktop: lớp trong suốt cho tới khi hình chụp cửa sổ sẵn
            // sàng (tối đa 200 ms) — xem `ready` của WorkspacePreview. Đã dựng
            // sẵn từ lúc nhấn Win thì thường sẵn rồi: chạy ngay.
            started = false
            if (loader.item && loader.item.ready) {
                run()
                return
            }
            readyTimeout.restart()
            return
        }
        run()
    }
    function run() {
        readyTimeout.stop()
        if (open && !started)
            dockFromDesktop = desktopDock !== null && desktopDock.reveal > 0.5
        started = true
        anim.to = open ? 1 : 0
        anim.restart()
    }
    property bool started: false
    Timer {
        id: readyTimeout
        interval: 200
        onTriggered: root.run()
    }

    // 0 = desktop, 1 = tổng quan mở hẳn.
    property real progress: 0
    NumberAnimation {
        id: anim
        target: root
        property: "progress"
        duration: 250
        easing.type: Easing.OutCubic
    }

    visible: open || progress > 0 || preparing
    color: Qt.rgba(Theme.c.card.r, Theme.c.card.g, Theme.c.card.b, progress)
    anchors { top: true; bottom: true; left: true; right: true }
    // Như PopupLayer: vùng dành riêng 0 → né thanh trên; bàn phím OnDemand
    // (Exclusive làm Hyprland chặn cả cú bấm vào thanh — đo 2026-10-01).
    exclusionMode: ExclusionMode.Normal
    exclusiveZone: 0
    WlrLayershell.namespace: "samhie-overview"
    // Chỉ lúc mở thật mới nhận phím/chuột. Lúc dựng sẵn (đang giữ Win) mà
    // nhận thì app đang dùng mất bàn phím, cú bấm chuột rơi vào lớp vô hình.
    WlrLayershell.keyboardFocus: open ? WlrKeyboardFocus.OnDemand : WlrKeyboardFocus.None
    mask: open ? null : noInput
    Region { id: noInput }

    // Dựng lại toàn bộ mỗi lần mở: ô nhỏ bên trái chụp cửa sổ MỘT lần lúc
    // dựng, đóng đi là bỏ — không tốn gì khi tổng quan đang đóng.
    Loader {
        id: loader
        anchors.fill: parent
        active: root.visible
        sourceComponent: content
    }

    Component {
        id: content

        Item {
            id: page

            // Chưa chạy hiệu ứng (đang chờ hình chụp / dựng sẵn): đẩy cả trang
            // ra NGOÀI màn hình. Qt vẫn vẽ nó nên ScreencopyView vẫn có hình;
            // `visible: false` hay độ mờ 0 thì Qt bỏ vẽ, không có hình (đo
            // trong container: dựng sẵn rồi mở vẫn chờ đủ 200 ms). Bản trước
            // để độ mờ 0,01: container đúng, nhưng máy ảo vẽ trang ĐẬM HẲN —
            // cửa sổ sát nhau, mất viền, nền màu bar — đúng lúc nhấn Win, cả
            // desktop "chớp" (user 2026-10-02, quay khung `share/flicker.sh`).
            transform: Translate { x: root.started ? 0 : -100000 }
            readonly property bool ready: preview.ready
            onReadyChanged: if (ready && !root.started && root.open) root.run()
            // Màn ảo không có cửa sổ: sẵn sàng ngay từ đầu, khỏi chờ.
            Component.onCompleted: if (ready && !root.started && root.open) root.run()

            // Cỡ logic của màn hình (cả phần thanh trên) — bản vẽ tính theo nó.
            readonly property real sw: root.screen.width
            readonly property real sh: root.screen.height
            readonly property int previewTop: 24 + 40 + 48       // dưới ô tìm kiếm
            readonly property int dockSpace: 52 + 8 + Math.round(sh * 0.086)  // dock + lề đáy + khe
            readonly property int side: 12 + Math.round(sw * 0.076) + 24   // cột trái + khe
            // 0,75 như Ubuntu; màn thấp/hẹp thì co cho vừa.
            readonly property real k: Math.min(0.75, (height - previewTop - dockSpace) / sh, (width - 2 * side) / sw)
            readonly property bool searching: search.text.trim() !== ""
            readonly property real p: root.progress
            function mix(a, b) { return a + (b - a) * p }

            // Bấm vào nền trống: đóng.
            MouseArea {
                anchors.fill: parent
                onClicked: root.close()
            }

            // ── Ô tìm kiếm ─────────────────────────────────────────────────
            Rectangle {
                id: searchBox
                anchors.horizontalCenter: parent.horizontalCenter
                y: 24
                opacity: page.p
                width: 320
                height: 40
                radius: 20
                color: Theme.c.subSecondary

                MouseArea {
                    anchors.fill: parent
                    onClicked: search.forceActiveFocus()
                }

                SamhieIcon {
                    x: 12
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "system-search"
                    size: 18
                    color: Theme.c.textMuted
                }

                TextInput {
                    id: search
                    x: 40
                    width: parent.width - x - 16
                    anchors.verticalCenter: parent.verticalCenter
                    focus: true
                    clip: true
                    color: Theme.c.text
                    selectionColor: Theme.c.selection
                    selectedTextColor: Theme.c.onSelection
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pointSize: Colors.fonts.size

                    Component.onCompleted: forceActiveFocus()

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        visible: search.text === ""
                        text: "Type to search"
                        color: Theme.c.textMuted
                        font: search.font
                    }

                    Keys.onEscapePressed: {
                        if (text !== "")
                            text = ""
                        else
                            root.close()
                    }
                    function accept() {
                        if (page.searching)
                            results.launchSelected()
                        else
                            root.goWorkspace(root.viewWs)
                    }
                    Keys.onReturnPressed: accept()
                    Keys.onEnterPressed: accept()
                    Keys.onUpPressed: results.move(0, -1)
                    Keys.onDownPressed: results.move(0, 1)
                    Keys.onTabPressed: results.move(1, 0)
                    Keys.onBacktabPressed: results.move(-1, 0)
                }
            }

            // ── Khung lớn: màn ảo đang xem ─────────────────────────────────
            // Chỗ cuối của khung (lúc mở hẳn); lúc progress 0 khung phủ đúng
            // màn hình thật — lớp này nằm DƯỚI thanh trên, nên lùi lên một
            // khoảng bằng thanh.
            readonly property real finalW: Math.round(sw * k)
            readonly property real finalH: Math.round(sh * k)
            readonly property real finalX: Math.round((width - finalW) / 2)

            WorkspacePreview {
                id: preview
                visible: !page.searching
                x: Math.round(page.mix(0, page.finalX))
                y: Math.round(page.mix(page.height - page.sh, page.previewTop))
                width: Math.round(page.mix(page.sw, page.finalW))
                height: Math.round(page.mix(page.sh, page.finalH))
                radius: page.mix(0, 20)
                spreadAmount: page.p
                imageSize: Qt.size(page.sw, page.sh)
                backdrop: "transparent"
                windowBackdrop: "transparent"
                monitor: root.monitor
                workspaceId: root.viewWs
                screenWidth: page.sw
                wallpaper: root.wallpaper
                live: true
                spread: true
                interactive: page.p === 1
                onBackgroundClicked: root.goWorkspace(root.viewWs)
                onWindowClicked: toplevel => {
                    toplevel.wayland.activate()
                    root.close()
                }
            }

            AppResults {
                id: results
                visible: page.searching
                x: page.finalX
                y: page.previewTop
                width: page.finalW
                height: page.finalH
                query: search.text
                onLaunched: root.close()
            }

            // ── Cột trái: các màn ảo ───────────────────────────────────────
            Column {
                id: column
                // Trượt vào từ mép trái.
                x: Math.round(page.mix(-width, 12))
                opacity: page.p
                // Tâm cột thẳng tâm khung lớn (bản vẽ).
                y: Math.round(page.previewTop + page.finalH / 2 - height / 2)
                spacing: 13

                readonly property int tw: Math.round(page.sw * 0.076)
                readonly property int th: Math.round(tw * page.sh / page.sw)
                readonly property var list: root.shell.workspaceSlots(root.monitor)

                Repeater {
                    model: column.list

                    delegate: Item {
                        id: thumb
                        required property var modelData   // { id, active, workspace }
                        readonly property bool viewed: modelData.id === root.viewWs
                        width: column.tw
                        height: column.th

                        WorkspacePreview {
                            anchors.fill: parent
                            radius: 8
                            monitor: root.monitor
                            workspaceId: thumb.modelData.id
                            screenWidth: page.sw
                            wallpaper: root.wallpaper
                            onBackgroundClicked: root.viewWs = thumb.modelData.id
                        }
                        // Màn đang xem: viền cyan (thứ đang được chọn); rê chuột:
                        // viền xám.
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -3
                            radius: 10
                            color: "transparent"
                            border.width: 2
                            border.color: thumb.viewed ? Theme.c.accent : Theme.c.subSecondary
                            visible: thumb.viewed || thumbHover.hovered
                        }
                        HoverHandler { id: thumbHover }
                    }
                }
            }

            // ── Dock ───────────────────────────────────────────────────────
            Dock {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8
                // Dock desktop đang hiện thì nó ở yên trên tổng quan
                // (DesktopDock.overviewOpen) — không vẽ dock thứ hai.
                visible: !root.dockFromDesktop
                opacity: page.p
                onLaunched: root.close()
            }
        }
    }
}
