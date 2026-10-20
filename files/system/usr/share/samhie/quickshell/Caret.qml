// Dấu nháy tự vẽ cho ô nhập chữ Dongle: `cursorDelegate: Caret { input: ô }`.
// Dấu nháy có sẵn của Qt cao bằng dòng chữ, mà dòng của Dongle rất cao (font
// "lùn", cỡ số lớn): ô tìm tổng quan ra vạch 40px = cả ô, chữ chỉ cao 13px
// (đo máy ảo 2026-10-20). Vạch cao cố định `size`, giữa ô nhập. Qt tự đặt x,
// y, chiều cao của delegate theo dòng chữ → vạch là con bên trong, dời về
// giữa. Gõ phím / dời con trỏ thì hiện lại ngay. Màn khoá (lock.qml) có bản
// riêng cùng cách làm.
import QtQuick

Item {
    id: caret

    required property Item input
    property color color
    property int size: 20       // màn khoá: 20 cho chữ 30px — giữ tỉ lệ ≈ 2/3 cỡ chữ
    property bool on: true

    width: 1
    visible: input.cursorVisible && on

    Rectangle {
        width: 1
        height: caret.size
        y: Math.round((caret.input.height - height) / 2 - caret.y)
        color: caret.color
    }

    Timer {
        id: blink
        interval: 500
        repeat: true
        running: caret.input.cursorVisible
        onTriggered: caret.on = !caret.on
    }
    Connections {
        target: caret.input
        function onCursorPositionChanged() {
            caret.on = true
            blink.restart()
        }
    }
}
