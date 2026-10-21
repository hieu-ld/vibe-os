// Nhóm thẻ kiểu macOS: tiêu đề nhỏ (tuỳ chọn) phía trên, thẻ bo góc nền
// `secondary`, các dòng xếp dọc trong thẻ. Dòng (InfoRow…) tự vẽ vạch ngăn
// phía trên trừ dòng đầu.
//
//   Group {
//       title: "Hardware"
//       InfoRow { label: "Processor"; value: "…" }
//       InfoRow { label: "Memory"; value: "…" }
//   }
import QtQuick
import ".."
import "../colors.js" as Colors

Column {
    id: root

    property string title: ""
    default property alias rows: list.data

    width: parent.width
    spacing: 6

    Text {
        visible: root.title !== ""
        x: 8
        text: root.title
        color: Theme.c.textMuted
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 24
    }

    Rectangle {
        width: parent.width
        height: list.height
        radius: 12
        color: Theme.c.secondary

        Column {
            id: list
            width: parent.width
        }
    }
}
