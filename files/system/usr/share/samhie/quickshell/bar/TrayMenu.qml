// Menu của một app trong khay (bấm phải vào icon), mở ngay dưới icon. Vẽ
// lại bằng QML cho cùng kiểu với menu nguồn — nền Card, dòng 40px, chữ
// Dongle — thay vì menu Qt mặc định của QsMenuAnchor.
//
// Menu con: bấm dòng có mũi tên thì menu thay bằng menu con, dòng đầu "Back"
// để quay lại. Mục đang chọn (ô tick/nút radio) ghi chữ cyan thay vì vẽ ô —
// như menu của theme SDDM.
import QtQuick
import Quickshell
import Quickshell.Widgets
import ".."
import "../colors.js" as Colors

Item {
    id: root

    required property var bar
    readonly property bool open: bar.popup === "tray"


    // Chồng menu con đang mở; rỗng = menu gốc của app.
    property var stack: []
    readonly property var handle: stack.length > 0 ? stack[stack.length - 1]
        : bar.trayItem ? bar.trayItem.menu : null

    onOpenChanged: stack = []

    function back() {
        stack = stack.slice(0, -1)
    }

    QsMenuOpener {
        id: opener
        menu: root.open ? root.handle : null
    }

    visible: open
    width: card.width
    height: card.height

    Rectangle {
        id: card
        width: 280
        height: list.implicitHeight + 12
        radius: 8
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        // Hút cú bấm vào chỗ trống trong thẻ — không thì nó lọt xuống lớp
        // phủ (PopupLayer) và đóng popup.
        MouseArea { anchors.fill: parent }


        Column {
            id: list
            x: 6
            y: 6
            width: parent.width - 12

            MenuRow {
                width: list.width
                visible: root.stack.length > 0
                label: "Back"
                leadIcon: "go-previous"
                onActivated: root.back()
            }

            Repeater {
                model: opener.children

                delegate: Loader {
                    id: entry
                    required property QsMenuEntry modelData
                    width: list.width
                    sourceComponent: modelData.isSeparator ? separator : row

                    Component {
                        id: separator
                        Item {
                            height: 9
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width
                                height: 1
                                color: Theme.c.subSecondary
                            }
                        }
                    }

                    Component {
                        id: row
                        MenuRow {
                            label: entry.modelData.text
                            appIcon: entry.modelData.icon
                            checked: entry.modelData.checkState === Qt.Checked
                            hasChildren: entry.modelData.hasChildren
                            enabled: entry.modelData.enabled
                            onActivated: {
                                if (entry.modelData.hasChildren) {
                                    root.stack = root.stack.concat([entry.modelData])
                                } else {
                                    entry.modelData.triggered()
                                    root.bar.popup = ""
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Một dòng: [icon] chữ [mũi tên menu con]. Cao 40, chữ Dongle 24px, nền
    // Secondary khi rê chuột — như dòng của menu nguồn.
    component MenuRow: Rectangle {
        id: menuRow
        property string label
        property string leadIcon     // icon Samhie (vd mũi tên Back)
        property string appIcon      // icon do app gửi
        property bool checked
        property bool hasChildren
        signal activated()

        height: 40
        radius: 6
        color: enabled && rowHover.hovered ? Theme.c.secondary : "transparent"
        opacity: enabled ? 1 : 0.4

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: menuRow.hasChildren ? 36 : 12
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            SamhieIcon {
                visible: menuRow.leadIcon !== ""
                anchors.verticalCenter: parent.verticalCenter
                iconName: menuRow.leadIcon
                color: Theme.c.text
            }
            IconImage {
                visible: menuRow.appIcon !== ""
                anchors.verticalCenter: parent.verticalCenter
                implicitSize: 18
                source: menuRow.appIcon
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - x
                elide: Text.ElideRight
                text: menuRow.label
                color: menuRow.checked ? Theme.c.accentText : Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 24  // 28 thấy to (user 2026-10-02)
            }
        }

        SamhieIcon {
            visible: menuRow.hasChildren
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            iconName: "go-next"
            color: Theme.c.textMuted
        }

        HoverHandler { id: rowHover }
        TapHandler {
            enabled: menuRow.enabled
            onTapped: menuRow.activated()
        }
    }
}
