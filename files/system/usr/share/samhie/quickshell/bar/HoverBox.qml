// Ô hover bo góc của nút icon trên thanh trên. Icon nào được build-icons.py
// dời nửa điểm ảnh cho sắc (icon-nudge.js) thì tâm nét lệch nửa điểm ảnh so
// với tâm nút → ô bớt 1px về phía đó (cạnh lẻ): tâm ô trùng tâm nét, icon
// vẫn ở toạ độ nguyên nên không nhoè. Đặt làm con ĐẦU TIÊN của nút (nằm dưới
// icon); nút phải cỡ chẵn và icon cỡ chẵn, đặt giữa (anchors.centerIn).
import QtQuick
import "icon-nudge.js" as IconNudge

Rectangle {
    property string iconName
    property int iconSize
    property bool lit
    required property var bar

    readonly property var half: IconNudge.half(iconName, iconSize)

    x: half.x > 0 ? 1 : 0
    y: half.y > 0 ? 1 : 0
    width: parent.width - (half.x !== 0 ? 1 : 0)
    height: parent.height - (half.y !== 0 ? 1 : 0)
    radius: 6
    color: lit ? bar.hoverFill : "transparent"
}
