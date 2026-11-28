// App ghim trên dock + menu chuột phải của dock / kết quả tìm app (user
// 2026-11-21: "pin app vào dock"). Dùng chung cho mọi dock (desktop mỗi màn
// hình + màn tổng quan) nên là singleton.
//
// Mô hình "mặc định + phần riêng" (CLAUDE.md): danh sách MẶC ĐỊNH nằm ở đây
// (trong image); người dùng ghim / bỏ ghim / kéo đổi chỗ lần đầu thì ghi cả
// danh sách ra ~/.config/samhie/dock.json — từ đó dock theo file đó, image
// không bao giờ ghi đè. Cái giá: app mới image thêm vào mặc định sau này
// không tự lên dock của người đã sửa (họ tự ghim). Xoá file = về mặc định.
//
// Menu (`menu`): màn hình, x, y, entry, up — x, y là toạ độ TRONG màn
// hình (logic, gốc trái trên của màn) của điểm neo: giữa cạnh trên icon dock
// (menu mọc lên trên) hoặc chỗ con trỏ (lưới tìm app, menu mọc xuống).
// DockMenuLayer của đúng màn hình đó vẽ menu.
// LƯU Ý: chú thích trên `pragma Singleton` KHÔNG được có dấu ngoặc nhọn —
// Quickshell quét pragma gặp ngoặc là dừng, file thành kiểu thường, mọi hàm
// "is not a function" (đo 2026-11-21).
pragma Singleton
import QtQuick
import Quickshell
import Quickshell.Io

Singleton {
    id: root

    // id file .desktop (không đuôi). App nào chưa cài thì dock bỏ qua.
    readonly property var defaults: ["samhie-settings", "org.kde.dolphin", "org.kde.konsole", "org.kde.kate", "com.microsoft.Edge"]
    property var pinned: defaults
    property bool custom: false   // đã có file của người dùng

    function isPinned(id) { return pinned.indexOf(id) >= 0 }

    function pin(id) {
        if (isPinned(id))
            return
        set(pinned.concat([id]))
    }
    function unpin(id) {
        set(pinned.filter(x => x !== id))
    }
    // Kéo đổi chỗ: đưa mục ở `from` về vị trí `to` (chỉ số trong `pinned`).
    function move(from, to) {
        if (from === to || from < 0 || from >= pinned.length)
            return
        const list = pinned.slice()
        const [id] = list.splice(from, 1)
        list.splice(Math.max(0, Math.min(to, list.length)), 0, id)
        set(list)
    }
    function set(list) {
        pinned = list
        custom = true
        store.adapter.pinned = list
        store.writeAdapter()
    }

    FileView {
        id: store
        path: (Quickshell.env("XDG_CONFIG_HOME") || Quickshell.env("HOME") + "/.config") + "/samhie/dock.json"
        // Đọc chặn (file vài trăm byte): dock dựng lên là đúng danh sách
        // luôn, không chớp danh sách mặc định rồi mới đổi.
        blockLoading: true
        // Chưa có file: dùng mặc định, ghi ra ở lần sửa đầu tiên (writeAdapter
        // tự tạo thư mục samhie/).
        // adapter.pinned đọc từ JSON là danh sách kiểu Qt (QVariantList), KHÔNG
        // phải mảng JS: Array.isArray = false (đo 2026-11-28) → trước đây bỏ qua
        // file, dock về mặc định sau mỗi lần khởi động, ghim lần sau ghi đè mất
        // danh sách cũ. Chép từng phần tử ra mảng JS thật.
        onLoaded: {
            const p = adapter.pinned
            if (p && typeof p.length === "number") {
                const list = []
                for (let i = 0; i < p.length; i++)
                    if (typeof p[i] === "string")
                        list.push(p[i])
                root.pinned = list
                root.custom = true
            }
        }
        JsonAdapter {
            property var pinned: null
        }
    }

    // ── Menu chuột phải ───────────────────────────────────────────────────
    property var menu: null
    function openMenu(screen, x, y, entry, up) {
        menu = { screen: screen, x: x, y: y, entry: entry, up: up }
    }
    function closeMenu() {
        menu = null
    }
}
