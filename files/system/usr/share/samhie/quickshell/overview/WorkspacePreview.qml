// Một màn ảo thu nhỏ: hình nền + cửa sổ THẬT (hình chụp trực tiếp). Dùng cho
// khung lớn giữa màn tổng quan và các ô nhỏ bên trái.
//
// Hai cách đặt cửa sổ:
//   - ô nhỏ (spread: false): đúng vị trí trên màn hình, nhân cùng tỉ lệ.
//   - khung lớn (spread: true): DÀN THÀNH HÀNG như GNOME — user thấy giữ vị
//     trí thật thì chật (2026-10-01): Hyprland xếp cửa sổ kín màn hình nên
//     hình thu nhỏ cũng kín khung. Ảnh Ubuntu của user: cửa sổ thu nhỏ thêm,
//     cách nhau ~25–30px, lộ hình nền quanh; icon app đè mép dưới, tên hiện
//     khi rê chuột, nút X tròn ~38px có TÂM ở góc trên-phải cửa sổ.
//
// Vị trí/cỡ lấy từ lastIpcObject (`at`, `size` — toạ độ logic toàn màn
// hình), Overview gọi Hyprland.refreshToplevels() lúc mở. Hình chụp:
// ScreencopyView { captureSource: <HyprlandToplevel>.wayland } — ra hình cả
// cửa sổ đang ở màn ảo KHÁC (đo 2026-10-01).
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Wayland
import Quickshell.Widgets
import ".."
import "../colors.js" as Colors

