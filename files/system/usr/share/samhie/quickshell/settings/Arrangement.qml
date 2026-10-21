// Ô xếp chỗ màn hình của trang Display: mỗi màn đang bật là một hình chữ
// nhật theo cỡ LOGIC (sau scale, xoay), thu nhỏ cho vừa ô. Bấm = chọn màn
// (phần cài đặt bên dưới đổi theo); kéo thả = dời màn, thả ra thì dính vào
// mép màn gần nhất (trái/phải/trên/dưới, thẳng mép trên/trái nếu gần), rồi
// dồn cả cụm về góc 0,0. Báo `moved({ tên: "XxY" })` cho mọi màn.
import QtQuick
import ".."
import "../colors.js" as Colors

Item {
    id: root

    property var monitors: []
    property string selected
    property bool locked: false
    signal picked(string name)
    signal moved(var positions)

    function size(m) {
        const rot = m.transform % 2 === 1
        return { w: Math.round((rot ? m.height : m.width) / m.scale),
                 h: Math.round((rot ? m.width : m.height) / m.scale) }
    }
    readonly property var rects: monitors.map(m => {
        const s = size(m)
        return { name: m.name, label: (m.model || m.name), x: m.x, y: m.y, w: s.w, h: s.h }
    })
    readonly property var box: {
        if (rects.length === 0)
            return { x: 0, y: 0, w: 1, h: 1 }
        const x0 = Math.min(...rects.map(r => r.x)), y0 = Math.min(...rects.map(r => r.y))
        const x1 = Math.max(...rects.map(r => r.x + r.w)), y1 = Math.max(...rects.map(r => r.y + r.h))
        return { x: x0, y: y0, w: x1 - x0, h: y1 - y0 }
    }
    // Chừa chỗ để kéo màn sang bên cạnh cụm.
    readonly property real k: Math.min((width - 48) / (box.w * 1.6), (height - 32) / (box.h * 1.6))
    readonly property real ox: (width - box.w * k) / 2 - box.x * k
    readonly property real oy: (height - box.h * k) / 2 - box.y * k

    function overlaps(a, b) {
        return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h
    }
    // Chỗ dính gần (x, y) nhất cho màn `d` (cỡ logic), không chồng màn khác.
    function snap(d, x, y) {
        const others = rects.filter(r => r.name !== d.name)
        const near = Math.max(d.w, d.h) * 0.08   // gần mép thì thẳng mép
        let best = null
        for (const o of others) {
            const alignY = v => Math.abs(v - o.y) < near ? o.y
                : Math.abs(v + d.h - o.y - o.h) < near ? o.y + o.h - d.h
                : Math.max(o.y - d.h + 1, Math.min(o.y + o.h - 1, v))
            const alignX = v => Math.abs(v - o.x) < near ? o.x
                : Math.abs(v + d.w - o.x - o.w) < near ? o.x + o.w - d.w
                : Math.max(o.x - d.w + 1, Math.min(o.x + o.w - 1, v))
            for (const c of [{ x: o.x + o.w, y: alignY(y) }, { x: o.x - d.w, y: alignY(y) },
                             { x: alignX(x), y: o.y + o.h }, { x: alignX(x), y: o.y - d.h }]) {
                const r = { x: Math.round(c.x), y: Math.round(c.y), w: d.w, h: d.h }
                if (others.some(p => overlaps(r, p)))
                    continue
                const dist = Math.hypot(r.x - x, r.y - y)
                if (!best || dist < best.dist)
                    best = { x: r.x, y: r.y, dist: dist }
            }
        }
        return best
    }
    function drop(d, x, y) {
        const p = snap(d, x, y)
        if (!p)
            return
        const all = rects.map(r => r.name === d.name ? Object.assign({}, r, { x: p.x, y: p.y }) : r)
        const x0 = Math.min(...all.map(r => r.x)), y0 = Math.min(...all.map(r => r.y))
        const out = {}
        for (const r of all)
            out[r.name] = (r.x - x0) + "x" + (r.y - y0)
        moved(out)
    }

    Repeater {
        model: root.rects
        Rectangle {
            id: tile
            required property var modelData
            readonly property bool current: modelData.name === root.selected

            function home() {
                x = Qt.binding(() => root.ox + modelData.x * root.k)
                y = Qt.binding(() => root.oy + modelData.y * root.k)
            }
            Component.onCompleted: home()
            width: modelData.w * root.k
            height: modelData.h * root.k
            radius: 6
            // Màn đang chọn: nền đậm hơn + chữ rõ — không viền accent (user
            // không thích viền cyan quanh thứ đang chọn).
            color: Qt.alpha(Theme.c.text, current || drag.drag.active ? 0.18 : 0.07)
            border.width: 1
            border.color: Theme.c.subSecondary
            z: drag.drag.active ? 1 : 0

            Text {
                anchors.centerIn: parent
                width: parent.width - 12
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: tile.modelData.label
                color: tile.current ? Theme.c.text : Theme.c.textMuted
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 22
            }
            MouseArea {
                id: drag
                anchors.fill: parent
                enabled: !root.locked
                cursorShape: drag.drag.active ? Qt.ClosedHandCursor : Qt.PointingHandCursor
                drag.target: tile
                drag.threshold: 4
                onPressed: root.picked(tile.modelData.name)
                onReleased: {
                    const lx = (tile.x - root.ox) / root.k, ly = (tile.y - root.oy) / root.k
                    tile.home()
                    // Chỉ bấm (không kéo) thì thôi.
                    if (Math.abs(lx - tile.modelData.x) * root.k > 2 || Math.abs(ly - tile.modelData.y) * root.k > 2)
                        root.drop(tile.modelData, lx, ly)
                }
            }
        }
    }
}
