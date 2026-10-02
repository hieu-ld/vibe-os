// Danh sách mọi cửa sổ, xổ xuống từ ô tên app trên thanh (WindowControls).
// User chốt 2026-10-02: mọi cửa sổ trên mọi màn ảo, chia nhóm "Workspace N";
// cửa sổ đã thu nhỏ hiện MỜ trong nhóm của màn nó đến từ (không rõ thì nhóm
// "Minimized" cuối danh sách). Bấm dòng = nhảy tới cửa sổ (đang thu nhỏ thì
// hiện lại); rê chuột thì hiện nút X nhỏ để đóng nhanh — danh sách vẫn mở.
// Cửa sổ đang focus có vạch nhấn bên trái.
//
// Thẻ như menu nguồn: nền Card, viền Sub Secondary, bo 8, dòng 40, chữ Dongle
// 24px. Nhiều cửa sổ quá thì cuộn (cao tối đa 600).
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Widgets
import ".."
import "../colors.js" as Colors

Item {
    id: root

    required property var bar
    readonly property bool open: bar.popup === "windows"

    // [{ title, windows: [HyprlandToplevel] }], màn ảo tăng dần.
    readonly property var groups: {
        const byWs = {}
        for (const t of Windows.windows) {
            const ws = Windows.homeWorkspace(t)
            if (!byWs[ws])
                byWs[ws] = []
            byWs[ws].push(t)
        }
        return Object.keys(byWs).map(Number).sort((a, b) => (a || 1e9) - (b || 1e9))
            .map(ws => ({ title: ws > 0 ? "Workspace " + ws : "Minimized", windows: byWs[ws] }))
    }

    visible: open
    width: card.width
    height: card.height

    Rectangle {
        id: card
        width: 340
        height: Math.min(list.implicitHeight + 12, 600)
        radius: 8
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        // Hút cú bấm vào chỗ trống trong thẻ — không thì nó lọt xuống lớp
        // phủ (PopupLayer) và đóng popup.
        MouseArea { anchors.fill: parent }

        Flickable {
            anchors.fill: parent
            anchors.margins: 6
            contentHeight: list.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            Column {
                id: list
                width: parent.width

                Repeater {
                    model: root.groups

                    delegate: Column {
                        id: group
                        required property var modelData
                        width: list.width

                        Text {
                            x: 12
                            height: 30
                            verticalAlignment: Text.AlignVCenter
                            text: group.modelData.title
                            color: Theme.c.textMuted
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pixelSize: 20
                        }

                        Repeater {
                            model: group.modelData.windows

                            delegate: Rectangle {
                                id: row
                                required property HyprlandToplevel modelData
                                readonly property bool minimized: Windows.isMinimized(modelData)
                                readonly property bool current: Windows.focused === modelData
                                readonly property string appId: modelData.wayland ? modelData.wayland.appId : ""
                                readonly property var entry: appId !== "" ? DesktopEntries.heuristicLookup(appId) : null

                                width: list.width
                                height: 40
                                radius: 6
                                color: rowHover.hovered ? Theme.c.secondary : "transparent"

                                Rectangle {
                                    visible: row.current
                                    x: 2
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 3
                                    height: 18
                                    radius: 1.5
                                    color: Theme.c.accent
                                }

                                IconImage {
                                    id: icon
                                    x: 12
                                    anchors.verticalCenter: parent.verticalCenter
                                    implicitSize: 20
                                    opacity: row.minimized ? 0.45 : 1
                                    source: Quickshell.iconPath(row.entry ? row.entry.icon : row.appId,
                                                                "application-x-executable")
                                }

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    x: icon.x + icon.width + 10
                                    width: (closeBtn.visible ? closeBtn.x - 6 : row.width - 12) - x
                                    elide: Text.ElideRight
                                    text: row.modelData.title !== "" ? row.modelData.title
                                        : row.entry ? row.entry.name : row.appId
                                    color: Theme.c.text
                                    opacity: row.minimized ? 0.45 : 1
                                    font.family: Colors.fonts.ui
                                    font.weight: Colors.fonts.uiWeight
                                    font.pixelSize: 24
                                }

                                HoverHandler { id: rowHover }
                                TapHandler {
                                    onTapped: {
                                        root.bar.popup = ""
                                        Windows.activate(row.modelData)
                                    }
                                }

                                // X nhỏ khi rê chuột: đóng cửa sổ, danh sách vẫn mở.
                                Rectangle {
                                    id: closeBtn
                                    visible: rowHover.hovered
                                    anchors.right: parent.right
                                    anchors.rightMargin: 6
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 28
                                    height: 28
                                    radius: 6
                                    color: closeHover.hovered ? Theme.c.subSecondary : "transparent"

                                    SamhieIcon {
                                        anchors.centerIn: parent
                                        iconName: "window-close-symbolic"
                                        size: 18
                                        color: Theme.c.text
                                    }

                                    HoverHandler { id: closeHover }
                                    TapHandler { onTapped: Windows.close(row.modelData) }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
