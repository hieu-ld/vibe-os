// Danh sách thông báo của chuông (BellButton.qml), mở dưới chuông, mép phải
// thẳng lề phải cụm nút trên thanh. Như bản 1.0 (user chốt 2026-09-13): xem
// lại thông báo đã qua, Clear All, công tắc Đừng làm phiền.
//
// Đang mở thì popup mới không bật lên (thông báo mới đã nằm ngay trong danh
// sách) — Notifications.centerOpen.
import QtQuick
import QtQuick.Controls.Basic
import Quickshell
import "../notifications"
import ".."
import "../colors.js" as Colors

PopupMotion {
    id: root

    required property var bar
    readonly property var notifs: bar.shell.notifs
    readonly property bool open: bar.popup === "notifications"

    onOpenChanged: notifs.centerOpen = open

    // Hiện/ẩn có hiệu ứng (PopupMotion).
    showing: open
    transformOrigin: Item.TopRight
    width: card.width
    height: card.height

    Rectangle {
        id: card
        width: 384
        height: header.height + body.height + 24
        radius: 12
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        // Hút cú bấm vào chỗ trống trong thẻ — không thì nó lọt xuống lớp
        // phủ (PopupLayer) và đóng popup.
        MouseArea { anchors.fill: parent }

        // ── Tiêu đề: "Notifications"  [công tắc] Do not disturb · Clear all ──
        Item {
            id: header
            x: 12
            y: 8
            width: parent.width - 24
            height: 36

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "Notifications"
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Font.Normal
                font.pixelSize: 30
            }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                // Công tắc Đừng làm phiền: bật = nền cyan, núm bên phải.
                Item {
                    width: dndRow.width
                    height: 28
                    Row {
                        id: dndRow
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 32
                            height: 18
                            radius: 9
                            color: root.notifs.doNotDisturb ? Theme.c.accent : Theme.c.secondary
                            border.width: 1
                            border.color: root.notifs.doNotDisturb ? Theme.c.accent : Theme.c.subSecondary
                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                anchors.verticalCenter: parent.verticalCenter
                                x: root.notifs.doNotDisturb ? parent.width - width - 3 : 3
                                color: root.notifs.doNotDisturb ? Theme.c.onAccent : Theme.c.textMuted
                                Behavior on x { NumberAnimation { duration: 120 } }
                            }
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Do not disturb"
                            color: Theme.c.textMuted
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pixelSize: 24
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.notifs.doNotDisturb = !root.notifs.doNotDisturb
                    }
                }

                Rectangle {
                    visible: root.notifs.history.length > 0
                    anchors.verticalCenter: parent.verticalCenter
                    width: clearText.implicitWidth + 16
                    height: 28
                    radius: 6
                    color: clearHover.hovered ? Theme.c.secondary : "transparent"
                    Text {
                        id: clearText
                        anchors.centerIn: parent
                        text: "Clear all"
                        color: Theme.c.text
                        font.family: Colors.fonts.ui
                        font.weight: Colors.fonts.uiWeight
                        font.pixelSize: 24
                    }
                    HoverHandler { id: clearHover }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.notifs.clearAll()
                    }
                }
            }
        }

        // ── Danh sách (mới nhất trên), cuộn khi dài ─────────────────────────
        Item {
            id: body
            x: 12
            y: header.y + header.height + 8
            width: parent.width - 24
            height: root.notifs.history.length === 0 ? 64
                : Math.min(list.contentHeight, root.bar.screen.height * 0.7)

            Text {
                visible: root.notifs.history.length === 0
                anchors.centerIn: parent
                text: "No notifications"
                color: Theme.c.textMuted
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 26
            }

            ListView {
                id: list
                anchors.fill: parent
                clip: true
                spacing: 6
                boundsBehavior: Flickable.StopAtBounds
                model: ScriptModel {
                    values: root.notifs.history
                }
                delegate: NotificationCard {
                    required property var modelData
                    width: list.width
                    entry: modelData
                    showTime: true
                    // Việc của app mở cửa sổ → đóng danh sách trước.
                    onActivated: {
                        root.bar.popup = ""
                        root.notifs.activate(modelData)
                    }
                    onCloseClicked: root.notifs.dismiss(modelData)
                    onActionClicked: action => {
                        root.bar.popup = ""
                        root.notifs.invokeAction(modelData, action)
                    }
                }
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            }
        }
    }
}
