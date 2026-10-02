// Kết quả tìm app khi gõ vào ô tìm kiếm của màn tổng quan — thay chỗ khung
// màn ảo lớn (user chốt 2026-10-01, như Ubuntu). Lưới icon + tên; ô đang
// chọn (Enter mở) viền cyan, mũi tên di chuyển.
//
// Khớp: không phân biệt hoa thường và DẤU ("cai dat" khớp "Cài đặt"). Tên
// app bắt đầu bằng chữ gõ > một từ trong tên bắt đầu bằng nó > tên chứa nó >
// tên chung/từ khoá/câu mô tả chứa nó.
import QtQuick
import Quickshell
import Quickshell.Widgets
import ".."
import "../colors.js" as Colors

Item {
    id: root

    required property string query
    property int selected: 0
    readonly property int columns: Math.max(1, Math.floor(width / cell))
    readonly property int cell: 132

    signal launched()

    function fold(s) {
        return (s ?? "").toLowerCase().normalize("NFD").replace(/[̀-ͯ]/g, "").replace(/đ/g, "d")
    }

    readonly property var results: {
        const q = fold(query.trim())
        if (q === "")
            return []
        const scored = []
        for (const e of DesktopEntries.applications.values) {
            if (e.noDisplay)
                continue
            const name = fold(e.name)
            let score = 0
            if (name.startsWith(q))
                score = 4
            else if (name.split(/[\s\-_.]+/).some(w => w.startsWith(q)))
                score = 3
            else if (name.includes(q))
                score = 2
            else if ([e.genericName, e.comment].concat(e.keywords ?? []).some(t => fold(t).includes(q)))
                score = 1
            if (score > 0)
                scored.push({ entry: e, score: score })
        }
        scored.sort((a, b) => b.score - a.score || a.entry.name.localeCompare(b.entry.name))
        return scored.slice(0, 4 * columns).map(s => s.entry)
    }
    onResultsChanged: selected = 0

    function move(dx, dy) {
        if (results.length === 0)
            return
        selected = Math.max(0, Math.min(results.length - 1, selected + dx + dy * columns))
    }
    function launchSelected() {
        const e = results[selected]
        if (e) {
            e.execute()
            launched()
        }
    }

    Text {
        anchors.centerIn: parent
        visible: root.query.trim() !== "" && root.results.length === 0
        text: "No results"
        color: Theme.c.textMuted
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pointSize: Colors.fonts.size * 1.2
    }

    Grid {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 24
        columns: Math.min(root.columns, Math.max(1, root.results.length))

        Repeater {
            model: root.results

            delegate: Rectangle {
                id: tile
                required property var modelData
                required property int index
                readonly property bool isSelected: index === root.selected

                width: root.cell
                height: root.cell
                radius: 12
                color: isSelected || tileArea.containsMouse ? Theme.c.secondary : "transparent"
                border.width: isSelected ? 2 : 0
                border.color: Theme.c.accent

                IconImage {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: 16
                    implicitSize: 64
                    source: Quickshell.iconPath(tile.modelData.icon, "application-x-executable")
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: 86
                    width: parent.width - 12
                    height: 34
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    text: tile.modelData.name
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pointSize: Colors.fonts.size
                }

                MouseArea {
                    id: tileArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        root.selected = tile.index
                        root.launchSelected()
                    }
                }
            }
        }
    }
}
