// Chỗ giữ cho trang chưa làm.
import QtQuick
import ".."
import "../colors.js" as Colors

Item {
    property string title
    width: parent ? parent.width : 0
    height: 200

    Text {
        anchors.centerIn: parent
        text: "Not built yet"
        color: Theme.c.textMuted
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 26
    }
}
