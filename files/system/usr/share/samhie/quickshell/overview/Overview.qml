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
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Wayland
import "../bar"
import "../dock"
import "../colors.js" as Colors

PanelWindow {
    id: root

    required property var shell
    readonly property HyprlandMonitor monitor: Hyprland.monitorFor(screen)
    readonly property bool open: monitor !== null && shell.overviewMonitor === monitor.name

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

    onOpenChanged: if (open) {
        // lastIpcObject (vị trí/cỡ cửa sổ) chỉ mới sau khi hỏi lại Hyprland.
        Hyprland.refreshToplevels()
        Hyprland.refreshWorkspaces()
        viewWs = monitor.activeWorkspace ? monitor.activeWorkspace.id : 1
    }

    visible: open
    color: Colors.dark.card
    anchors { top: true; bottom: true; left: true; right: true }
    // Như PopupLayer: vùng dành riêng 0 → né thanh trên; bàn phím OnDemand
    // (Exclusive làm Hyprland chặn cả cú bấm vào thanh — đo 2026-10-01).
    exclusionMode: ExclusionMode.Normal
    exclusiveZone: 0
    WlrLayershell.namespace: "samhie-overview"
    WlrLayershell.keyboardFocus: WlrKeyboardFocus.OnDemand

    // Dựng lại toàn bộ mỗi lần mở: ô nhỏ bên trái chụp cửa sổ MỘT lần lúc
    // dựng, đóng đi là bỏ — không tốn gì khi tổng quan đang đóng.
    Loader {
        anchors.fill: parent
        active: root.open
        sourceComponent: content
    }

    Component {
        id: content

        Item {
            id: page

            // Cỡ logic của màn hình (cả phần thanh trên) — bản vẽ tính theo nó.
            readonly property real sw: root.screen.width
            readonly property real sh: root.screen.height
            readonly property int previewTop: 24 + 40 + 48       // dưới ô tìm kiếm
            readonly property int dockSpace: 52 + 8 + Math.round(sh * 0.086)  // dock + lề đáy + khe
            readonly property int side: 12 + Math.round(sw * 0.076) + 24   // cột trái + khe
            // 0,75 như Ubuntu; màn thấp/hẹp thì co cho vừa.
            readonly property real k: Math.min(0.75, (height - previewTop - dockSpace) / sh, (width - 2 * side) / sw)
            readonly property bool searching: search.text.trim() !== ""

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
                width: 320
                height: 40
                radius: 20
                color: Colors.dark.subSecondary

                MouseArea {
                    anchors.fill: parent
                    onClicked: search.forceActiveFocus()
                }

                SamhieIcon {
                    x: 12
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "system-search"
                    size: 18
                    color: Colors.dark.textMuted
                }

                TextInput {
                    id: search
                    x: 40
                    width: parent.width - x - 16
                    anchors.verticalCenter: parent.verticalCenter
                    focus: true
                    clip: true
                    color: Colors.dark.text
                    selectionColor: Colors.dark.selection
                    selectedTextColor: Colors.dark.onSelection
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pointSize: Colors.fonts.size

                    Component.onCompleted: forceActiveFocus()

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        visible: search.text === ""
                        text: "Type to search"
                        color: Colors.dark.textMuted
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
            WorkspacePreview {
                id: preview
                visible: !page.searching
                anchors.horizontalCenter: parent.horizontalCenter
                y: page.previewTop
                width: Math.round(page.sw * page.k)
                height: Math.round(page.sh * page.k)
                radius: 20
                monitor: root.monitor
                workspaceId: root.viewWs
                screenWidth: page.sw
                wallpaper: root.wallpaper
                live: true
                spread: true
                interactive: true
                onBackgroundClicked: root.goWorkspace(root.viewWs)
                onWindowClicked: toplevel => {
                    toplevel.wayland.activate()
                    root.close()
                }
            }

            AppResults {
                id: results
                visible: page.searching
                x: preview.x
                y: preview.y
                width: preview.width
                height: preview.height
                query: search.text
                onLaunched: root.close()
            }

            // ── Cột trái: các màn ảo ───────────────────────────────────────
            Column {
                id: column
                x: 12
                // Tâm cột thẳng tâm khung lớn (bản vẽ).
                y: Math.round(preview.y + preview.height / 2 - height / 2)
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
                            border.color: thumb.viewed ? Colors.dark.accent : Colors.dark.subSecondary
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
                onLaunched: root.close()
            }
        }
    }
}
