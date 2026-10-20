// Lịch sử clipboard (Win+V) — thay `cliphist list | fuzzel`. Phần lệnh (đọc,
// copy, ghim, xoá) nằm ở /usr/libexec/samhie/clipboard; ở đây chỉ vẽ.
//
// User chốt 2026-10-02: hiện CẠNH CON TRỎ CHUỘT (như Windows 11 hiện gần chỗ
// đang gõ), không tràn khỏi màn hình; chọn mục = CHỈ COPY rồi đóng, người
// dùng tự Ctrl+V; lịch sử xoá mỗi lần đăng nhập, mục GHIM thì giữ.
// Giao diện thẻ Claude dựng trước, user chỉnh sau.
//
// Bàn phím: gõ để lọc, ↑/↓ chọn, Enter copy, Delete xoá mục đang chọn, Esc
// xoá chữ lọc rồi đóng. Bấm ra ngoài thẻ là đóng.
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Io
import Quickshell.Wayland
import ".."
import "../bar"
import "../colors.js" as Colors

PanelWindow {
    id: root

    readonly property string helper: "/usr/libexec/samhie/clipboard"

    property bool open: false
    // [{ kind: "P"|"H", id, preview, thumb }] — mục ghim trước.
    property var items: []
    property int current: 0
    // Toạ độ con trỏ trên màn hình này (logic) lúc mở.
    property point cursor: Qt.point(0, 0)

    readonly property var shown: {
        const q = search.text.trim().toLowerCase()
        return q === "" ? items : items.filter(i => i.thumb === "" && i.preview.toLowerCase().includes(q))
    }

    function toggle() {
        if (open)
            close()
        else
            start()
    }
    function start() {
        const m = Hyprland.focusedMonitor
        const s = m ? Quickshell.screens.find(x => x.name === m.name) : null
        if (s)
            screen = s
        search.text = ""
        current = 0
        cursorQuery.running = true
    }
    function close() {
        open = false
    }
    function reload() {
        listQuery.running = true
    }
    function run(args, then) {
        action.then = then ?? null
        action.command = [helper].concat(args)
        action.running = true
    }
    function copy(item) {
        run(["copy", item.kind, item.id])
        close()
    }

    // Vị trí con trỏ: Quickshell không có, hỏi Hyprland (toạ độ toàn màn hình).
    Process {
        id: cursorQuery
        command: ["hyprctl", "cursorpos", "-j"]
        stdout: StdioCollector {
            onStreamFinished: {
                let p = { x: 0, y: 0 }
                try { p = JSON.parse(text) } catch (e) {}
                const m = Hyprland.focusedMonitor
                root.cursor = Qt.point(p.x - (m ? m.x : 0), p.y - (m ? m.y : 0))
                listQuery.running = true
            }
        }
    }

    Process {
        id: listQuery
        command: [root.helper, "list"]
        stdout: StdioCollector {
            onStreamFinished: {
                const pins = []
                const list = []
                for (const line of text.split("\n")) {
                    const f = line.split("\t")
                    if (f.length < 4)
                        continue
                    const item = { kind: f[0], id: f[1], preview: f[2], thumb: f[3] }
                    if (item.kind === "P")
                        pins.push(item)
                    else
                        list.push(item)
                }
                // Mục đã ghim mà còn trong lịch sử (vừa copy lại) thì chỉ hiện bản ghim.
                const pinned = pins.map(p => p.preview)
                root.items = pins.concat(list.filter(i => pinned.indexOf(i.preview) < 0))
                root.current = Math.min(root.current, Math.max(0, root.items.length - 1))
                root.open = true
            }
        }
    }

    Process {
        id: action
        property var then: null
        onExited: {
            if (then)
                then()
        }
    }

    visible: open
    color: "transparent"
    anchors { top: true; bottom: true; left: true; right: true }
    // Phủ cả thanh trên (bấm vào đâu ngoài thẻ cũng đóng).
    exclusionMode: ExclusionMode.Ignore
    WlrLayershell.layer: WlrLayer.Overlay
    WlrLayershell.namespace: "samhie-clipboard"
    WlrLayershell.keyboardFocus: WlrKeyboardFocus.OnDemand

    onVisibleChanged: if (visible) search.forceActiveFocus()

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onPressed: root.close()
    }

    Rectangle {
        id: card

        readonly property int gap: 12   // khe giữa con trỏ và thẻ
        readonly property int edge: 8   // không sát mép màn hình

        width: 380
        height: Math.min(list.y + Math.max(list.contentHeight, empty.height) + 12, 520)
        // Mặc định: góc trên-trái ở dưới-phải con trỏ; tràn thì lật sang
        // phía bên kia con trỏ, vẫn tràn thì kẹp vào trong màn hình.
        x: {
            let v = root.cursor.x + gap
            if (v + width > root.width - edge)
                v = root.cursor.x - gap - width
            return Math.max(edge, Math.min(v, root.width - edge - width))
        }
        y: {
            let v = root.cursor.y + gap
            if (v + height > root.height - edge)
                v = root.cursor.y - gap - height
            return Math.max(edge, Math.min(v, root.height - edge - height))
        }
        radius: 12
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        // Hút cú bấm vào chỗ trống trong thẻ — không thì lọt xuống nền và đóng.
        MouseArea { anchors.fill: parent }

        // ── Ô lọc + Clear all ────────────────────────────────────────────
        Item {
            id: header
            x: 12
            y: 12
            width: parent.width - 24
            height: 36

            Rectangle {
                anchors.left: parent.left
                anchors.right: clearAll.left
                anchors.rightMargin: 8
                height: parent.height
                radius: height / 2
                color: Theme.c.subSecondary

                SamhieIcon {
                    x: 12
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "system-search"
                    size: 16
                    color: Theme.c.textMuted
                }

                TextInput {
                    id: search
                    x: 36
                    width: parent.width - x - 14
                    anchors.verticalCenter: parent.verticalCenter
                    focus: true
                    clip: true
                    color: Theme.c.text
                    selectionColor: Theme.c.selection
                    selectedTextColor: Theme.c.onSelection
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 24
                    cursorDelegate: Caret { input: search; color: Theme.c.text; size: 16 }
                    onTextChanged: root.current = 0

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        visible: search.text === ""
                        text: "Search clipboard"
                        color: Theme.c.textMuted
                        font: search.font
                    }

                    Keys.onPressed: event => {
                        const n = root.shown.length
                        if (event.key === Qt.Key_Escape) {
                            if (search.text !== "")
                                search.text = ""
                            else
                                root.close()
                        } else if (event.key === Qt.Key_Down) {
                            root.current = Math.min(root.current + 1, n - 1)
                        } else if (event.key === Qt.Key_Up) {
                            root.current = Math.max(root.current - 1, 0)
                        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            if (n > 0)
                                root.copy(root.shown[root.current])
                        } else if (event.key === Qt.Key_Delete && search.text === "") {
                            if (n > 0)
                                rowActions.remove(root.shown[root.current])
                        } else {
                            return
                        }
                        event.accepted = true
                    }
                }
            }

            Rectangle {
                id: clearAll
                visible: root.items.some(i => i.kind === "H")
                anchors.right: parent.right
                width: clearText.implicitWidth + 20
                height: parent.height
                radius: 8
                color: clearHover.hovered ? Theme.c.secondary : "transparent"

                Text {
                    id: clearText
                    anchors.centerIn: parent
                    text: "Clear all"
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 24
                }
                HoverHandler { id: clearHover }
                TapHandler { onTapped: root.run(["clear"], root.reload) }
            }
        }

        Text {
            id: empty
            visible: root.shown.length === 0
            x: 12
            y: header.y + header.height + 6
            width: parent.width - 24
            height: 60
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: search.text === "" ? "Nothing copied yet" : "No matches"
            color: Theme.c.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 26
        }

        // ── Danh sách ────────────────────────────────────────────────────
        ListView {
            id: list
            x: 12
            y: header.y + header.height + 6
            width: parent.width - 24
            height: parent.height - y - 12
            clip: true
            spacing: 6
            boundsBehavior: Flickable.StopAtBounds
            model: root.shown
            currentIndex: root.current
            highlightFollowsCurrentItem: false
            onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)

            delegate: Rectangle {
                id: row
                required property var modelData
                required property int index
                readonly property bool pinned: modelData.kind === "P"
                readonly property bool isImage: modelData.thumb !== ""
                readonly property bool lit: rowHover.hovered || index === root.current

                width: list.width
                height: (isImage ? thumb.height : body.height) + 20
                radius: 8
                // Thẻ như Windows 11: nền Secondary trên nền Card (card-alt bản
                // tối cố ý trùng card), đang chọn / rê chuột thì Sub Secondary.
                color: lit ? Theme.c.subSecondary : Theme.c.secondary

                Text {
                    id: body
                    visible: !row.isImage
                    x: 12
                    y: 10
                    width: parent.width - 24 - buttons.width
                    text: row.modelData.preview
                    wrapMode: Text.Wrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                    color: Theme.c.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pixelSize: 24
                    // Dongle: dòng cao gần gấp đôi phần mực — ép lại cho ba dòng gọn.
                    lineHeight: 0.8
                }

                Image {
                    id: thumb
                    visible: row.isImage
                    x: 12
                    y: 10
                    width: Math.min(parent.width - 24 - buttons.width, implicitWidth)
                    height: visible ? Math.min(120, implicitHeight * width / Math.max(1, implicitWidth)) : 0
                    fillMode: Image.PreserveAspectFit
                    horizontalAlignment: Image.AlignLeft
                    source: row.isImage ? "file://" + row.modelData.thumb : ""
                    sourceSize.height: 240
                    asynchronous: true
                    cache: false
                }

                HoverHandler { id: rowHover }
                TapHandler { onTapped: root.copy(row.modelData) }

                // Ghim (luôn hiện khi đã ghim) + xoá (khi rê chuột / đang chọn).
                // Ẩn bằng độ mờ, vẫn giữ chỗ: chữ không chảy lại khi rê chuột.
                Row {
                    id: buttons
                    anchors.right: parent.right
                    anchors.rightMargin: 6
                    y: 6
                    spacing: 2

                    RowButton {
                        iconName: "pin"
                        revealed: row.lit || row.pinned
                        tint: row.pinned ? Theme.c.accent : Theme.c.text
                        onClicked: rowActions.togglePin(row.modelData)
                    }
                    RowButton {
                        iconName: "edit-delete"
                        revealed: row.lit
                        onClicked: rowActions.remove(row.modelData)
                    }
                }
            }
        }
    }

    QtObject {
        id: rowActions
        function togglePin(item) {
            if (item.kind === "P")
                root.run(["unpin", item.id], root.reload)
            else
                root.run(["pin", item.id], root.reload)
        }
        // Xoá mục ghim = bỏ ghim (bản trong lịch sử, nếu còn, hiện lại).
        function remove(item) {
            root.run([item.kind === "P" ? "unpin" : "delete", item.id], root.reload)
        }
    }

    component RowButton: Rectangle {
        id: btn
        property string iconName
        property color tint: Theme.c.text
        property bool revealed: true
        signal clicked()

        opacity: revealed ? 1 : 0
        width: 28
        height: 28
        radius: 6
        // Thẻ đang sáng đã là Sub Secondary → nền nút lấy màu chữ mờ.
        color: btnHover.hovered ? Qt.alpha(Theme.c.text, 0.12) : "transparent"

        SamhieIcon {
            anchors.centerIn: parent
            iconName: btn.iconName
            size: 18
            color: btn.tint
        }
        HoverHandler { id: btnHover }
        // MouseArea, KHÔNG TapHandler: TapHandler của thẻ bên dưới cũng nhận
        // cú bấm (bấm ghim/xoá → thẻ copy rồi đóng bảng — user gặp 2026-10-02).
        // MouseArea nhận rồi thì cú bấm dừng ở đây.
        MouseArea {
            anchors.fill: parent
            enabled: btn.revealed
            onClicked: btn.clicked()
        }
    }
}
