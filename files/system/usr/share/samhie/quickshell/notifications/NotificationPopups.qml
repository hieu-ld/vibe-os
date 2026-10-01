// Popup thông báo ở góc trên-phải, ngay dưới thanh (user chốt 2026-10-01),
// trên màn hình đang focus. Mới nhất ở TRÊN, hiện tối đa 4. Rê chuột lên thẻ
// thì thẻ đứng yên (không tự tắt) — như Windows.
//
// Lớp chỉ to đúng bằng các thẻ (không phủ màn hình) nên bấm ra ngoài vẫn vào
// app bên dưới; không nhận bàn phím. Tầng Overlay: nổi trên cả màn tổng quan.
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Wayland

PanelWindow {
    id: root

    required property var notifs   // Notifications.qml

    readonly property int maxShown: 4
    readonly property int gap: 6   // như khe giữa thanh và popup của thanh

    screen: {
        const m = Hyprland.focusedMonitor
        return Quickshell.screens.find(s => m && s.name === m.name) ?? Quickshell.screens[0]
    }
    // Danh sách của chuông đang mở thì ẩn — popup nằm tầng trên, đè lên nó.
    visible: notifs.popups.length > 0 && !notifs.centerOpen
    color: "transparent"

    anchors { top: true; right: true }
    margins { top: gap; right: gap }
    exclusionMode: ExclusionMode.Normal
    exclusiveZone: 0
    implicitWidth: 360
    implicitHeight: Math.max(1, column.height)
    WlrLayershell.namespace: "samhie-notifications"
    WlrLayershell.layer: WlrLayer.Overlay
    WlrLayershell.keyboardFocus: WlrKeyboardFocus.None

    Column {
        id: column
        width: parent.width
        spacing: root.gap

        // Thẻ mới trượt vào từ mép phải.
        add: Transition {
            NumberAnimation { property: "x"; from: 380; duration: 200; easing.type: Easing.OutCubic }
        }
        move: Transition {
            NumberAnimation { property: "y"; duration: 150; easing.type: Easing.OutCubic }
        }

        Repeater {
            model: ScriptModel {
                values: root.notifs.popups.slice(0, root.maxShown)
            }

            delegate: NotificationCard {
                id: card
                required property var modelData
                entry: modelData

                onActivated: root.notifs.activate(modelData)
                onCloseClicked: root.notifs.dismiss(modelData)
                onActionClicked: action => root.notifs.invokeAction(modelData, action)

                Timer {
                    running: card.modelData.timeout > 0 && !card.hovered
                    interval: card.modelData.timeout
                    onTriggered: root.notifs.timeoutPopup(card.modelData)
                }
            }
        }
    }
}
