// Cửa sổ đang focus, bên phải cụm workspace:
//     [icon Konsole        ˅]  ⊖ ⤢ ⊗
// Ô tên như combobox: bấm xổ danh sách mọi cửa sổ (WindowList.qml); nút thu
// nhỏ, phóng to / thu về, đóng. Hyprland không vẽ thanh tiêu đề, nên app KDE
// (Dolphin, Konsole) không có nút đóng nào; Edge/VS Code tự vẽ nút của chúng
// (user gặp trong máy ảo 2026-09-28, chọn làm trên thanh trên 2026-10-02).
//
// Ô tên RỘNG CỐ ĐỊNH (vừa "Microsoft Edge") để tên dài/ngắn không làm ba nút
// nhảy chỗ (user 2026-10-02); tên dài hơn thì cắt "…".
//
// Chỉ coi là "đang focus" khi cửa sổ nằm ở màn ảo đang xem của màn hình này:
// sang màn ảo trống thì Hyprland có thể vẫn giữ cửa sổ cũ là active. Không có
// cửa sổ nào đang focus (màn trống, hoặc thu nhỏ hết) → ô ghi "Desktop", ẩn
// ba nút, vẫn bấm xổ danh sách được (cách lấy lại cửa sổ đã thu nhỏ). Không
// còn cửa sổ nào cả → ẩn hết.
//
// Trạng thái phóng to đọc từ lastIpcObject.fullscreen (1 = maximized) — chỉ
// mới sau khi hỏi lại Hyprland, nên hỏi lại mỗi lần có sự kiện "fullscreen"
// hoặc đổi cửa sổ.
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Widgets
import ".."
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

    readonly property int iconSize: 18
    readonly property int pad: 8

    function act(fn) {
        bar.popup = ""
        fn(window)
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

    // Bề rộng ô tên: đo chữ "Microsoft Edge" bằng đúng font của thanh.
    TextMetrics {
        id: widest
        text: "Microsoft Edge"
        font: title.font
    }

    visible: Windows.windows.length > 0
    implicitWidth: row.implicitWidth
    implicitHeight: bar.height

    Row {
        id: row
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        Rectangle {
            id: picker
            readonly property bool lit: pickHover.hovered || root.bar.popup === "windows"

            width: root.pad + root.iconSize + 8 + Math.ceil(widest.width) + 8 + 16 + root.pad
            height: 26
            radius: 6
            color: lit ? root.bar.hoverFill : "transparent"

            IconImage {
                id: appIcon
                visible: root.window !== null
                anchors.verticalCenter: parent.verticalCenter
                x: root.pad
                implicitSize: root.iconSize
                source: Quickshell.iconPath(root.entry ? root.entry.icon : root.appId,
                                            "application-x-executable")
            }

            Text {
                id: title
                anchors.verticalCenter: parent.verticalCenter
                x: root.window ? appIcon.x + root.iconSize + 8 : root.pad
                width: arrow.x - 8 - x
                elide: Text.ElideRight
                // Tên app như thanh trên Ubuntu; app không có file .desktop
                // thì lấy tiêu đề cửa sổ.
                text: !root.window ? "Desktop"
                    : root.entry && root.entry.name !== "" ? root.entry.name
                    : root.window.title
                color: picker.lit ? root.bar.hoverColor : root.bar.textColor
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pointSize: Colors.fonts.size
            }

            SamhieIcon {
                id: arrow
                anchors.verticalCenter: parent.verticalCenter
                x: parent.width - root.pad - width
                iconName: "arrow-down"
                size: 16
                color: picker.lit ? root.bar.hoverColor : root.bar.textColor
            }

            HoverHandler { id: pickHover }
            TapHandler {
                onTapped: {
                    root.bar.windowListX = picker.mapToItem(null, 0, 0).x
                    root.bar.toggle("windows")
                }
            }
        }

        WindowButton {
            iconName: "window-minimize-symbolic"
            onClicked: root.act(t => Windows.minimize(t))
        }

        WindowButton {
            iconName: root.maximized ? "window-restore-symbolic" : "window-maximize-symbolic"
            onClicked: root.act(t => Windows.toggleMaximize(t))
        }

        WindowButton {
            iconName: "window-close-symbolic"
            onClicked: root.act(t => Windows.close(t))
        }
    }

    // Nút như QuickButton: nền bo mờ khi rê chuột, icon sáng nhẹ.
    component WindowButton: Rectangle {
        id: btn
        property string iconName
        signal clicked()

        visible: root.window !== null
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
