// Màn hình đăng nhập Samhie (SDDM, Theme-API 2.0, Qt 6).
//
// Cùng họ với màn hình khoá (/usr/share/samhie/quickshell/lock.qml) — user chốt
// 2026-09-30: giờ to + ngày tiếng Việt ở giữa, dưới là người dùng + ô mật
// khẩu cùng kiểu ô của màn hình khoá, nền là hình nền làm mờ, MỘT nút nguồn ở góc
// mở menu Sleep / Restart / Shut Down.
//
// Chỉ dùng module của Qt (QtQuick, QtQuick.Controls.Basic) — không Plasma,
// không Kirigami, không shader: máy không có tăng tốc đồ hoạ vẫn chạy.
// SDDM đưa vào: sddm, userModel, sessionModel, keyboard, config.
// Màu: colors.js sinh từ theme/palette.toml (bản tối — nền luôn tối).

import QtQuick
import QtQuick.Controls.Basic
import "colors.js" as Colors

Rectangle {
    id: root

    readonly property var c: Colors.dark
    // Font giao diện của Samhie (palette.toml [fonts]): Dongle Light. Dongle
    // "lùn" nên cỡ chữ lớn hơn font thường nhiều (Dongle 20 ≈ chữ 11 của Hack).
    readonly property string uiFont: "Dongle"

    color: c.bar

    // ── Người dùng và phiên ────────────────────────────────────────────────
    // userModel là model Qt: đọc từng dòng qua Instantiator để có mảng JS.
    property int userIndex: userModel.lastIndex >= 0 ? userModel.lastIndex : 0
    property int sessionIndex: sessionModel.lastIndex >= 0 ? sessionModel.lastIndex : 0
    readonly property var currentUser: users.count > 0 ? users.objectAt(userIndex) : null
    // Không có ai trong danh sách (mọi tài khoản bị ẩn) thì gõ tên.
    readonly property bool typeUserName: userModel.count === 0

    Instantiator {
        id: users
        model: userModel
        delegate: QtObject {
            required property string name
            required property string realName
        }
    }

    Instantiator {
        id: sessions
        model: sessionModel
        delegate: QtObject {
            required property string name
        }
    }

    function userLabel(u) {
        return u ? (u.realName !== "" ? u.realName : u.name) : ""
    }

    function login() {
        const name = typeUserName ? userField.text : (currentUser ? currentUser.name : "")
        if (name === "" || busy)
            return
        busy = true
        failed = false
        sddm.login(name, password.text, sessionIndex)
    }

    property bool busy: false
    property bool failed: false

    Connections {
        target: sddm
        function onLoginFailed() {
            root.busy = false
            root.failed = true
            password.text = ""
            password.forceActiveFocus()
            shake.restart()
        }
        function onLoginSucceeded() {
            root.busy = false
        }
    }

    // ── Nền ────────────────────────────────────────────────────────────────
    Image {
        anchors.fill: parent
        source: config.background
        fillMode: Image.PreserveAspectCrop
        // Ảnh đã mờ sẵn ở 960x540; smooth để phóng lên không ra ô vuông.
        smooth: true
        asynchronous: false
    }
    // Tối đi một nửa (như hyprlock cũ, brightness = 0.5) để chữ trắng đọc được.
    Rectangle {
        anchors.fill: parent
        color: root.c.bar
        opacity: 0.5
    }

    // ── Giờ + ngày ────────────────────────────────────────────────────────
    property date now: new Date()
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: root.now = new Date()
    }

    // "Thứ 2, 28/09/2026" / "Chủ nhật, 04/10/2026" — giống
    // màn hình khoá (quickshell/lock.qml). Tự ghép vì máy chạy locale en_US.
    function vietnameseDate(d) {
        const day = d.getDay() // 0 = Chủ nhật
        const name = day === 0 ? "Chủ nhật" : "Thứ " + (day + 1)
        return name + ", " + Qt.formatDate(d, "dd/MM/yyyy")
    }

    Column {
        id: clock
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.verticalCenter
        anchors.bottomMargin: 56
        // Dongle có khoảng trên/dưới chữ rất lớn — kéo ngày sát lên giờ.
        spacing: -56

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatTime(root.now, "HH:mm")
            color: root.c.text
            font.family: root.uiFont
            font.weight: Font.Light
            font.pixelSize: 168
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.vietnameseDate(root.now)
            color: root.c.textMuted
            font.family: root.uiFont
            font.weight: Font.Light
            font.pixelSize: 40
        }
    }

    // ── Người dùng + mật khẩu ─────────────────────────────────────────────
    Column {
        id: form
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.verticalCenter
        spacing: 12

        // Ảnh đại diện: vòng tròn với chữ cái đầu tên. Ảnh ~/.face.icon không
        // dùng: home của Fedora là 700 nên tài khoản sddm không đọc được.
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: !root.typeUserName
            width: 72
            height: 72
            radius: width / 2
            color: root.c.secondary
            border.width: 1
            border.color: root.c.subSecondary

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 3
                text: root.userLabel(root.currentUser).charAt(0).toUpperCase()
                color: root.c.text
                font.family: root.uiFont
                font.weight: Font.Light
                font.pixelSize: 56
            }
        }

        // Tên. Nhiều tài khoản thì bấm vào để chọn người khác.
        AbstractButton {
            id: userButton
            anchors.horizontalCenter: parent.horizontalCenter
            visible: !root.typeUserName
            enabled: userModel.count > 1
            padding: 0
            implicitWidth: userRow.implicitWidth
            implicitHeight: userRow.implicitHeight
            onClicked: userMenu.popup(userButton, 0, userButton.height)

            contentItem: Row {
                id: userRow
                spacing: 4
                Text {
                    text: root.userLabel(root.currentUser)
                    color: root.c.text
                    font.family: root.uiFont
                    font.weight: Font.Light
                    font.pixelSize: 36
                }
                SamhieIcon {
                    visible: userButton.enabled
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "arrow-down"
                    size: 16
                }
            }
            background: null
        }

        SamhieMenu {
            id: userMenu
            Repeater {
                model: users.count
                delegate: SamhieMenuItem {
                    required property int index
                    text: root.userLabel(users.objectAt(index))
                    checked: index === root.userIndex
                    onTriggered: {
                        root.userIndex = index
                        root.failed = false
                        password.forceActiveFocus()
                    }
                }
            }
        }

        Item { width: 1; height: 4 }

        SamhieField {
            id: userField
            visible: root.typeUserName
            placeholderText: "Username"
            onAccepted: password.forceActiveFocus()
        }

        SamhieField {
            id: password
            echoMode: TextInput.Password
            placeholderText: "Password"
            enabled: !root.busy
            focus: true
            // Viền: cyan lúc đang kiểm, đỏ khi sai — như màn hình khoá.
            borderColor: root.busy ? root.c.accent : root.failed ? root.c.danger : root.c.subSecondary
            rightPadding: 48
            onAccepted: root.login()
            onTextEdited: root.failed = false

            AbstractButton {
                id: go
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                width: 36
                height: 36
                visible: password.text !== ""
                onClicked: root.login()
                background: Rectangle {
                    radius: 8
                    color: go.hovered ? root.c.secondary : "transparent"
                }
                contentItem: Item {
                    SamhieIcon {
                        anchors.centerIn: parent
                        iconName: "samhie-log-in"
                        size: 20
                    }
                }
            }

            SequentialAnimation {
                id: shake
                NumberAnimation { target: password; property: "anchors.horizontalCenterOffset"; to: -10; duration: 50 }
                NumberAnimation { target: password; property: "anchors.horizontalCenterOffset"; to: 10; duration: 80 }
                NumberAnimation { target: password; property: "anchors.horizontalCenterOffset"; to: -6; duration: 70 }
                NumberAnimation { target: password; property: "anchors.horizontalCenterOffset"; to: 0; duration: 60 }
            }
        }

        // Một dòng báo dưới ô: sai mật khẩu (đỏ) hoặc Caps Lock (xám).
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 28
            text: root.failed ? "Incorrect password" : keyboard.capsLock ? "Caps Lock is on" : ""
            color: root.failed ? root.c.danger : root.c.textMuted
            font.family: root.uiFont
            font.weight: Font.Light
            font.pixelSize: 28
        }
    }

    // ── Nút nguồn (góc dưới phải) ─────────────────────────────────────────
    AbstractButton {
        id: power
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        width: 44
        height: 44
        // Không có gì để bấm (daemon báo không tắt/khởi động lại được, một
        // phiên duy nhất) thì ẩn nút — như ở chế độ --test-mode.
        visible: sddm.canSuspend || sddm.canReboot || sddm.canPowerOff || sessions.count > 1
        onClicked: powerMenu.open()
        background: Rectangle {
            radius: width / 2
            color: power.hovered || powerMenu.visible ? root.c.secondary : "transparent"
        }
        contentItem: Item {
            SamhieIcon {
                anchors.centerIn: parent
                iconName: "samhie-power"
                size: 22
            }
        }

        SamhieMenu {
            id: powerMenu
            // Mở lên trên, mép phải thẳng mép phải nút.
            x: power.width - width
            y: -height - 8

            // Chọn phiên chỉ hiện khi có hơn một (Samhie ẩn phiên "Hyprland"
            // thường, chỉ còn "Hyprland (uwsm-managed)" — xem sddm.conf.d).
            Repeater {
                model: sessions.count > 1 ? sessions.count : 0
                delegate: SamhieMenuItem {
                    required property int index
                    text: sessions.objectAt(index).name
                    checked: index === root.sessionIndex
                    onTriggered: root.sessionIndex = index
                }
            }
            MenuSeparator {
                visible: sessions.count > 1
                height: visible ? implicitHeight : 0
                contentItem: Rectangle {
                    implicitHeight: 1
                    color: root.c.subSecondary
                }
            }
            SamhieMenuItem {
                text: "Sleep"
                iconName: "system-suspend"
                visible: sddm.canSuspend
                height: visible ? implicitHeight : 0
                onTriggered: sddm.suspend()
            }
            SamhieMenuItem {
                text: "Restart"
                iconName: "system-reboot"
                visible: sddm.canReboot
                height: visible ? implicitHeight : 0
                onTriggered: sddm.reboot()
            }
            SamhieMenuItem {
                text: "Shut Down"
                iconName: "system-shutdown"
                visible: sddm.canPowerOff
                height: visible ? implicitHeight : 0
                onTriggered: sddm.powerOff()
            }
        }
    }

    Component.onCompleted: {
        if (typeUserName)
            userField.forceActiveFocus()
        else
            password.forceActiveFocus()
    }
}
