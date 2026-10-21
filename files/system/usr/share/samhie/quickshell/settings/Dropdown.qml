// Ô chọn một-trong-nhiều kiểu nút bật menu của macOS: nút ghi lựa chọn hiện
// tại + mũi tên xuống; bấm mở danh sách ngay dưới nút, ✓ ở lựa chọn đang
// dùng. Popup của QtQuick.Controls vẽ TRONG cửa sổ Settings (lớp phủ của
// cửa sổ) — bấm ra ngoài hoặc Esc là đóng, không cần grab của Hyprland.
//
//   Dropdown {
//       options: [{ label: "1920 × 1080", value: "1920x1080" }, …]
//       current: "1920x1080"
//       onPicked: v => …
//   }
import QtQuick
import QtQuick.Controls.Basic as C
import "../bar"
import ".."
import "../colors.js" as Colors

Rectangle {
    id: root

    property var options: []        // [{ label, value }]
    property var current
    signal picked(var value)

    readonly property var chosen: options.find(o => o.value === current)

    width: Math.max(160, label.implicitWidth + 52)
    height: 32
    radius: 6
    color: Qt.alpha(Theme.c.text, mouse.containsMouse && enabled ? 0.14 : 0.08)
    opacity: enabled ? 1 : 0.5

    Text {
        id: label
        x: 14
        width: parent.width - 44
        height: parent.height
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        text: root.chosen ? root.chosen.label : ""
        color: Theme.c.text
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 24
    }
    SamhieIcon {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        iconName: "arrow-down"
        size: 16
        color: Theme.c.textMuted
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: if (root.enabled && root.options.length > 0) menu.open()
    }

    C.Popup {
        id: menu
        y: root.height + 4
        x: root.width - width
        width: Math.max(root.width, 200)
        height: Math.min(list.contentHeight, 36 * 9) + 12
        padding: 6
        margins: 8   // không tràn khỏi cửa sổ
        closePolicy: C.Popup.CloseOnEscape | C.Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 10
            color: Theme.c.card
            border.width: 1
            border.color: Theme.c.subSecondary
        }
        contentItem: ListView {
            id: list
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: root.options
            currentIndex: root.options.findIndex(o => o.value === root.current)
            // Mở ra là thấy ngay dòng đang chọn (danh sách dài: độ phân giải).
            onVisibleChanged: if (visible) positionViewAtIndex(Math.max(0, currentIndex), ListView.Contain)
            C.ScrollBar.vertical: C.ScrollBar { policy: C.ScrollBar.AsNeeded }

            delegate: Item {
                id: item
                required property var modelData
                required property int index
                width: ListView.view.width
                height: 36

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: Qt.alpha(Theme.c.text, itemMouse.containsMouse ? 0.08 : 0)
                }
                Text {
                    x: 12
                    width: parent.width - 48
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: item.modelData.label
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 24
                }
                SamhieIcon {
                    visible: item.index === list.currentIndex
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "dialog-ok"
                    size: 16
                    color: Theme.c.accentText
                }
                MouseArea {
                    id: itemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        menu.close()
                        if (item.modelData.value !== root.current)
                            root.picked(item.modelData.value)
                    }
                }
            }
        }
    }
}
