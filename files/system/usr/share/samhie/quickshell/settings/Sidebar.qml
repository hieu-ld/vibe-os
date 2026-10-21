// Cột trang của app Settings: ô tìm trên cùng, dưới là các trang (icon +
// tên). Trang đang mở = nền `sub-secondary` không viền (như danh sách trang
// hộp cài đặt KDE — style mục 77), trỏ chuột = nền `hover`. Gõ vào ô tìm thì
// chỉ còn trang có tên khớp.
import QtQuick
import "../bar"
import ".."
import "../colors.js" as Colors

Item {
    id: root

    required property var pages     // [{ id, title, icon, category }]
    required property string current
    signal picked(string id)

    readonly property var shown: pages.filter(p => p.title.toLowerCase().includes(search.text.trim().toLowerCase()))

    // Ô tìm.
    Rectangle {
        id: searchBox
        x: 12
        y: 16
        width: parent.width - 24
        height: 40
        radius: 10
        color: Theme.c.secondary

        SamhieIcon {
            x: 12
            anchors.verticalCenter: parent.verticalCenter
            iconName: "system-search"
            size: 16
            color: Theme.c.textMuted
        }
        TextInput {
            id: search
            x: 38
            width: parent.width - x - 12
            height: parent.height
            verticalAlignment: TextInput.AlignVCenter
            clip: true
            color: Theme.c.text
            selectionColor: Theme.c.selection
            selectedTextColor: Theme.c.onSelection
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 26
            // Enter: mở trang đầu tiên còn lại.
            onAccepted: if (root.shown.length > 0) root.picked(root.shown[0].id)

            Text {
                visible: search.text === ""
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                text: "Search"
                color: Theme.c.textMuted
                font: search.font
            }
        }
    }

    Column {
        x: 12
        anchors.top: searchBox.bottom
        anchors.topMargin: 12
        width: parent.width - 24
        spacing: 2

        Repeater {
            model: root.shown
            delegate: Rectangle {
                id: item
                required property var modelData
                readonly property bool active: modelData.id === root.current
                width: parent.width
                height: 40
                radius: 10
                color: active ? Theme.c.subSecondary : mouse.containsMouse ? Theme.c.hover : "transparent"

                SamhieIcon {
                    x: 12
                    anchors.verticalCenter: parent.verticalCenter
                    category: item.modelData.category
                    iconName: item.modelData.icon
                    size: 20
                    color: Theme.c.text
                }
                Text {
                    x: 44
                    width: parent.width - x - 8
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: item.modelData.title
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 26
                }
                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.picked(item.modelData.id)
                }
            }
        }
    }
}
