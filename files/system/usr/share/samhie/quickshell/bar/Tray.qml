// Khay hệ thống (StatusNotifierItem): icon các app đặt vào khay, trước nút
// nguồn. Bấm trái = việc chính của app, bấm phải = menu của app (TrayMenu),
// bấm giữa = việc phụ, lăn chuột = gửi cho app.
//
// fcitx5 thì KHÔNG vẽ icon mà vẽ chữ `en`/`vn` như bản 1.0 — đo 2026-10-01
// (container, fcitx5 5.1.22): item id "Fcitx" đổi icon giữa
// `input-keyboard-symbolic` (gõ tiếng Anh) và `fcitx-unikey` (gõ Việt). Bấm
// trái vào nó là đổi qua lại, như Ctrl+Space. Kiểu gõ khác (người dùng tự
// thêm) thì không đoán chữ — hiện icon của fcitx5.
import QtQuick
import Quickshell
import Quickshell.Services.SystemTray
import Quickshell.Widgets
import "../colors.js" as Colors

Row {
    id: root

    required property var bar

    spacing: 2

    Repeater {
        // Item "Passive" là app tự nói nó đang không có gì đáng xem.
        model: SystemTray.items.values.filter(i => i.status !== Status.Passive)

        delegate: Rectangle {
            id: slot

            required property SystemTrayItem modelData
            readonly property bool menuOpen: root.bar.popup === "tray" && root.bar.trayItem === modelData
            readonly property bool lit: hover.hovered || menuOpen
            readonly property string label: modelData.id !== "Fcitx" ? ""
                : modelData.icon.indexOf("input-keyboard") >= 0 ? "en"
                : modelData.icon.indexOf("unikey") >= 0 ? "vn" : ""

            width: label !== "" ? Math.max(30, labelText.implicitWidth + 12) : 30
            height: 26
            radius: 6
            color: lit ? root.bar.hoverFill : "transparent"

            Text {
                id: labelText
                visible: slot.label !== ""
                anchors.centerIn: parent
                text: slot.label
                color: slot.lit ? root.bar.hoverColor : root.bar.textColor
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pointSize: Colors.fonts.size
            }

            IconImage {
                visible: slot.label === ""
                anchors.centerIn: parent
                implicitSize: 18
                source: slot.modelData.icon
            }

            HoverHandler { id: hover }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                onClicked: mouse => {
                    const item = slot.modelData
                    if (mouse.button === Qt.MiddleButton) {
                        root.bar.popup = ""
                        item.secondaryActivate()
                    } else if (mouse.button === Qt.RightButton || item.onlyMenu) {
                        if (item.hasMenu)
                            root.bar.openTrayMenu(item, slot)
                    } else {
                        root.bar.popup = ""
                        item.activate()
                    }
                }
                onWheel: wheel => slot.modelData.scroll(wheel.angleDelta.y, false)
            }
        }
    }
}
