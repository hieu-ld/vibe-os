// Một dòng trong SamhieMenu: icon + chữ, nền Secondary khi rê chuột/đang chọn.
import QtQuick
import QtQuick.Controls.Basic
import "colors.js" as Colors

MenuItem {
    id: item

    implicitHeight: 40
    leftPadding: 12
    rightPadding: 12
    spacing: 10
    // Tên icon trong bộ Samhie (SamhieIcon), rỗng = không icon.
    property string iconName

    font.family: "Dongle"
    font.weight: Font.Light
    font.pixelSize: 28

    // Dòng "đang chọn" (người dùng / phiên hiện tại) đánh dấu bằng chữ cyan
    // thay vì ô tick.
    indicator: null
    contentItem: Row {
        spacing: item.spacing
        SamhieIcon {
            visible: item.iconName !== ""
            anchors.verticalCenter: parent.verticalCenter
            iconName: item.iconName
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: item.text
            color: item.checked ? Colors.dark.accentText : Colors.dark.text
            font: item.font
        }
    }
    background: Rectangle {
        radius: 6
        color: item.highlighted ? Colors.dark.secondary : "transparent"
    }
}
