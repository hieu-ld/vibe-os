// Ảnh xem trước sau khi chụp CẢ màn hình (Print) — /usr/libexec/samhie/
// screenshot gọi file này bằng `quickshell -p` sau khi grim đã lưu ảnh, truyền
// đường dẫn ảnh qua biến SAMHIE_SHOT_FILE.
//
// Ảnh vừa chụp thu nhỏ vừa màn hình, đóng khung (user tả 2026-09-29): ngoài
// cùng chừa 5px trống, rồi 5px viền trắng, ảnh ở trong. Đứng yên ~1 s rồi mờ
// dần và thoát. Không nhận chuột: bấm xuyên qua xuống cửa sổ bên dưới.
//
// grim ghép mọi màn hình thành MỘT ảnh theo bố cục của chúng, nên mỗi màn hình
// hiện đúng phần của nó: cả ảnh được co theo cùng tỉ lệ rồi dời tới vị trí
// màn hình đó, phần thừa bị cắt (clip).
import QtQuick
import Quickshell
import Quickshell.Wayland
import "colors.js" as Colors

ShellRoot {
    id: root

    readonly property string file: Quickshell.env("SAMHIE_SHOT_FILE") ?? ""
    readonly property int gap: 5
    readonly property int border: 5
    readonly property int holdMs: 1000
    readonly property int fadeMs: 300

    // Khung bao mọi màn hình (toạ độ logic) — cũng là khung của ảnh grim.
    readonly property var screens: Array.from(Quickshell.screens)
    readonly property int minX: Math.min(...screens.map(s => s.x))
    readonly property int minY: Math.min(...screens.map(s => s.y))
    readonly property int layoutWidth: Math.max(...screens.map(s => s.x + s.width)) - minX
    readonly property int layoutHeight: Math.max(...screens.map(s => s.y + s.height)) - minY

    // Mọi màn hình cùng hiện, cùng mờ, cùng thoát.
    property int ready: 0
    property real fade: 1

    Component.onCompleted: if (file === "") Qt.quit()

    SequentialAnimation {
        id: life
        PauseAnimation { duration: root.holdMs }
        NumberAnimation { target: root; property: "fade"; to: 0; duration: root.fadeMs; easing.type: Easing.InQuad }
        ScriptAction { script: Qt.quit() }
    }

    // Lỡ ảnh không nạp được thì cũng đừng treo script chụp.
    Timer { interval: 5000; running: true; onTriggered: Qt.quit() }

    Variants {
        model: root.screens

        PanelWindow {
            id: win
            required property ShellScreen modelData
            // Hiện khi ảnh nạp xong rồi GIỮ hiện — xem chú thích cùng chỗ trong
            // screenshot-region.qml (scale lẻ: nạp lại ảnh khi dpr đổi, lặp mãi).
            property bool loaded: false

            screen: modelData
            visible: loaded
            color: "transparent"
            anchors { top: true; bottom: true; left: true; right: true }
            exclusionMode: ExclusionMode.Ignore
            WlrLayershell.layer: WlrLayer.Overlay
            WlrLayershell.namespace: "samhie-screenshot"
            WlrLayershell.keyboardFocus: WlrKeyboardFocus.None
            mask: Region {}

            onLoadedChanged: if (++root.ready === root.screens.length) life.start()

            // Tỉ lệ co: phần của màn hình này phải lọt vào (màn − 2·(gap+border)),
            // giữ nguyên tỉ lệ khung hình. Lấy cỡ từ màn hình chứ không từ cửa
            // sổ: cửa sổ chưa hiện thì chưa có cỡ.
            readonly property int inset: root.gap + root.border
            readonly property int sw: modelData.width
            readonly property int sh: modelData.height
            readonly property real k: Math.min((sw - 2 * inset) / sw, (sh - 2 * inset) / sh)
            readonly property real picW: Math.round(sw * k)
            readonly property real picH: Math.round(sh * k)

            Rectangle {
                id: frame
                opacity: root.fade
                color: Colors.shared.captureFrame
                width: win.picW + 2 * root.border
                height: win.picH + 2 * root.border
                anchors.centerIn: parent

                Item {
                    x: root.border; y: root.border
                    width: win.picW; height: win.picH
                    clip: true

                    Image {
                        id: shot
                        source: root.file === "" ? ""
                            : "file://" + root.file.split("/").map(encodeURIComponent).join("/")
                        x: -(win.modelData.x - root.minX) * win.k
                        y: -(win.modelData.y - root.minY) * win.k
                        width: root.layoutWidth * win.k
                        height: root.layoutHeight * win.k
                        fillMode: Image.Stretch
                        smooth: true
                        mipmap: true
                        asynchronous: true
                        cache: false
                        onStatusChanged: if (status === Image.Ready) win.loaded = true
                    }
                }
            }
        }
    }
}
