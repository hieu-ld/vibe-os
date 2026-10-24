// Trang Bluetooth — dáng trang Bluetooth của macOS System Settings:
//
//   ┌──────────────────────────────────────────────────────┐
//   │ Bluetooth                                       [●━] │
//   └──────────────────────────────────────────────────────┘
//   My Devices
//   │ (tai nghe) WH-1000XM4                [Disconnect] [Forget] │
//   │            Connected · Battery 80%                         │
//   │ (chuột)    MX Master 3                 [Connect] [Forget] │
//   │            Not connected                                   │
//   Nearby Devices
//   │ (đt)       Pixel 8                               [Connect] │
//
// Đọc/ghi BlueZ qua module Bluetooth của Quickshell (D-Bus, không gọi lệnh).
// Trang mở + Bluetooth bật thì quét (adapter.discovering); đổi trang / đóng
// app là thôi quét (BlueZ cũng tự dừng khi tiến trình đã bắt đầu quét thoát).
// Máy KHÔNG bật chế độ cho máy khác thấy (discoverable): ghép luôn bắt đầu từ
// máy này, đỡ phơi máy ra cho người lạ.
//
// Ghép: Quickshell 0.2.1 không đăng ký agent của BlueZ (không có chuỗi
// RegisterAgent trong /usr/bin/quickshell — đo 2026-10-21) → BlueZ ghép kiểu
// "Just Works" (máy này coi như không màn hình, không phím). Tai nghe, chuột,
// loa, tay cầm: đủ. Thiết bị BẮT gõ mã PIN (bàn phím Bluetooth đời cũ) sẽ
// không ghép được — chưa thử máy thật.
// Ghép xong thì đặt trusted (lần sau thiết bị tự kết nối lại được) rồi kết
// nối luôn, như bấm Connect ở macOS.
import QtQuick
import Quickshell
import Quickshell.Bluetooth
import ".."
import "../bar"
import "../colors.js" as Colors
import "rows.js" as Rows

