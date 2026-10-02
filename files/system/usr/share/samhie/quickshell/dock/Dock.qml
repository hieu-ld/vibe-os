// Dock — bản vẽ của user (Figma "Reminder" node 308-854, Dock.png 2026-10-01):
// viên bo góc nền `bar`, viền 1px `sub-secondary`, dày 52 kể cả viền. Ô 40px
// (rê chuột thì ô hiện nền `secondary`, bo 8), cách nhau 12, lề 12; icon 32px
// giữa ô (hình 28 cách lề 2 — icon app của Samhie cắt khung đúng vậy, xem
// tools/build-app-icons.py). 4 app = 220×52 (bản vẽ 5 app = 272×52).
//
// Dùng ở hai chỗ, cùng một vị trí (giữa, cách đáy 8): màn tổng quan
// (overview/Overview.qml) và desktop (DesktopDock.qml). Bấm là mở app.
//
// Icon bánh răng (Settings) trong bản vẽ TẠM ẨN (user chốt 2026-10-01): image
// chưa có app cài đặt. Có rồi thì thêm id vào `pinned`.
import QtQuick
import Quickshell
import Quickshell.Widgets
import ".."
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
    readonly property int iconSize: 32
    // Bản vẽ: icon Samhie hình 28 (khung 32), icon app khác hình 30. Icon của
    // app khác lề trong mỗi cái một khác → cỡ vẽ riêng cho hình ra 30 (đo
    // 2026-10-01: vscode.png vẽ 32 ra hình 28 → 34; Edge Flatpak vẽ 32 ra
    // hình 32, không lề → 30).
    readonly property var iconSizes: ({ "com.microsoft.VSCode": 34, "com.microsoft.Edge": 30 })

    width: apps.length * cell + Math.max(0, apps.length - 1) * gap + 2 * gap
    height: 52
    radius: 12  // đo góc bo trên Dock.png
    color: Theme.c.bar
    border.width: 1
    border.color: Theme.c.subSecondary

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
                    radius: 8
                    color: slotArea.containsMouse ? Theme.c.secondary : "transparent"
                }
                IconImage {
                    anchors.centerIn: parent
                    implicitSize: root.iconSizes[slot.modelData.id] ?? root.iconSize
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
