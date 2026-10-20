// Dock — bản A (user chọn 2026-10-20 từ ảnh phác trên hình nền thật; bản
// trước: Figma "Reminder" node 308-854, Dock.png 2026-10-01).
//   * Mặt "kính": `dock` trong 40% — tối #262626 có một lớp sáng mờ dần ở
//     nửa trên + viền 1px `dock-rim` sáng dần từ dưới lên (viền bắt sáng);
//     sáng #fafafa phẳng, viền đều (user 2026-10-20, Figma) + nét lót đen
//     mờ cho màn tổng quan (nền card sáng, kính tan vào nền). KHÔNG
//     blur/bóng đổ thật: sau dock gần như luôn là đáy đen phẳng
//     của hình nền (đo: núi cũ #0b0f0f, logo 2026-10-20 #060b13 + vài ngôi
//     sao), dock lại tự ẩn khi cửa sổ đè lên — blur không có gì để làm mờ,
//     bóng đổ không thấy (phác 2026-10-20).
//   * Cao 60 = viền 1 + lề 8 + ô 40 + khe 3 + chấm 4 + lề 3 + viền 1; lề
//     trái/phải 8, khe giữa các ô 8. 4 app = 200×60.
//   * Ô 40 luôn hiện: gradient `dock-tile-top` → `-bottom`, viền
//     `dock-tile-rim` (tối: sáng dần như mặt dock; sáng: đục), bo 10. Rê chuột: ô nhích lên 2px; đang
//     bấm thì hạ về chỗ cũ. Chấm đứng yên dưới ô.
//   * Logo 32 giữa ô; mọi logo hình rộng ~28 (icon Samhie: 28×26, thấp hơn
//     2px là user cố ý — xem iconSizes).
//
// Dùng ở hai chỗ, cùng một vị trí (giữa, cách đáy 8): màn tổng quan
// (overview/Overview.qml) và desktop (DesktopDock.qml).
//
// Bấm như dock Ubuntu (user chốt 2026-10-02):
//   * app chưa chạy → mở app;
//   * app đang chạy, không phải cửa sổ đang focus → đưa cửa sổ dùng gần nhất
//     của nó ra (đang thu nhỏ thì hiện lại, ở màn ảo khác thì nhảy sang);
//   * app ĐANG focus: một cửa sổ → thu nhỏ; nhiều cửa sổ → sang cửa sổ kế;
//   * chuột giữa → luôn mở cửa sổ mới.
// Chấm dưới icon: mỗi cửa sổ một chấm (tối đa 3), `dock-dot-active` khi app
// đang focus.
//
// Icon bánh răng (Settings) trong bản vẽ TẠM ẨN (user chốt 2026-10-01): image
// chưa có app cài đặt. Có rồi thì thêm id vào `pinned`.
import QtQuick
import Quickshell
import Quickshell.Widgets
import ".."
import "../colors.js" as Colors

