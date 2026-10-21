// App Settings của Samhie — dáng macOS System Settings (user chốt
// 2026-10-21): cột trang bên trái (ô tìm + danh sách trang có icon), bên phải
// tiêu đề trang + các NHÓM thẻ bo góc, mỗi nhóm nhiều dòng ngăn vạch mảnh.
// Chạy instance Quickshell RIÊNG (/usr/libexec/samhie/settings →
// `quickshell -n -p` file này), không nằm trong shell chính: đóng cửa sổ là
// thoát hẳn, không tốn RAM lúc không mở.
//
// Sáng/tối: đọc `lightMode` trong ~/.local/state/samhie/settings.json (file
// quick settings ghi — quicksettings/Settings.qml), theo dõi file nên đổi chế
// độ lúc app đang mở là đổi theo.
//
// Trang: settings/<Tên>Page.qml. Trang chưa làm hiện ComingSoon.
import QtQuick
import Quickshell
import Quickshell.Io
import "settings"
import "colors.js" as Colors

ShellRoot {
    id: root

    // Trang mở đầu: biến môi trường SAMHIE_SETTINGS_PAGE (vd "sound" — dòng
    // "Sound Settings" của quick settings), không có thì Appearance.
    property string page: Quickshell.env("SAMHIE_SETTINGS_PAGE") || "appearance"

    readonly property var pages: [
        { id: "display", title: "Display", icon: "video-display", category: "devices" },
        { id: "sound", title: "Sound", icon: "audio-volume-high", category: "status" },
        { id: "bluetooth", title: "Bluetooth", icon: "network-bluetooth", category: "devices" },
        { id: "datetime", title: "Date & Time", icon: "document-open-recent", category: "actions" },
        { id: "appearance", title: "Appearance", icon: "samhie-dark-style", category: "actions" },
        { id: "input", title: "Mouse & Keyboard", icon: "preferences-desktop-keyboard", category: "actions" },
        { id: "updates", title: "Updates", icon: "view-refresh", category: "actions" },
        { id: "about", title: "About", icon: "help-about", category: "actions" },
    ]
    readonly property var current: pages.find(p => p.id === page) ?? pages[0]

    // `settings <trang>` khi app đang mở: đổi trang thay vì mở cửa sổ thứ
    // hai (/usr/libexec/samhie/settings gọi qua `quickshell ipc`).
    IpcHandler {
        target: "settings"
        function open(page: string): void {
            if (page !== "")
                root.page = page
        }
    }

    FileView {
        path: (Quickshell.env("XDG_STATE_HOME") || Quickshell.env("HOME") + "/.local/state") + "/samhie/settings.json"
        blockLoading: true
        watchChanges: true
        onFileChanged: reload()
        onLoaded: Theme.light = adapter.lightMode
        JsonAdapter {
            property bool lightMode: false
        }
    }

    FloatingWindow {
        id: win
        title: "Settings"
        implicitWidth: 900
        implicitHeight: 640
        minimumSize: Qt.size(720, 480)
        color: Theme.c.card
        onClosed: Qt.quit()

        Sidebar {
            id: sidebar
            width: 250
            height: parent.height
            pages: root.pages
            current: root.page
            onPicked: id => root.page = id
        }

        // Vạch ngăn cột trang | nội dung.
        Rectangle {
            x: sidebar.width
            width: 1
            height: parent.height
            color: Theme.c.subSecondary
        }

        Item {
            id: content
            x: sidebar.width + 1
            width: parent.width - x
            height: parent.height

            Text {
                id: heading
                x: 32
                y: 20
                height: 44
                verticalAlignment: Text.AlignVCenter
                text: root.current.title
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 38
            }

            Flickable {
                id: scroller
                anchors.top: heading.bottom
                anchors.topMargin: 12
                anchors.bottom: parent.bottom
                width: parent.width
                contentHeight: loader.height + 32
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                Loader {
                    id: loader
                    x: 32
                    width: scroller.width - 64
                    source: ({
                        about: "settings/AboutPage.qml",
                        display: "settings/DisplayPage.qml",
                        bluetooth: "settings/BluetoothPage.qml",
                        datetime: "settings/DateTimePage.qml",
                        sound: "settings/SoundPage.qml",
                    })[root.current.id]
                        ?? "settings/ComingSoon.qml"
                    onLoaded: if (item.title !== undefined) item.title = root.current.title
                }
            }
        }
    }
}
