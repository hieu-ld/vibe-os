// Giao diện chọn vùng quay màn hình của Samhie (Win+Shift+R) — gọi bởi
// /usr/libexec/samhie/record bằng `quickshell -p`.
//
// Ba chế độ chọn ngay trên thanh công cụ dưới đáy màn hình:
//   - Region: kéo chọn vùng như screenshot-region.qml (có 8 tay nắm chỉnh cỡ,
//     kéo bên trong để dời).
//   - Screen: quay trọn một màn hình (bấm vào màn hình nào chọn màn hình đó).
//   - Window: rê chuột/bấm chọn một cửa sổ Hyprland đang mở.
// Bấm nút Start (hoặc Enter) để bắt đầu quay; bấm ✕ (hoặc Esc) để huỷ.
import QtQuick
import QtQuick.Controls.impl
import Quickshell
import Quickshell.Wayland
import "colors.js" as Colors

ShellRoot {
    id: root

    readonly property bool lightMode: Quickshell.env("SAMHIE_REC_LIGHT") === "1"
    readonly property var c: lightMode ? Colors.light : Colors.dark
    readonly property var screens: Array.from(Quickshell.screens)

    // Danh sách cửa sổ Hyprland do script truyền vào qua biến môi trường JSON:
    // [{ address, className, title, monitor, x, y, width, height }]
    readonly property var windowsList: {
        const raw = Quickshell.env("SAMHIE_REC_WINDOWS") ?? "[]"
        try {
            return JSON.parse(raw)
        } catch (e) {
            return []
        }
    }

    // Chế độ: "region" | "screen" | "window"
    property string captureMode: {
        const m = Quickshell.env("SAMHIE_REC_MODE") ?? "region"
        return (m === "screen" || m === "window") ? m : "region"
    }
    property bool recordAudio: true
    property bool recordMic: false
    property bool showCursor: true

    // Trạng thái chọn của từng chế độ
    property var selScreen: screens.length > 0 ? screens[0] : null
    property rect sel: Qt.rect(0, 0, 0, 0)
    readonly property bool hasRegion: selScreen !== null && sel.width >= 32 && sel.height >= 32

    property var selectedWin: windowsList.length > 0 ? windowsList[0] : null
    property var hoveredWin: null

    readonly property bool canStart: {
        if (captureMode === "region") return hasRegion
        if (captureMode === "screen") return selScreen !== null
        if (captureMode === "window") return selectedWin !== null
        return false
    }

    readonly property int frameWidth: 2
    readonly property int handleThick: 5
    readonly property int handleLength: 22
    readonly property int grab: 14

    function startRecording() {
        if (!canStart) return
        let payload = {
            mode: captureMode,
            audio: recordAudio,
            mic: recordMic,
            cursor: showCursor
        }
        if (captureMode === "region") {
            payload.monitor = selScreen.name
            payload.x = Math.round(sel.x)
            payload.y = Math.round(sel.y)
            payload.width = Math.round(sel.width)
            payload.height = Math.round(sel.height)
        } else if (captureMode === "screen") {
            payload.monitor = selScreen.name
            payload.x = 0
            payload.y = 0
            payload.width = Math.round(selScreen.width)
            payload.height = Math.round(selScreen.height)
        } else if (captureMode === "window") {
            payload.monitor = selectedWin.monitor
            payload.windowAddress = selectedWin.address
            payload.windowClass = selectedWin.className
            payload.windowTitle = selectedWin.title
            payload.width = Math.round(selectedWin.width)
            payload.height = Math.round(selectedWin.height)
        }
        console.info("SAMHIE_REC_RESULT:" + JSON.stringify(payload))
        Qt.quit()
    }

    function winAt(screenName, mx, my) {
        for (let i = 0; i < windowsList.length; i++) {
            const w = windowsList[i]
            if (w.monitor === screenName &&
                mx >= w.x && mx <= w.x + w.width &&
                my >= w.y && my <= w.y + w.height) {
                return w
            }
        }
        return null
    }

    Variants {
        model: root.screens

        PanelWindow {
            id: win
            required property ShellScreen modelData
            readonly property bool ownsRegion: root.selScreen === modelData

            screen: modelData
            visible: true
            color: "transparent"
            anchors { top: true; bottom: true; left: true; right: true }
            exclusionMode: ExclusionMode.Ignore
            WlrLayershell.layer: WlrLayer.Overlay
            WlrLayershell.namespace: "samhie-screenshot"
            WlrLayershell.keyboardFocus: WlrKeyboardFocus.Exclusive

            // Tính khung sáng hiện tại trên màn hình này theo chế độ
            readonly property rect activeRect: {
                if (root.captureMode === "region") {
                    return ownsRegion && root.hasRegion ? root.sel : Qt.rect(0, 0, 0, 0)
                }
                if (root.captureMode === "screen") {
                    return ownsRegion ? Qt.rect(0, 0, win.width, win.height) : Qt.rect(0, 0, 0, 0)
                }
                if (root.captureMode === "window") {
                    const target = root.hoveredWin ?? root.selectedWin
                    if (target && target.monitor === modelData.name) {
                        const x1 = Math.max(0, target.x)
                        const y1 = Math.max(0, target.y)
                        const x2 = Math.min(win.width, target.x + target.width)
                        const y2 = Math.min(win.height, target.y + target.height)
                        return Qt.rect(x1, y1, Math.max(0, x2 - x1), Math.max(0, y2 - y1))
                    }
                    return Qt.rect(0, 0, 0, 0)
                }
                return Qt.rect(0, 0, 0, 0)
            }
            readonly property bool hasHole: activeRect.width >= 1 && activeRect.height >= 1

            // Lớp tối bốn dải quanh vùng đang chọn
            Item {
                id: shade
                anchors.fill: parent
                readonly property rect r: win.activeRect
                readonly property bool hole: win.hasHole

                component Band: Rectangle {
                    color: Colors.shared.captureShade
                    opacity: 0.5
                }
                Band { x: 0; y: 0; width: parent.width; height: shade.hole ? shade.r.y : parent.height }
                Band {
                    visible: shade.hole
                    x: 0; y: shade.r.y + shade.r.height
                    width: parent.width; height: parent.height - y
                }
                Band {
                    visible: shade.hole
                    x: 0; y: shade.r.y
                    width: shade.r.x; height: shade.r.height
                }
                Band {
                    visible: shade.hole
                    x: shade.r.x + shade.r.width; y: shade.r.y
                    width: parent.width - x; height: shade.r.height
                }
            }

            // Khung bao vùng chọn + 8 tay nắm khi ở chế độ Region
            Item {
                id: frame
                visible: win.hasHole
                x: win.activeRect.x
                y: win.activeRect.y
                width: win.activeRect.width
                height: win.activeRect.height

                Rectangle {
                    x: -root.frameWidth
                    y: -root.frameWidth
                    width: parent.width + 2 * root.frameWidth
                    height: parent.height + 2 * root.frameWidth
                    color: "transparent"
                    border.width: root.frameWidth
                    border.color: root.captureMode === "window" ? Colors.shared.cyan : Colors.shared.captureFrame
                }

                // Nhãn kích thước / tên cửa sổ nằm ngay góc trên-trái của khung
                Rectangle {
                    visible: frame.width >= 40 && frame.height >= 30
                    x: 0
                    y: frame.y >= 34 ? -30 : 8
                    width: badgeRow.implicitWidth + 16
                    height: 24
                    radius: 6
                    color: root.c.card
                    border.width: 1
                    border.color: root.c.subSecondary

                    Row {
                        id: badgeRow
                        anchors.centerIn: parent
                        spacing: 6

                        Text {
                            text: Math.round(frame.width) + " × " + Math.round(frame.height)
                            color: root.c.text
                            font.family: Colors.fonts.mono
                            font.pixelSize: 11
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            visible: root.captureMode === "window" && (root.hoveredWin ?? root.selectedWin) !== null
                            text: {
                                const w = root.hoveredWin ?? root.selectedWin
                                return w ? "· " + (w.title || w.className) : ""
                            }
                            color: root.c.accentText
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pixelSize: 20
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                // Tay nắm góc & cạnh (chỉ hiện ở chế độ Region)
                Repeater {
                    model: root.captureMode === "region" ? [[false, false], [true, false], [false, true], [true, true]] : []

                    Item {
                        id: corner
                        required property var modelData
                        readonly property bool onRight: modelData[0]
                        readonly property bool onBottom: modelData[1]
                        readonly property int t: root.handleThick
                        readonly property int len: root.handleLength
                        x: onRight ? frame.width : -t
                        y: onBottom ? frame.height : -t
                        Rectangle {
                            color: Colors.shared.captureFrame
                            x: corner.onRight ? corner.t - corner.len : 0
                            width: corner.len; height: corner.t
                        }
                        Rectangle {
                            color: Colors.shared.captureFrame
                            y: corner.onBottom ? corner.t - corner.len : 0
                            width: corner.t; height: corner.len
                        }
                    }
                }

                component EdgeBar: Rectangle {
                    visible: root.captureMode === "region"
                    color: Colors.shared.captureFrame
                }
                readonly property int barLen: Math.min(root.handleLength, Math.max(0, width - 2 * root.handleLength))
                readonly property int barLenV: Math.min(root.handleLength, Math.max(0, height - 2 * root.handleLength))
                EdgeBar { x: (frame.width - width) / 2; y: -root.handleThick; width: frame.barLen; height: root.handleThick }
                EdgeBar { x: (frame.width - width) / 2; y: frame.height; width: frame.barLen; height: root.handleThick }
                EdgeBar { x: -root.handleThick; y: (frame.height - height) / 2; width: root.handleThick; height: frame.barLenV }
                EdgeBar { x: frame.width; y: (frame.height - height) / 2; width: root.handleThick; height: frame.barLenV }
            }

            // Vùng bắt chuột chọn khung
            MouseArea {
                id: mouse
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                cursorShape: {
                    if (root.captureMode === "region")
                        return cursorFor(pressed ? dragMode : hitTest(mouseX, mouseY))
                    return Qt.PointingHandCursor
                }

                property string dragMode: ""
                property real startX: 0
                property real startY: 0
                property rect startSel: Qt.rect(0, 0, 0, 0)

                function hitTest(x, y) {
                    if (!win.ownsRegion || !root.hasRegion) return ""
                    const s = root.sel, g = root.grab
                    const l = s.x, r = s.x + s.width, t = s.y, b = s.y + s.height
                    if (x < l - g || x > r + g || y < t - g || y > b + g) return ""
                    const dl = Math.abs(x - l), dr = Math.abs(x - r)
                    const dt = Math.abs(y - t), db = Math.abs(y - b)
                    let h = "", v = ""
                    if (Math.min(dl, dr) <= g) h = dl < dr ? "w" : "e"
                    if (Math.min(dt, db) <= g) v = dt < db ? "n" : "s"
                    if (h || v) return v + h
                    return "move"
                }

                function cursorFor(m) {
                    switch (m) {
                    case "move": return Qt.SizeAllCursor
                    case "n": case "s": return Qt.SizeVerCursor
                    case "e": case "w": return Qt.SizeHorCursor
                    case "nw": case "se": return Qt.SizeFDiagCursor
                    case "ne": case "sw": return Qt.SizeBDiagCursor
                    default: return Qt.CrossCursor
                    }
                }

                function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)) }

                onPressed: event => {
                    root.selScreen = win.modelData
                    if (root.captureMode === "screen") {
                        return
                    }
                    if (root.captureMode === "window") {
                        const hit = root.winAt(win.modelData.name, event.x, event.y)
                        if (hit) root.selectedWin = hit
                        return
                    }
                    dragMode = hitTest(event.x, event.y)
                    startX = clamp(Math.round(event.x), 0, win.width)
                    startY = clamp(Math.round(event.y), 0, win.height)
                    if (dragMode === "") {
                        dragMode = "new"
                        root.sel = Qt.rect(startX, startY, 0, 0)
                    }
                    startSel = root.sel
                }

                onPositionChanged: event => {
                    if (root.captureMode === "window") {
                        root.hoveredWin = root.winAt(win.modelData.name, event.x, event.y)
                        return
                    }
                    if (root.captureMode !== "region" || !pressed) return
                    const x = clamp(Math.round(event.x), 0, win.width)
                    const y = clamp(Math.round(event.y), 0, win.height)
                    const s = startSel

                    if (dragMode === "new") {
                        root.sel = Qt.rect(Math.min(startX, x), Math.min(startY, y),
                                           Math.abs(x - startX), Math.abs(y - startY))
                    } else if (dragMode === "move") {
                        root.sel = Qt.rect(clamp(s.x + x - startX, 0, win.width - s.width),
                                           clamp(s.y + y - startY, 0, win.height - s.height),
                                           s.width, s.height)
                    } else {
                        let l = s.x, r = s.x + s.width, t = s.y, b = s.y + s.height
                        const dx = x - startX, dy = y - startY
                        if (dragMode.includes("w")) l = clamp(l + dx, 0, win.width)
                        if (dragMode.includes("e")) r = clamp(r + dx, 0, win.width)
                        if (dragMode.includes("n")) t = clamp(t + dy, 0, win.height)
                        if (dragMode.includes("s")) b = clamp(b + dy, 0, win.height)
                        root.sel = Qt.rect(Math.min(l, r), Math.min(t, b),
                                           Math.abs(r - l), Math.abs(b - t))
                    }
                }

                onReleased: {
                    if (root.captureMode === "region") {
                        dragMode = ""
                    }
                }
            }

            // Gợi ý hướng dẫn nhỏ phía trên thanh công cụ khi chưa kéo vùng
            Rectangle {
                visible: root.captureMode === "region" && !root.hasRegion
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: toolbar.top
                anchors.bottomMargin: 12
                width: hintText.implicitWidth + 24
                height: 32
                radius: 8
                color: root.c.card
                border.width: 1
                border.color: root.c.subSecondary

                Text {
                    id: hintText
                    anchors.centerIn: parent
                    text: "Drag to select an area to record"
                    color: root.c.textMuted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 22
                }
            }

            // ── Thanh điều khiển nổi dưới đáy màn hình ─────────────────────
            Rectangle {
                id: toolbar
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 36
                width: toolbarRow.implicitWidth + 20
                height: 54
                radius: 12
                color: root.c.card
                border.width: 1
                border.color: root.c.subSecondary

                // Chặn cú bấm lọt xuống vùng chọn bên dưới
                MouseArea { anchors.fill: parent }

                component ToolBtn: Rectangle {
                    id: btn
                    property string label: ""
                    property string iconName: ""
                    property string iconCat: "actions"
                    property bool active: false
                    signal clicked()

                    width: btnContent.implicitWidth + 22
                    height: 38
                    radius: 8
                    anchors.verticalCenter: parent.verticalCenter
                    color: active ? root.c.selection
                        : Qt.alpha(root.c.text, btnMouse.containsMouse ? 0.14 : 0.07)

                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        visible: btn.active
                        color: root.c.onSelection
                        opacity: btnMouse.pressed ? 0.16 : btnMouse.containsMouse ? 0.09 : 0
                    }

                    Row {
                        id: btnContent
                        anchors.centerIn: parent
                        spacing: 7

                        IconImage {
                            visible: btn.iconName !== ""
                            source: btn.iconName !== ""
                                ? "file:///usr/share/icons/Samhie/" + btn.iconCat + "/scalable/" + btn.iconName + ".svg"
                                : ""
                            sourceSize: Qt.size(18, 18)
                            color: btn.active ? root.c.onSelection : root.c.text
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            visible: btn.label !== ""
                            text: btn.label
                            color: btn.active ? root.c.onSelection : root.c.text
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pixelSize: 24
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        id: btnMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: btn.clicked()
                    }
                }

                component Divider: Rectangle {
                    width: 1
                    height: 26
                    anchors.verticalCenter: parent.verticalCenter
                    color: root.c.subSecondary
                }

                Row {
                    id: toolbarRow
                    anchors.centerIn: parent
                    spacing: 6

                    // 1. Ba chế độ: Region | Screen | Window
                    ToolBtn {
                        label: "Region"
                        iconName: "edit-select"
                        active: root.captureMode === "region"
                        onClicked: root.captureMode = "region"
                    }
                    ToolBtn {
                        label: "Screen"
                        iconName: "view-fullscreen"
                        active: root.captureMode === "screen"
                        onClicked: root.captureMode = "screen"
                    }
                    ToolBtn {
                        label: "Window"
                        iconName: "window-new"
                        active: root.captureMode === "window"
                        onClicked: root.captureMode = "window"
                    }

                    Divider {}

                    // 2. Tuỳ chọn thu âm & con trỏ
                    ToolBtn {
                        label: "Sound"
                        iconName: "audio-speakers"
                        iconCat: "devices"
                        active: root.recordAudio
                        onClicked: root.recordAudio = !root.recordAudio
                    }
                    ToolBtn {
                        label: "Mic"
                        iconName: "audio-input-microphone"
                        iconCat: "devices"
                        active: root.recordMic
                        onClicked: root.recordMic = !root.recordMic
                    }
                    ToolBtn {
                        label: "Cursor"
                        iconName: "input-mouse"
                        iconCat: "devices"
                        active: root.showCursor
                        onClicked: root.showCursor = !root.showCursor
                    }

                    Divider {}

                    // 3. Nút Start
                    Rectangle {
                        id: startBtn
                        width: startRow.implicitWidth + 26
                        height: 38
                        radius: 8
                        anchors.verticalCenter: parent.verticalCenter
                        color: root.c.danger
                        opacity: root.canStart ? (startMouse.pressed ? 0.85 : startMouse.containsMouse ? 0.93 : 1.0) : 0.4

                        Row {
                            id: startRow
                            anchors.centerIn: parent
                            spacing: 8

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: Colors.shared.white
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: "Start"
                                color: Colors.shared.white
                                font.family: Colors.fonts.ui
                                font.weight: Colors.fonts.uiWeight
                                font.pixelSize: 24
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            id: startMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: root.canStart ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: root.startRecording()
                        }
                    }

                    // 4. Nút đóng (Esc)
                    Rectangle {
                        width: 38
                        height: 38
                        radius: 8
                        anchors.verticalCenter: parent.verticalCenter
                        color: Qt.alpha(root.c.text, closeMouse.containsMouse ? 0.14 : 0.07)

                        IconImage {
                            anchors.centerIn: parent
                            source: "file:///usr/share/icons/Samhie/actions/scalable/dialog-close.svg"
                            sourceSize: Qt.size(16, 16)
                            color: root.c.text
                        }

                        MouseArea {
                            id: closeMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.quit()
                        }
                    }
                }
            }

            // Phím tắt bàn phím: Enter = Start, Esc = Huỷ, 1/2/3 = Đổi chế độ
            Item {
                anchors.fill: parent
                focus: true
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Escape) {
                        Qt.quit()
                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        root.startRecording()
                    }
                    event.accepted = true
                }
            }
        }
    }
}
