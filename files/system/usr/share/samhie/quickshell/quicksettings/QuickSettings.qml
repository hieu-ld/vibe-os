// Quick settings — kiểu Quick Settings của Windows 11 như bản 1.0 (user chốt
// 2026-09-08/10, bản 2 chốt lại 2026-10-02): mở bằng nút mũi tên trên thanh
// (bar/QuickButton.qml), mép phải thẳng lề phải cụm nút. Trên cùng là hàng
// âm lượng, dưới là lưới 2 cột các ô bật/tắt:
//   Wired · Wi-Fi · Bluetooth · Night light · Do not disturb · Dark mode · S-bot
// S-bot (2026-10-26) không phải công tắc: bấm là đóng bảng, mở khung chat
// của trợ lý (assistant/Assistant.qml); ô sáng khi khung chat đang mở.
// Ô nào máy không có phần cứng thì ẩn (lẻ hàng không sao — user chốt ở 1.0).
// Ô Wi-Fi tách đôi kiểu Windows 11: phần trái bật/tắt sóng, mũi tên bên phải
// mở trang chọn mạng (WifiPage.qml) thay chỗ cả bảng; nút › cạnh thanh âm
// lượng mở trang chọn ngõ ra (SoundPage.qml) cũng vậy.
//
// Trạng thái mạng hỏi `nmcli` CHỈ KHI BẢNG ĐANG MỞ (mỗi 2 s) — bảng đóng gần
// như suốt ngày, khỏi tốn (bài học bản 1.0). Bấm ô là đổi hình NGAY, 600 ms
// sau hỏi lại để chốt; lệnh hỏng thì lần hỏi đó kéo ô về trạng thái thật.
// Bluetooth, âm lượng đọc thẳng qua module của Quickshell nên luôn đúng.
import QtQuick
import Quickshell
import Quickshell.Bluetooth
import Quickshell.Io
import ".."
import "../bar"
import "../colors.js" as Colors

