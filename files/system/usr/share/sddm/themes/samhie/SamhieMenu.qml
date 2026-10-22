// Menu nổi: nền Card, viền 1px Sub Secondary, bo 8 — như popup của fcitx5.
import QtQuick
import QtQuick.Controls.Basic
import "colors.js" as Colors

Menu {
    property var c: Colors.dark

    padding: 6
    implicitWidth: 200

    background: Rectangle {
        radius: 8
        color: c.card
        border.width: 1
        border.color: c.subSecondary
    }
}
