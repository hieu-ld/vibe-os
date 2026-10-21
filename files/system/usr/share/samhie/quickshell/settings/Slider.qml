// Thanh trượt — cùng hình với thanh âm lượng quick settings (VolumeRow.qml):
// rãnh `sub-secondary` cao 4, phần đã chạy `accent`, núm tròn 14. Núm bản
// sáng trắng viền xám (vai trò slider-knob*), bản tối `text` không viền.
// Kéo / bấm / lăn chuột; `value` 0…1 do nơi dùng giữ, chỉ báo `moved(v)`.
import QtQuick
import ".."

Item {
    id: root

    property real value: 0
    property bool dimmed: false     // tắt tiếng: phần đã chạy mờ đi
    readonly property alias pressed: mouse.pressed
    signal moved(real v)

    width: 240
    height: 24

    readonly property real shown: Math.max(0, Math.min(1, value))

    function set(v) {
        moved(Math.max(0, Math.min(1, v)))
    }

    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        height: 4
        radius: 2
        color: Theme.c.subSecondary
    }
    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        width: Math.max(4, parent.width * root.shown)
        height: 4
        radius: 2
        color: root.dimmed ? Theme.c.textMuted : Theme.c.accent
    }
    Rectangle {
        width: 14
        height: 14
        radius: 7
        anchors.verticalCenter: parent.verticalCenter
        x: Math.round(root.shown * (parent.width - width))
        color: !Theme.light ? Theme.c.text
            : mouse.pressed ? Theme.c.sliderKnobPressed : Theme.c.sliderKnob
        border.width: Theme.light ? 1 : 0
        border.color: mouse.pressed ? Theme.c.sliderKnobRimPressed : Theme.c.sliderKnobRim
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        onPressed: e => root.set(e.x / width)
        onPositionChanged: e => { if (pressed) root.set(e.x / width) }
        onWheel: w => root.set(root.value + (w.angleDelta.y > 0 ? 0.05 : -0.05))
    }
}