PopupMotion {
    id: root

    required property var bar
    readonly property var shell: bar.shell
    readonly property bool open: bar.popup === "quick"
    // Trang đang xem: "" = bảng chính, "wifi" = chọn mạng, "sound" = chọn
    // ngõ ra âm thanh. Đóng bảng thì về
    // bảng chính.
    property string page: ""
    // Về trang chính khi đã ẩn hẳn — đổi giữa lúc mờ dần thì thấy giật.
    onVisibleChanged: if (!visible) page = ""

    // Hiện/ẩn có hiệu ứng (PopupMotion).
    showing: open
    transformOrigin: Item.TopRight
    width: card.width
    height: card.height

    // ── Mạng (nmcli) ───────────────────────────────────────────────────────
    // Thiết bị: [{ type, state, device, connection }], wifiRadio: bật/tắt sóng.
    property var netDevices: []
    property bool wifiRadio: false

    readonly property var ethernet: netDevices.filter(d => d.type === "ethernet")
    readonly property var wifi: netDevices.filter(d => d.type === "wifi")

    Process {
        id: netQuery
        // Một lần chạy cho cả hai câu hỏi. CONNECTION để CUỐI: tên kết nối có
        // thể chứa ':' (nmcli thoát thành '\:'), ba cột trước thì không.
        command: ["sh", "-c", "nmcli radio wifi; nmcli -t -f TYPE,STATE,DEVICE,CONNECTION device"]
        stdout: StdioCollector {
            onStreamFinished: {
                const lines = text.trim().split("\n")
                root.wifiRadio = lines[0] === "enabled"
                root.netDevices = lines.slice(1).map(l => {
                    const p = l.split(":")
                    return {
                        type: p[0], state: p[1], device: p[2],
                        connection: p.slice(3).join(":").replace(/\\:/g, ":")
                    }
                })
            }
        }
    }
    function pollNet() {
        if (!netQuery.running)
            netQuery.running = true
    }
    Timer {
        running: root.open
        interval: 2000
        repeat: true
        triggeredOnStart: true
        onTriggered: root.pollNet()
    }
    Timer {
        id: recheck
        interval: 600
        onTriggered: root.pollNet()
    }
    function run(cmd) {
        Quickshell.execDetached(cmd)
        recheck.restart()
    }

    readonly property var btAdapter: Bluetooth.defaultAdapter

    // ── Các ô ──────────────────────────────────────────────────────────────
    readonly property var tiles: {
        const list = []
        const eth = ethernet.find(d => d.state === "connected") ?? ethernet[0]
        if (eth)
            list.push({
                id: "wired", label: "Wired", icon: "network-wired-activated", category: "devices",
                active: eth.state === "connected",
                sub: eth.state === "connected" ? eth.connection
                    : eth.state === "unavailable" ? "Cable unplugged" : "Disconnected"
            })
        const wl = wifi.find(d => d.state === "connected") ?? wifi[0]
        if (wl)
            list.push({
                id: "wifi", label: "Wi-Fi", icon: "network-wireless-connected-100", category: "status",
                active: wifiRadio, more: true,
                sub: !wifiRadio ? "Off" : wl.state === "connected" ? wl.connection : "Not connected"
            })
        if (btAdapter) {
            const connected = (btAdapter.devices ? btAdapter.devices.values : []).filter(d => d.connected)
            list.push({
                id: "bluetooth", label: "Bluetooth", icon: "network-bluetooth", category: "devices",
                active: btAdapter.enabled,
                sub: !btAdapter.enabled ? "Off" : connected.length > 0 ? connected.map(d => d.name).join(", ") : "On"
            })
        }
        list.push({
            id: "night", label: "Night light", icon: "redshift-status-on", category: "status",
            active: shell.prefs.nightLight,
            sub: shell.prefs.nightLight && shell.prefs.nightUnsupported ? "Not supported" : ""
        })
        list.push({
            id: "dnd", label: "Do not disturb", icon: "samhie-do-not-disturb", category: "actions",
            active: shell.notifs.doNotDisturb, sub: ""
        })
        list.push({
            id: "dark", label: "Dark mode", icon: "samhie-dark-style", category: "actions",
            active: !shell.prefs.lightMode, sub: ""
        })
        const ai = shell.assistant
        list.push({
            id: "sbot", label: "S-bot", icon: "samhie-assistant", category: "actions",
            active: ai.open,
            sub: ai.server === "ready" ? "Ready"
                : ai.server === "checking" || ai.server === "starting" ? "Waking up…"
                : ai.server === "downloading" ? "Downloading…" : ""
        })
        return list
    }

    function setWifi(on) {
        wifiRadio = on
        run(["nmcli", "radio", "wifi", on ? "on" : "off"])
    }

    // Bật/tắt Bluetooth bằng rfkill như GNOME: systemd-rfkill nhớ trạng thái
    // qua mỗi lần khởi động. Chỉ tắt nguồn (adapter.enabled) thì BlueZ
    // (AutoEnable) bật lại lúc khởi động (user 2026-10-24). Mở khoá thì BlueZ
    // tự bật nguồn (đo máy thật); vẫn đặt enabled cho trường hợp đang tắt
    // nguồn mà không bị khoá. Trang Bluetooth của Settings làm y hệt.
    function setBluetooth(on) {
        Quickshell.execDetached(["rfkill", on ? "unblock" : "block", "bluetooth"])
        if (on)
            btAdapter.enabled = true
    }

    function toggle(id) {
        switch (id) {
        case "wired": {
            const eth = ethernet.find(d => d.state === "connected") ?? ethernet[0]
            const on = eth.state === "connected"
            // Đổi hình ngay; lần hỏi lại sau 600 ms chốt trạng thái thật.
            eth.state = on ? "disconnected" : "connected"
            netDevices = netDevices.slice()
            run(["nmcli", "device", on ? "disconnect" : "connect", eth.device])
            break
        }
        case "wifi":
            setWifi(!wifiRadio)
            break
        case "bluetooth":
            setBluetooth(!btAdapter.enabled)
            break
        case "night":
            shell.prefs.nightLight = !shell.prefs.nightLight
            break
        case "dnd":
            shell.notifs.doNotDisturb = !shell.notifs.doNotDisturb
            break
        case "dark":
            shell.prefs.lightMode = !shell.prefs.lightMode
            break
        case "sbot":
            bar.popup = ""
            shell.assistant.toggle()
            break
        }
    }

    // ── Giao diện ──────────────────────────────────────────────────────────
    Rectangle {
        id: card
        width: 360
        height: content.height + 24
        radius: 12
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        // Hút cú bấm vào chỗ trống trong thẻ — không thì nó lọt xuống lớp
        // phủ (PopupLayer) và đóng popup.
        MouseArea { anchors.fill: parent }

        Column {
            id: content
            x: 12
            y: 12
            width: parent.width - 24
            spacing: 12

            VolumeRow {
                visible: root.page === ""
                width: parent.width
                onMoreClicked: root.page = "sound"
            }

            SoundPage {
                visible: root.page === "sound"
                width: parent.width
                qs: root
            }

            WifiPage {
                visible: root.page === "wifi"
                width: parent.width
                qs: root
            }

            Grid {
                id: grid
                visible: root.page === ""
                columns: 2
                spacing: 8
                readonly property real cellWidth: (width - spacing) / 2
                width: parent.width

                // Mảng thường: mỗi lần đổi là dựng lại các ô — ô không giữ
                // trạng thái gì riêng nên không sao.
                Repeater {
                    model: root.tiles
                    delegate: QuickTile {
                        required property var modelData
                        width: grid.cellWidth
                        label: modelData.label
                        sub: modelData.sub
                        iconName: modelData.icon
                        iconCategory: modelData.category
                        active: modelData.active
                        more: modelData.more ?? false
                        onClicked: root.toggle(modelData.id)
                        onMoreClicked: root.page = modelData.id
                    }
                }
            }
        }
    }
}
