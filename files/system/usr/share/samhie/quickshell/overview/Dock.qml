// Dock dưới đáy màn tổng quan (bản vẽ của user 2026-10-01): viên bo góc nền
// `bar`, viền 1px `sub-secondary`, ô 40px cách nhau 12, lề 12 → 4 app =
// 232×52 (bản vẽ 5 app = 272×52). Bấm là mở app và đóng tổng quan.
//
// Đợt này dock CHỈ có trong tổng quan; dock luôn hiện trên desktop là mục
// "Dock nổi" riêng của giai đoạn 3. Icon bánh răng (Settings) trong bản vẽ
// chưa có: image chưa có app cài đặt nào để mở.
import QtQuick
import Quickshell
import Quickshell.Widgets
import "../colors.js" as Colors

Rectangle {
    id: root

    // id của file .desktop (không có đuôi). App nào chưa cài thì bỏ qua.
    property var pinned: ["org.kde.dolphin", "org.kde.konsole", "com.microsoft.VSCode", "com.microsoft.Edge"]
    // Đọc qua applications.values (không dùng byId) để danh sách TỰ CẬP NHẬT:
    // lúc dock dựng mà Quickshell chưa đọc xong file .desktop thì byId trả
    // null và dock trống luôn (đo 2026-10-01).
    readonly property var apps: {
        const all = DesktopEntries.applications.values
        return pinned.map(id => all.find(e => e.id === id)).filter(e => e)
    }

    signal launched()

    readonly property int cell: 40
    readonly property int gap: 12

    width: apps.length * cell + Math.max(0, apps.length - 1) * gap + 2 * gap
    height: 52
    radius: 14
    color: Colors.dark.bar
    border.width: 1
    border.color: Colors.dark.subSecondary

    Row {
        anchors.centerIn: parent
        spacing: root.gap

        Repeater {
            model: root.apps

            delegate: Item {
                id: slot
                required property var modelData
                width: root.cell
                height: root.cell

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: -2
                    radius: 8
                    color: slotArea.containsMouse ? Colors.dark.secondary : "transparent"
                }
                IconImage {
                    anchors.centerIn: parent
                    implicitSize: 36
                    source: Quickshell.iconPath(slot.modelData.icon, "application-x-executable")
                }
                MouseArea {
                    id: slotArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        slot.modelData.execute()
                        root.launched()
                    }
                }
            }
        }
    }
}