ClippingRectangle {
    id: root

    required property HyprlandMonitor monitor
    required property int workspaceId
    // Cỡ logic của màn hình thật — tỉ lệ = width / screenWidth.
    required property real screenWidth
    required property string wallpaper
    property bool live: false
    property bool spread: false
    // 0 = cửa sổ đúng vị trí thật, 1 = dàn hàng (spread). Màn tổng quan chạy
    // nó 0 → 1 lúc mở: cửa sổ đi từ chỗ thật ra hàng, như Ubuntu.
    property real spreadAmount: 1
    property bool interactive: false
    // Cỡ nạp ảnh nền. Mặc định = cỡ đang vẽ (ô nhỏ nạp nhỏ cho nhẹ); khung đổi
    // cỡ liên tục (hiệu ứng mở) thì đặt cố định, không thì nạp lại mỗi khung.
    property size imageSize: Qt.size(width, height)
    // Màu lót sau ảnh nền / sau hình chụp cửa sổ khi chúng CHƯA vẽ kịp. Khung
    // lớn của màn tổng quan đặt trong suốt: lúc bắt đầu hiệu ứng nó trùng
    // khít desktop thật bên dưới, chỗ chưa vẽ kịp thì nhìn xuyên xuống đúng
    // hình đó. Máy ảo (user 2026-10-02, quay khung) lộ đúng MỘT khung màu lót
    // tối phủ cả màn — ảnh nền chưa kịp vẽ lần đầu ra màn hình.
    property color backdrop: Theme.c.bar
    property color windowBackdrop: Theme.c.secondary

    signal windowClicked(var toplevel)
    signal backgroundClicked()

    readonly property real k: width / screenWidth
    readonly property var workspace: Hyprland.workspaces.values.find(w => w.id === workspaceId) ?? null
    readonly property var toplevels: (workspace ? workspace.toplevels.values : [])
        .filter(t => t.lastIpcObject && t.lastIpcObject.at !== undefined && t.lastIpcObject.size !== undefined)

    // Dàn kiểu GNOME: thử 1..n hàng (chia lần lượt theo thứ tự đọc, cân bề
    // rộng), chọn số hàng cho cửa sổ to nhất; khe giữa cửa sổ KHÔNG co theo.
    // Cửa sổ không to quá maxScale lần cỡ thật-thu-nhỏ — chừa chỗ thoáng.
    readonly property int pad: 48
    readonly property int gap: 40
    readonly property int iconSize: 48
    readonly property real maxScale: 0.75
    readonly property var slots: spread ? layout(toplevels, width, height) : null

    function layout(list, W, H) {
        const items = list.map((t, i) => ({
            i: i,
            w: t.lastIpcObject.size[0] * k,
            h: t.lastIpcObject.size[1] * k,
            x: t.lastIpcObject.at[0],
            y: t.lastIpcObject.at[1]
        })).sort((a, b) => a.y - b.y || a.x - b.x)
        const n = items.length
        if (n === 0)
            return []
        const areaW = W - 2 * pad
        // Đáy chừa thêm nửa icon (icon đè mép dưới cửa sổ thò ra ngoài).
        const areaH = H - 2 * pad - iconSize / 2
        const total = items.reduce((s, it) => s + it.w, 0)
        let best = null
        for (let r = 1; r <= n; r++) {
            const rows = [[]]
            let acc = 0
            for (const it of items) {
                if (rows[rows.length - 1].length > 0 && rows.length < r
                        && acc + it.w / 2 > total * rows.length / r)
                    rows.push([])
                rows[rows.length - 1].push(it)
                acc += it.w
            }
            let f = maxScale
            let sumH = 0
            for (const row of rows) {
                const w = row.reduce((s, it) => s + it.w, 0)
                f = Math.min(f, (areaW - gap * (row.length - 1)) / w)
                sumH += Math.max(...row.map(it => it.h))
            }
            f = Math.min(f, (areaH - gap * (rows.length - 1)) / sumH)
            if (!best || f > best.f + 0.001)
                best = { f: f, rows: rows }
        }
        const out = []
        const f = best.f
        const totalH = best.rows.reduce((s, row) => s + f * Math.max(...row.map(it => it.h)), 0)
            + gap * (best.rows.length - 1)
        let y = pad + (areaH - totalH) / 2
        for (const row of best.rows) {
            const rowH = f * Math.max(...row.map(it => it.h))
            const rowW = row.reduce((s, it) => s + f * it.w, 0) + gap * (row.length - 1)
            let x = pad + (areaW - rowW) / 2
            for (const it of row) {
                out[it.i] = { x: Math.round(x), y: Math.round(y + (rowH - f * it.h) / 2),
                              w: Math.round(f * it.w), h: Math.round(f * it.h) }
                x += f * it.w + gap
            }
            y += rowH + gap
        }
        return out
    }

    color: root.backdrop

    Image {
        id: wallpaperImage
        anchors.fill: parent
        source: root.wallpaper
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        // Ảnh nền gốc to (4K); nạp ở cỡ cần vẽ cho nhẹ.
        sourceSize: root.imageSize
    }

    // Bấm chỗ trống (hình nền). Cửa sổ nằm trên và có MouseArea riêng, nên
    // bấm vào cửa sổ không lọt xuống đây.
    MouseArea {
        anchors.fill: parent
        onClicked: root.backgroundClicked()
    }

    // Mọi cửa sổ đã có hình chụp (ScreencopyView cần vài chục ms sau khi
    // dựng). Màn tổng quan chờ cái này rồi mới chạy hiệu ứng mở — không thì
    // khung đầu lộ ô xám trống (đo trong container 2026-10-02).
    // Đếm đủ: lúc vừa dựng Repeater chưa tạo cửa sổ nào (count 0) — không
    // so với toplevels thì "không có cửa sổ = sẵn sàng" chạy hiệu ứng quá sớm
    // (đo: khung xám). Hình nền nạp bất đồng bộ nên cũng phải chờ.
    readonly property bool ready: {
        if (wallpaperImage.status !== Image.Ready || windowsRepeater.count !== toplevels.length)
            return false
        for (let i = 0; i < windowsRepeater.count; i++) {
            const it = windowsRepeater.itemAt(i)
            if (!it || !it.captured)
                return false
        }
        return true
    }

    Repeater {
        id: windowsRepeater
        model: root.toplevels

        delegate: Item {
            id: win
            readonly property bool captured: capture.hasContent
            required property HyprlandToplevel modelData
            required property int index
            readonly property var ipc: modelData.lastIpcObject
            readonly property var slot: root.slots ? root.slots[index] : null
            readonly property bool hovered: winArea.containsMouse || close.hovered
            readonly property string appId: modelData.wayland ? modelData.wayland.appId : ""
            readonly property var entry: appId !== "" ? DesktopEntries.heuristicLookup(appId) : null

            // Đang rê chuột thì nổi lên trên cửa sổ bên cạnh (nút X, tên).
            z: hovered ? 1 : 0
            // Vị trí thật (thu theo khung) và ô dàn hàng; trộn theo spreadAmount.
            readonly property real realX: (ipc.at[0] - root.monitor.x) * root.k
            readonly property real realY: (ipc.at[1] - root.monitor.y) * root.k
            readonly property real realW: ipc.size[0] * root.k
            readonly property real realH: ipc.size[1] * root.k
            function mix(a, b) { return Math.round(a + (b - a) * root.spreadAmount) }
            x: slot ? mix(realX, slot.x) : Math.round(realX)
            y: slot ? mix(realY, slot.y) : Math.round(realY)
            width: slot ? mix(realW, slot.w) : Math.round(realW)
            height: slot ? mix(realH, slot.h) : Math.round(realH)

            ClippingRectangle {
                anchors.fill: parent
                radius: root.spread ? 8 : 2
                color: root.windowBackdrop   // chưa có hình (vừa mở / thu nhỏ)

                ScreencopyView {
                    id: capture
                    anchors.fill: parent
                    captureSource: win.modelData.wayland
                    live: root.live
                }
            }

            MouseArea {
                id: winArea
                anchors.fill: parent
                enabled: root.interactive
                hoverEnabled: true
                onClicked: root.windowClicked(win.modelData)
            }

            // Icon app đè giữa mép dưới (như Ubuntu) — biết cửa sổ nào là gì.
            IconImage {
                visible: root.spread
                opacity: root.spreadAmount
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.bottom
                implicitSize: root.iconSize
                source: Quickshell.iconPath(win.entry ? win.entry.icon : win.appId, "application-x-executable")
            }

            // Tên cửa sổ khi rê chuột: viên dưới icon.
            Rectangle {
                visible: root.interactive && win.hovered
                anchors.horizontalCenter: parent.horizontalCenter
                y: parent.height + root.iconSize / 2 + 6
                width: Math.min(titleText.implicitWidth + 24, Math.max(parent.width, 200))
                height: 30
                radius: 15
                color: Theme.c.secondary

                Text {
                    id: titleText
                    anchors.centerIn: parent
                    width: Math.min(implicitWidth, parent.width - 24)
                    elide: Text.ElideRight
                    text: win.modelData.title
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pointSize: Colors.fonts.size * 0.9
                }
            }

            // Nút X: tâm nằm đúng góc trên-phải cửa sổ (như Ubuntu).
            CloseButton {
                id: close
                visible: root.interactive && win.hovered
                x: parent.width - width / 2
                y: -height / 2
                onClicked: win.modelData.wayland.close()
            }
        }
    }

    // Nút X tròn 36px, nền Sub Secondary, dấu X màu chữ.
    component CloseButton: Rectangle {
        id: btn
        signal clicked()
        readonly property bool hovered: btnArea.containsMouse
        width: 36
        height: 36
        radius: 18
        color: hovered ? Theme.c.secondary : Theme.c.subSecondary

        // Dấu X vẽ bằng hai nét: icon window-close của bộ Samhie đã có vòng
        // tròn riêng, đặt trong nút tròn thành hai vòng.
        Repeater {
            model: [45, -45]
            delegate: Rectangle {
                required property int modelData
                anchors.centerIn: parent
                width: 14
                height: 2
                radius: 1
                rotation: modelData
                antialiasing: true
                color: Theme.c.text
            }
        }
        MouseArea {
            id: btnArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: btn.clicked()
        }
    }
}
