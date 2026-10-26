// Mặt của S-bot: màn hình đen trên đầu mascot (bản Figma của user,
// /var/mnt/storage/Design/distro/mascot/mascot.svg) — hai mắt tròn cyan, có
// điểm sáng, miệng là một vạch. Dựng bằng QML để đổi biểu cảm theo `mood`:
//   sleep     máy chủ model đang tắt — mắt nhắm (vạch)
//   waking    đang khởi động model — mở mắt, chớp liên tục
//   idle      sẵn sàng — chớp mắt mỗi 3–6 giây
//   thinking  chờ chữ đầu tiên — mắt liếc lên, ba chấm nháy
//   typing    đang trả lời — mắt nhìn xuống, miệng mấp máy
//   error     lỗi — mắt màu danger
// Màu: Colors.shared (black/cyan/white, má hồng mascotBlush) — mặt mascot
// giống nhau ở cả chế độ sáng lẫn tối, như logo.
import QtQuick
import ".."
import "../colors.js" as Colors

Rectangle {
    id: face

    property string mood: "sleep"

    implicitWidth: 72
    implicitHeight: 48
    radius: height * 0.32
    color: Colors.shared.black
    border.width: 1
    border.color: Qt.alpha(Colors.shared.cyan, 0.35)

    readonly property color ink: mood === "error" ? Theme.c.danger : Colors.shared.cyan
    readonly property real eyeSize: height * 0.36

    // Hướng nhìn (−1…1) và độ mở mắt (0 = nhắm thành vạch, 1 = mở hẳn).
    property real lookX: mood === "thinking" ? 0.45 : 0
    property real lookY: mood === "thinking" ? -0.55 : mood === "typing" ? 0.35 : 0
    property real open: mood === "sleep" ? 0.12 : 1
    Behavior on lookX { NumberAnimation { duration: 260; easing.type: Easing.OutCubic } }
    Behavior on lookY { NumberAnimation { duration: 260; easing.type: Easing.OutCubic } }
    Behavior on open { NumberAnimation { duration: 180 } }

    // Chớp mắt: nhắm 90 ms rồi mở. Ngủ thì không chớp.
    property real blink: 1
    SequentialAnimation {
        id: blinkAnim
        NumberAnimation { target: face; property: "blink"; to: 0.1; duration: 70 }
        NumberAnimation { target: face; property: "blink"; to: 1; duration: 110 }
    }
    Timer {
        running: face.visible && face.mood !== "sleep"
        repeat: true
        interval: face.mood === "waking" ? 700 : 3000 + Math.random() * 3000
        onTriggered: {
            interval = face.mood === "waking" ? 700 : 3000 + Math.random() * 3000
            blinkAnim.restart()
        }
    }

    component Eye: Rectangle {
        property real cx
        width: face.eyeSize
        height: face.eyeSize * Math.max(0.1, face.open * face.blink)
        radius: Math.min(width, height) / 2
        x: cx - width / 2 + face.lookX * face.eyeSize * 0.35
        y: face.height * 0.42 - height / 2 + face.lookY * face.eyeSize * 0.3
        color: Qt.darker(face.ink, 2.2)
        border.width: Math.max(1, face.eyeSize * 0.12)
        border.color: face.ink

        // Điểm sáng (như bản vẽ) — ẩn khi mắt gần nhắm.
        Rectangle {
            visible: parent.height > face.eyeSize * 0.5
            width: face.eyeSize * 0.26
            height: width
            radius: width / 2
            x: parent.width * 0.52
            y: parent.height * 0.2
            color: Colors.shared.white
        }
    }

    Eye { cx: face.width * 0.32 }
    Eye { cx: face.width * 0.68 }

    // Má hồng: hai chấm dưới mỗi mắt, lệch ra ngoài (như bản vẽ). Ngủ thì mờ.
    component Blush: Row {
        property real cx
        x: cx - width / 2
        y: face.height * 0.62
        spacing: face.height * 0.04
        opacity: face.mood === "sleep" ? 0.4 : 0.9
        Repeater {
            model: 2
            Rectangle {
                width: face.height * 0.07
                height: width
                radius: width / 2
                color: Colors.shared.mascotBlush
            }
        }
    }
    Blush { cx: face.width * 0.2 }
    Blush { cx: face.width * 0.8 }

    // Miệng: vạch ngắn; lúc gõ thì co giãn như đang nói.
    Rectangle {
        id: mouth
        property real talk: 0
        width: face.width * (0.12 + talk * 0.06)
        height: Math.max(1.5, face.height * 0.035)
        radius: height / 2
        x: (face.width - width) / 2
        y: face.height * 0.74
        color: face.ink
        opacity: face.mood === "sleep" ? 0.5 : 1

        SequentialAnimation on talk {
            running: face.mood === "typing"
            loops: Animation.Infinite
            NumberAnimation { to: 1; duration: 140 }
            NumberAnimation { to: 0.2; duration: 160 }
        }
    }

    // Ba chấm "đang nghĩ" bên phải màn hình.
    Row {
        visible: face.mood === "thinking" || face.mood === "waking"
        anchors.right: parent.right
        anchors.rightMargin: face.width * 0.08
        y: face.height * 0.16
        spacing: face.height * 0.05

        Repeater {
            model: 3
            Rectangle {
                required property int index
                width: face.height * 0.07
                height: width
                radius: width / 2
                color: face.ink
                opacity: 0.25
                SequentialAnimation on opacity {
                    running: face.mood === "thinking" || face.mood === "waking"
                    loops: Animation.Infinite
                    PauseAnimation { duration: index * 180 }
                    NumberAnimation { to: 1; duration: 220 }
                    NumberAnimation { to: 0.25; duration: 320 }
                    PauseAnimation { duration: (2 - index) * 180 }
                }
            }
        }
    }
}
