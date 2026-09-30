// Ô nhập kiểu ô mật khẩu của hyprlock: 320x52, bo 12, nền Card, viền 1px.
import QtQuick
import QtQuick.Controls.Basic
import "colors.js" as Colors

TextField {
    id: field
    property color borderColor: Colors.dark.subSecondary

    anchors.horizontalCenter: parent.horizontalCenter
    width: 320
    height: 52
    leftPadding: 18
    rightPadding: 18
    verticalAlignment: TextInput.AlignVCenter

    color: Colors.dark.text
    placeholderTextColor: Colors.dark.textMuted
    selectionColor: Colors.dark.selection
    selectedTextColor: Colors.dark.onSelection
    // Dấu chấm mật khẩu của Dongle chỉ cao 3px (đo trong máy ảo 2026-09-30)
    // → ô mật khẩu ĐANG CÓ CHỮ dùng Noto Sans; chữ mờ "Password" vẫn Dongle.
    readonly property bool dots: echoMode === TextInput.Password && text !== ""
    font.family: dots ? "Noto Sans" : "Dongle"
    font.weight: dots ? Font.Normal : Font.Light
    font.pixelSize: dots ? 14 : 30
    font.letterSpacing: dots ? 4 : 0
    passwordCharacter: "●"

    background: Rectangle {
        radius: 12
        color: Colors.dark.card
        border.width: 1
        border.color: field.borderColor
    }
}
