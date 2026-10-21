// Hộp thoại "Keep these display settings?" của trang Display — một lớp phủ
// cả màn (layer-shell Overlay, nền tối mờ, che cả thanh trên), thẻ ở giữa:
//
//   ┌────────────────────────────────────────────┐
//   │ Keep these display settings?               │
//   │ Reverting to the previous settings in 12 s │
//   │                         [Revert]  [Keep]   │
//   └────────────────────────────────────────────┘
//
// Mỗi màn một bản (Variants ở DisplayPage). Bàn phím chỉ ở bản trên màn đang
// focus: Enter = Keep, Esc = Revert. Bấm ra ngoài thẻ KHÔNG làm gì — chỉ hai
// nút (đếm hết giờ thì tự Revert).
import QtQuick
import Quickshell
import Quickshell.Wayland
import Quickshell.Hyprland
import ".."
import "../colors.js" as Colors

PanelWindow {
    id: root

    required property var modelData
    property int countdown
    signal kept()
    signal reverted()

    readonly property bool focusedScreen: Hyprland.focusedMonitor !== null
        && Hyprland.focusedMonitor.name === modelData.name

    screen: modelData
    color: Qt.alpha(Colors.shared.black, 0.45)
    anchors { top: true; bottom: true; left: true; right: true }
    exclusionMode: ExclusionMode.Ignore
    WlrLayershell.layer: WlrLayer.Overlay
    WlrLayershell.namespace: "samhie-dialog"
    WlrLayershell.keyboardFocus: focusedScreen ? WlrKeyboardFocus.Exclusive : WlrKeyboardFocus.None

    // Nuốt cú bấm ngoài thẻ (không lọt xuống app, không đóng hộp thoại).
    MouseArea {
        anchors.fill: parent
    }

    Rectangle {
        id: card
        anchors.centerIn: parent
        width: 440
        height: body.height + 48
        radius: 12
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        Item {
            focus: true
            Keys.onReturnPressed: root.kept()
            Keys.onEnterPressed: root.kept()
            Keys.onEscapePressed: root.reverted()
            Component.onCompleted: forceActiveFocus()
        }

        Column {
            id: body
            x: 24
            y: 24
            width: parent.width - 48
            spacing: 4

            Text {
                width: parent.width
                wrapMode: Text.Wrap
                text: "Keep these display settings?"
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 30
            }
            Text {
                width: parent.width
                wrapMode: Text.Wrap
                text: "Reverting to the previous settings in " + root.countdown + " s"
                color: Theme.c.textMuted
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 24
            }
            Item {
                width: parent.width
                height: 20
            }
            Row {
                anchors.right: parent.right
                spacing: 8
                Button {
                    label: "Revert"
                    onClicked: root.reverted()
                }
                Button {
                    label: "Keep"
                    primary: true
                    onClicked: root.kept()
                }
            }
        }
    }
}