Item {
    id: root

    // id của file .desktop (không có đuôi). App nào chưa cài thì bỏ qua.
    property var pinned: ["org.kde.dolphin", "org.kde.konsole", "org.kde.kate", "com.microsoft.Edge"]
    // Đọc qua applications.values (không dùng byId) để danh sách TỰ CẬP NHẬT:
    // lúc dock dựng mà Quickshell chưa đọc xong file .desktop thì byId trả
    // null và dock trống luôn (đo 2026-10-01).
    readonly property var apps: {
        const all = DesktopEntries.applications.values
        return pinned.map(id => all.find(e => e.id === id)).filter(e => e)
    }

    signal launched()

    function click(entry, button) {
        const wins = Windows.windowsOf(entry)
        if (button === Qt.MiddleButton || wins.length === 0) {
            entry.execute()
        } else {
            const at = wins.indexOf(Windows.focused)
            if (at < 0)
                Windows.activate(wins[0])
            else if (wins.length === 1)
                Windows.minimize(wins[0])
            else
                Windows.activate(wins[(at + 1) % wins.length])
        }
        launched()
    }

    readonly property int cell: 40
    readonly property int gap: 8
    readonly property int side: 8
    readonly property int cellTop: 9      // viền 1 + lề 8
    readonly property int dotTop: 52      // ô 9..49, khe 3
    readonly property int iconSize: 32
    // Hình mọi logo rộng ~28 như icon Samhie (khung 32, hình 2..30). Icon app
    // khác lề mỗi cái một khác (đo 2026-10-20): kate.svg (hicolor) vẽ 34 ra
    // hình 28,5 → 33 (27,7); Edge Flatpak không lề, vẽ 30 ra 30 → 28. Đã thử
    // cho hai logo tròn to hơn (30, bù mắt): lấn át thư mục — user muốn đều.
    readonly property var iconSizes: ({ "org.kde.kate": 33, "com.microsoft.Edge": 28 })

    readonly property color white: Colors.shared.white
    readonly property color black: Colors.shared.black

    width: 2 * side + apps.length * cell + Math.max(0, apps.length - 1) * gap
    height: 60

    // Mặt kính + viền bắt sáng (Canvas: viền gradient — Rectangle chỉ có viền
    // một màu). Vẽ lại khi đổi cỡ/màu.
    component Glass: Canvas {
        property real radius
        property color fillTop
        property color fillBottom: fillTop
        property color sheen: "transparent"   // lớp sáng, mờ dần tới giữa
        property color edge: "transparent"    // nét lót dưới viền (bản sáng)
        property color rimTop
        property color rimBottom
        readonly property string look: [width, height, radius, fillTop, fillBottom, sheen, edge, rimTop, rimBottom].join()
        onLookChanged: requestPaint()

        function css(c) {
            return `rgba(${Math.round(c.r * 255)},${Math.round(c.g * 255)},${Math.round(c.b * 255)},${c.a})`
        }
        function vertical(ctx, a, b, mid) {
            const g = ctx.createLinearGradient(0, 0, 0, height)
            g.addColorStop(0, css(a))
            g.addColorStop(mid ?? 1, css(b))
            return g
        }
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.beginPath()
            ctx.roundedRect(0, 0, width, height, radius, radius)
            ctx.fillStyle = vertical(ctx, fillTop, fillBottom)
            ctx.fill()
            ctx.fillStyle = vertical(ctx, sheen, Qt.alpha(sheen, 0), 0.5)
            ctx.fill()
            // Nét 1px nằm trọn trong mép: đường đi lùi vào nửa điểm ảnh.
            ctx.beginPath()
            ctx.roundedRect(0.5, 0.5, width - 1, height - 1, radius - 0.5, radius - 0.5)
            ctx.lineWidth = 1
            ctx.strokeStyle = css(edge)
            ctx.stroke()
            ctx.strokeStyle = vertical(ctx, rimTop, rimBottom)
            ctx.stroke()
        }
    }

    Glass {
        anchors.fill: parent
        radius: 14
        fillTop: Qt.alpha(Theme.c.dock, 0.40)
        sheen: Theme.light ? "transparent" : Qt.alpha(root.white, 0.06)
        edge: Theme.light ? Qt.alpha(root.black, 0.08) : "transparent"
        rimTop: Qt.alpha(Theme.c.dockRim, 0.20)
        rimBottom: Qt.alpha(Theme.c.dockRim, Theme.light ? 0.20 : 0.04)
    }

    Row {
        x: root.side
        spacing: root.gap

        Repeater {
            model: root.apps

            delegate: Item {
                id: slot
                required property var modelData
                // Cửa sổ của app, dùng gần nhất trước.
                readonly property var windows: Windows.windowsOf(modelData)
                readonly property bool focused: windows.indexOf(Windows.focused) >= 0
                width: root.cell
                height: root.height

                Item {
                    width: root.cell
                    height: root.cell
                    y: root.cellTop - (slotArea.containsMouse && !slotArea.pressed ? 2 : 0)
                    Behavior on y {
                        NumberAnimation { duration: 120; easing.type: Easing.OutCubic }
                    }

                    Glass {
                        anchors.fill: parent
                        radius: 10
                        fillTop: Theme.c.dockTileTop
                        fillBottom: Theme.c.dockTileBottom
                        rimTop: Theme.light ? Theme.c.dockTileRim : Qt.alpha(Theme.c.dockTileRim, 0.18)
                        rimBottom: Theme.light ? Theme.c.dockTileRim : Qt.alpha(Theme.c.dockTileRim, 0.04)
                    }
                    IconImage {
                        anchors.centerIn: parent
                        implicitSize: root.iconSizes[slot.modelData.id] ?? root.iconSize
                        source: Quickshell.iconPath(slot.modelData.icon, "application-x-executable")
                    }
                }
                // Mỗi cửa sổ một chấm, giữa dưới ô.
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: root.dotTop
                    spacing: 3
                    Repeater {
                        model: Math.min(slot.windows.length, 3)
                        delegate: Rectangle {
                            width: 4
                            height: 4
                            radius: 2
                            color: slot.focused ? Theme.c.dockDotActive : Theme.c.dockDot
                        }
                    }
                }
                MouseArea {
                    id: slotArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.MiddleButton
                    onClicked: mouse => root.click(slot.modelData, mouse.button)
                }
            }
        }
    }
}
