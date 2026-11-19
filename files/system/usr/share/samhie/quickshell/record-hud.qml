// Thanh báo đang quay màn hình (góc trên-phải, ngay dưới thanh Bar 30px) —
// hiện đồng hồ đếm giây và nút Stop. Người dùng có thể bấm nút Stop trên đây
// hoặc nhấn lại phím tắt (Win+Shift+R) để dừng quay.
import QtQuick
import QtQuick.Controls.impl
import Quickshell
import Quickshell.Io
import Quickshell.Wayland
import "colors.js" as Colors

ShellRoot {
    id: root

    readonly property bool lightMode: Quickshell.env("SAMHIE_REC_LIGHT") === "1"
    readonly property string stopCmd: Quickshell.env("SAMHIE_REC_BIN") ?? "/usr/libexec/samhie/record"
    readonly property var c: lightMode ? Colors.light : Colors.dark

    property int elapsedSeconds: 0
    property bool stopping: false

    function formatTime(sec) {
        const m = Math.floor(sec / 60)
        const s = sec % 60
        const mm = m < 10 ? "0" + m : "" + m
        const ss = s < 10 ? "0" + s : "" + s
        return mm + ":" + ss
    }

    function requestStop() {
        if (stopping) return
        stopping = true
        Quickshell.execDetached([stopCmd, "stop"])
    }

    Timer {
        interval: 1000
        running: !root.stopping
        repeat: true
        onTriggered: root.elapsedSeconds++
    }

    PanelWindow {
        id: hudWin
        visible: true
        color: "transparent"
        anchors { top: true; right: true }
        margins { top: 38; right: 12 }
        implicitWidth: pill.width
        implicitHeight: pill.height
        exclusionMode: ExclusionMode.Ignore
        WlrLayershell.layer: WlrLayer.Top
        WlrLayershell.namespace: "samhie-popup"
        WlrLayershell.keyboardFocus: WlrKeyboardFocus.None

        Rectangle {
            id: pill
            width: row.implicitWidth + 20
            height: 36
            radius: 18
            color: root.c.card
            border.width: 1
            border.color: root.c.subSecondary

            Row {
                id: row
                anchors.centerIn: parent
                spacing: 8

                // Chấm đỏ nhấp nháy báo đang ghi hình
                Rectangle {
                    id: recDot
                    width: 10
                    height: 10
                    radius: 5
                    color: root.c.danger
                    anchors.verticalCenter: parent.verticalCenter

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        running: !root.stopping
                        NumberAnimation { to: 0.25; duration: 600; easing.type: Easing.InOutQuad }
                        NumberAnimation { to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
                    }
                }

                // Đồng hồ đếm thời gian quay (Fira Code)
                Text {
                    text: root.stopping ? "Saving…" : root.formatTime(root.elapsedSeconds)
                    color: root.c.text
                    font.family: Colors.fonts.mono
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Nút Stop
                Rectangle {
                    width: stopRow.implicitWidth + 16
                    height: 24
                    radius: 12
                    color: root.c.danger
                    opacity: stopMouse.pressed ? 0.8 : stopMouse.containsMouse ? 0.9 : 1.0
                    anchors.verticalCenter: parent.verticalCenter

                    Row {
                        id: stopRow
                        anchors.centerIn: parent
                        spacing: 5

                        Rectangle {
                            width: 8
                            height: 8
                            radius: 2
                            color: Colors.shared.white
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "Stop"
                            color: Colors.shared.white
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pixelSize: 20
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        id: stopMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.requestStop()
                    }
                }
            }
        }
    }
}
