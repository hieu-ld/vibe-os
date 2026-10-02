// Cửa sổ đang focus, bên phải cụm workspace: icon + tên app, nút phóng to /
// thu về và nút đóng. Hyprland không vẽ thanh tiêu đề, nên app KDE (Dolphin,
// Konsole) không có nút đóng nào; Edge/VS Code tự vẽ nút của chúng (user gặp
// trong máy ảo 2026-09-28, chọn thử trên thanh trên 2026-10-02).
//
// Không có nút thu nhỏ: Hyprland không có trạng thái "thu nhỏ xuống thanh
// tác vụ" (icon `minus` user vẽ còn để dành).
//
// Chỉ hiện trên thanh của màn hình chứa cửa sổ đó, và chỉ khi cửa sổ nằm ở
// màn ảo đang xem: sang màn ảo trống thì Hyprland có thể vẫn giữ cửa sổ cũ
// là "đang focus".
//
// Phóng to = fullscreen kiểu "maximized" của Hyprland: kín vùng làm việc,
// thanh trên vẫn thấy. Trạng thái đọc từ lastIpcObject.fullscreen (1 =
// maximized) — chỉ mới sau khi hỏi lại Hyprland, nên hỏi lại mỗi lần có sự
// kiện "fullscreen" hoặc đổi cửa sổ.
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Widgets
import "../colors.js" as Colors

Item {
    id: root

    required property var bar

    readonly property HyprlandMonitor monitor: Hyprland.monitorFor(bar.screen)
    readonly property HyprlandToplevel window: {
        const t = Hyprland.activeToplevel
        if (!t || !monitor || !t.workspace || t.workspace !== monitor.activeWorkspace)
            return null
        return t
    }
    readonly property string appId: window && window.wayland ? window.wayland.appId : ""
    readonly property var entry: appId !== "" ? DesktopEntries.heuristicLookup(appId) : null
    readonly property bool maximized: window !== null && window.lastIpcObject
        && (window.lastIpcObject.fullscreen ?? 0) === 1

    function dispatch(cmd) {
        if (!window)
            return
        bar.popup = ""
        Hyprland.dispatch("hl.dsp.window." + cmd.replace("%w", "address:0x" + window.address))
    }
    function toggleMaximize() {
        dispatch('fullscreen({ window = "%w", mode = "maximized", action = "toggle" })')
    }

    Timer {
        id: refresh
        interval: 50
        onTriggered: Hyprland.refreshToplevels()
    }
    Connections {
        target: Hyprland
        function onRawEvent(event) {
            if (event.name === "fullscreen" || event.name === "activewindowv2")
                refresh.restart()
        }
    }

    visible: window !== null
    implicitWidth: row.implicitWidth
    implicitHeight: bar.height

    Row {
        id: row
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        // Icon + tên: bấm đúp phóng to / thu về như thanh tiêu đề Windows.
        Item {
            width: title.x + title.width + 8
            height: root.height

            IconImage {
                id: appIcon
                anchors.verticalCenter: parent.verticalCenter
                implicitSize: 18
                source: Quickshell.iconPath(root.entry ? root.entry.icon : root.appId,
                                            "application-x-executable")
            }

            Text {
                id: title
                anchors.verticalCenter: parent.verticalCenter
                x: appIcon.width + 8
                width: Math.min(implicitWidth, 280)
                elide: Text.ElideRight
                // Tên app như thanh trên Ubuntu; app không có file .desktop
                // thì lấy tiêu đề cửa sổ.
                text: root.entry && root.entry.name !== "" ? root.entry.name
                    : (root.window ? root.window.title : "")
                color: root.bar.textColor
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pointSize: Colors.fonts.size
            }

            TapHandler {
                onDoubleTapped: root.toggleMaximize()
            }
        }

        WindowButton {
            iconName: root.maximized ? "window-restore-symbolic" : "window-maximize-symbolic"
            onClicked: root.toggleMaximize()
        }

        WindowButton {
            iconName: "window-close-symbolic"
            onClicked: root.dispatch('close({ window = "%w" })')
        }
    }

    // Nút như QuickButton: nền bo mờ khi rê chuột, icon sáng nhẹ.
    component WindowButton: Rectangle {
        id: btn
        property string iconName
        signal clicked()

        width: 30
        height: 26
        radius: 6
        color: hover.hovered ? root.bar.hoverFill : "transparent"

        SamhieIcon {
            anchors.centerIn: parent
            iconName: btn.iconName
            size: 20
            color: hover.hovered ? root.bar.hoverColor : root.bar.textColor
        }

        HoverHandler { id: hover }
        TapHandler { onTapped: btn.clicked() }
    }
}