Column {
    id: root

    property string title

    readonly property BluetoothAdapter adapter: Bluetooth.defaultAdapter
    readonly property bool powered: adapter !== null && adapter.enabled
    readonly property var all: adapter ? adapter.devices.values : []

    // Đã ghép: đang kết nối lên đầu, rồi theo tên.
    readonly property var mine: all.filter(d => d.paired)
        .sort((a, b) => (b.connected - a.connected) || a.name.localeCompare(b.name))
    // Chưa ghép, quét thấy: chỉ thiết bị có tên (BLE quanh nhà — TV, đồng hồ
    // của hàng xóm — hay không tên, chỉ có địa chỉ; macOS cũng giấu).
    readonly property var nearby: all.filter(d => !d.paired && d.deviceName !== "")
        .sort((a, b) => a.name.localeCompare(b.name))

    // ── Quét ───────────────────────────────────────────────────────────────
    function scan() {
        if (powered && !adapter.discovering)
            adapter.discovering = true
    }
    onPoweredChanged: scan()
    Component.onCompleted: scan()
    Component.onDestruction: {
        if (adapter && adapter.discovering)
            adapter.discovering = false
    }

    // ── Kết nối / ghép (một thiết bị mỗi lúc) ─────────────────────────────
    // `target`: thiết bị vừa bấm Connect — dòng của nó hiện "Connecting…" từ
    // lúc bấm (BlueZ báo trạng thái chậm một nhịp). `failed`: địa chỉ thiết
    // bị lần thử cuối không được → dòng hiện "Couldn't connect" màu đỏ.
    property BluetoothDevice target: null
    property string failed: ""

    function connectTo(d) {
        failed = ""
        target = d
        giveUp.restart()
        if (d.paired)
            d.connect()
        else
            d.pair()
    }
    function finish(ok) {
        if (!ok && target)
            failed = target.address
        target = null
        giveUp.stop()
    }
    Connections {
        target: root.target
        function onPairedChanged() {
            const d = root.target
            if (d.paired) {
                d.trusted = true
                d.connect()
            }
        }
        // pairing → false trước hay sau paired → true thì tuỳ BlueZ gửi —
        // đợi một chút rồi mới xem có ghép được không.
        function onPairingChanged() {
            if (!root.target.pairing)
                pairCheck.restart()
        }
        function onConnectedChanged() {
            if (root.target.connected)
                root.finish(true)
        }
        function onStateChanged() {
            if (root.target.paired && root.target.state === BluetoothDeviceState.Disconnected
                    && !giveUp.justStarted)
                root.finish(false)
        }
    }
    Timer {
        id: pairCheck
        interval: 500
        onTriggered: if (root.target && !root.target.paired) root.finish(false)
    }
    // Lệnh kết nối lỗi ngay (BlueZ trả lỗi D-Bus) thì không có đổi trạng thái
    // nào — sau 30 s vẫn chưa xong là coi như hỏng. `justStarted`: nửa giây
    // đầu, trạng thái Disconnected còn là của lúc trước khi bấm.
    Timer {
        id: giveUp
        property bool justStarted: false
        interval: 30000
        onRunningChanged: if (running) { justStarted = true; settle.restart() }
        onTriggered: root.finish(false)
    }
    Timer {
        id: settle
        interval: 500
        onTriggered: giveUp.justStarted = false
    }

    // Icon theo `icon` BlueZ gửi (tên freedesktop theo lớp thiết bị).
    function iconFor(d) {
        const i = d.icon
        if (i.startsWith("audio-head"))
            return "audio-headphones"
        if (i === "audio-card")
            return "audio-speakers"
        if (i === "input-mouse" || i === "input-tablet")
            return "input-mouse"
        if (i === "input-keyboard" || i === "input-gaming" || i === "phone")
            return i
        if (i === "computer")
            return "computer-laptop"
        return "network-bluetooth"
    }

    width: parent ? parent.width : 0
    spacing: 20

    // Dòng thiết bị: icon + tên + trạng thái, nút cuối dòng.
    component DeviceRow: Item {
        id: row
        required property BluetoothDevice modelData
        readonly property BluetoothDevice device: modelData
        readonly property bool busy: device.pairing || root.target === device
            || device.state === BluetoothDeviceState.Connecting
            || device.state === BluetoothDeviceState.Disconnecting
        readonly property string status: busy
            ? (device.pairing ? "Pairing…"
               : device.state === BluetoothDeviceState.Disconnecting ? "Disconnecting…" : "Connecting…")
            : root.failed === device.address ? "Couldn't connect"
            : device.connected ? "Connected" + (device.batteryAvailable
                ? " · Battery " + Math.round(device.battery * 100) + "%" : "")
            : device.paired ? "Not connected" : ""
        readonly property bool first: Rows.isFirst(row)

        width: parent.width
        height: status === "" ? 48 : 60

        Rectangle {
            visible: !row.first
            x: 16
            width: parent.width - 32
            height: 1
            color: Theme.c.subSecondary
        }
        SamhieIcon {
            x: 16
            anchors.verticalCenter: parent.verticalCenter
            category: "devices"
            iconName: root.iconFor(row.device)
            size: 20
            color: Theme.c.text
        }
        Column {
            x: 48
            width: buttons.x - x - 12
            anchors.verticalCenter: parent.verticalCenter
            Text {
                width: parent.width
                elide: Text.ElideRight
                text: row.device.name
                color: Theme.c.text
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 26
            }
            Text {
                visible: row.status !== ""
                width: parent.width
                elide: Text.ElideRight
                text: row.status
                color: root.failed === row.device.address && !row.busy ? Theme.c.danger : Theme.c.textMuted
                font.family: Colors.fonts.ui
                font.weight: Colors.fonts.uiWeight
                font.pixelSize: 22
            }
        }
        Row {
            id: buttons
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            Button {
                visible: row.device.connected
                label: "Disconnect"
                enabled: !row.busy
                onClicked: row.device.disconnect()
            }
            Button {
                visible: !row.device.connected
                label: "Connect"
                enabled: !row.busy && root.target === null
                onClicked: root.connectTo(row.device)
            }
            Button {
                visible: row.device.paired
                label: "Forget"
                enabled: !row.busy
                onClicked: {
                    if (root.failed === row.device.address)
                        root.failed = ""
                    row.device.forget()
                }
            }
        }
    }

    Group {
        InfoRow {
            visible: root.adapter === null
            label: "No Bluetooth adapter found"
        }
        SettingRow {
            visible: root.adapter !== null
            label: "Bluetooth"
            // Tắt = khoá rfkill (như quick settings, QuickSettings.qml
            // setBluetooth) nên "Blocked" là trạng thái tắt bình thường.
            sub: root.adapter === null ? ""
                : root.adapter.state === BluetoothAdapterState.Enabling ? "Turning on…"
                : root.adapter.state === BluetoothAdapterState.Disabling ? "Turning off…" : ""
            Switch {
                checked: root.powered
                onToggled: on => {
                    Quickshell.execDetached(["rfkill", on ? "unblock" : "block", "bluetooth"])
                    if (on)
                        root.adapter.enabled = true
                }
            }
        }
    }

    Group {
        visible: root.powered && root.mine.length > 0
        title: "My Devices"
        Repeater {
            model: root.mine
            DeviceRow {}
        }
    }

    Group {
        visible: root.powered
        title: "Nearby Devices"
        Repeater {
            model: root.nearby
            DeviceRow {}
        }
        InfoRow {
            visible: root.nearby.length === 0
            label: root.adapter && root.adapter.discovering ? "Searching…" : "No devices found"
        }
    }
}
