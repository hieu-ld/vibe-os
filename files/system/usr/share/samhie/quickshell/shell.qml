// Giao diện Quickshell của Samhie (giai đoạn 3) — Hyprland chạy lúc đăng nhập:
//     quickshell -n -p /usr/share/samhie/quickshell
// (xem samhie.shell trong /usr/share/samhie/hypr/samhie.lua). Mỗi phần một
// thư mục con; screenshot-*.qml là hai giao diện chạy riêng theo lệnh chụp.
//
// Thanh trên: mỗi màn hình một thanh (bar/Bar.qml). Bố cục như bản 1.0 (user chốt
// 2026-10-01): trái là cụm workspace, giữa là đồng hồ tiếng Việt (bấm ra
// đồng hồ kim + lịch âm dương), phải là khay hệ thống (bộ gõ hiện chữ
// en/vn) và nút nguồn.
//
// Màn tổng quan (overview/Overview.qml): nhấn-thả phím Win, Win+R/Win+Space,
// hoặc bấm cụm workspace trên thanh. Mở trên màn hình đang focus.
//
// Dock (dock/): trên desktop hiện khi không cửa sổ nào đè lên chỗ của nó
// (DesktopDock.qml), trong tổng quan luôn có.
//
// Thông báo (notifications/): popup góc trên-phải, chuông trên thanh mở danh
// sách — thay mako.
//
// Lịch sử clipboard (clipboard/): Win+V, hiện cạnh con trỏ — thay fuzzel.
import QtQuick
import Quickshell
import Quickshell.Hyprland
import Quickshell.Io
import "bar"
import "clipboard"
import "dock"
import "notifications"
import "quicksettings"
import "overview"

ShellRoot {
    // Không đặt id `shell`: Bar/Overview có thuộc tính tên `shell`, viết
    // `shell: shell` thì vế phải trỏ vào chính thuộc tính đó (undefined).
    id: samhie

    // Máy chủ thông báo (thay mako): popup góc trên-phải + danh sách của chuông.
    readonly property alias notifs: notifications
    Notifications { id: notifications }
    NotificationPopups { notifs: notifications }

    // Thiết lập người dùng đổi trong quick settings (nhớ qua lần đăng nhập):
    // Night light, sáng/tối, Đừng làm phiền.
    // Không đặt tên `settings`: ShellRoot đã có thuộc tính đó (thiết lập của
    // chính Quickshell).
    readonly property alias prefs: userSettings
    Settings {
        id: userSettings
        notifs: notifications
    }

    // Tên màn hình (Hyprland) đang mở tổng quan; "" = đóng.
    property string overviewMonitor: ""

    function openOverview() {
        const m = Hyprland.focusedMonitor
        overviewMonitor = m ? m.name : ""
        overviewPrepare = ""
    }

    // Màn hình đang DỰNG SẴN tổng quan (đang giữ Win, chưa thả) — xem
    // `preparing` trong Overview.qml. Win+phím khác thì không bao giờ có lệnh
    // mở: bỏ sau 1,5 s.
    property string overviewPrepare: ""
    function prepareOverview() {
        if (overviewMonitor !== "")
            return
        const m = Hyprland.focusedMonitor
        overviewPrepare = m ? m.name : ""
        prepareTimeout.restart()
    }
    Timer {
        id: prepareTimeout
        interval: 1500
        onTriggered: samhie.overviewPrepare = ""
    }
    // samhie.lua gọi lúc NHẤN Win trái (hl.on input.keyboard.key).
    GlobalShortcut {
        appid: "samhie"
        name: "overview-prepare"
        description: "Prepare the overview while Super is held"
        onPressed: samhie.prepareOverview()
    }
    function closeOverview() {
        overviewMonitor = ""
    }
    function toggleOverview() {
        if (overviewMonitor !== "")
            closeOverview()
        else
            openOverview()
    }

    // Các màn ảo của một màn hình, như Ubuntu (user chốt 2026-10-01): các
    // workspace đang có + LUÔN một màn trống ở cuối, ít nhất 2. Chấm trên
    // thanh (bar/Workspaces.qml) và cột trái màn tổng quan dùng CHUNG danh
    // sách này. [{ id, active, workspace (null nếu là màn trống chưa có) }]
    // — Hyprland tự tạo workspace khi nhảy tới và tự xoá cái trống khi rời đi.
    function workspaceSlots(monitor) {
        const existing = Hyprland.workspaces.values
            .filter(w => w.id > 0 && w.monitor === monitor)
            .sort((a, b) => a.id - b.id)
        const list = existing.map(w => ({ id: w.id, active: w.active, workspace: w }))
        const last = existing[existing.length - 1]
        if (existing.length < 2 || last.toplevels.values.length > 0) {
            // id nhỏ nhất lớn hơn màn cuối mà chưa màn hình nào dùng (id của
            // workspace là chung cho mọi màn hình).
            const used = Hyprland.workspaces.values.map(w => w.id)
            let id = last ? last.id + 1 : 1
            while (used.indexOf(id) >= 0)
                id++
            list.push({ id: id, active: false, workspace: null })
        }
        return list
    }

    // samhie.lua gắn phím vào đây: hl.dsp.global("samhie:overview") cho
    // Win+R / Win+Space (bấm thường: Hyprland gửi "nhấn" rồi "thả").
    GlobalShortcut {
        appid: "samhie"
        name: "overview"
        description: "Open or close the overview"
        onPressed: samhie.toggleOverview()
    }

    // Phím Win đứng riêng gắn KIỂU "KHI THẢ" (release = true). Đo trong máy
    // ảo 2026-10-02 (45 lần bấm, chậm lẫn spam): mỗi lần bấm Hyprland gửi
    // ĐÚNG MỘT sự kiện "released", không bao giờ "pressed". Bản trước nghe
    // cả hai và bỏ qua mọi sự kiện trong 400 ms sau đó — spam Win (cách nhau
    // 250–350 ms) thì mất nhịp (user: bấm 5 lần, lần 3 và 5 không ăn).
    GlobalShortcut {
        appid: "samhie"
        name: "overview-key"
        description: "Open or close the overview (Super key alone)"
        onReleased: samhie.toggleOverview()
    }

    // Lịch sử clipboard: Win+V → hl.dsp.global("samhie:clipboard").
    Clipboard { id: clipboard }
    GlobalShortcut {
        appid: "samhie"
        name: "clipboard"
        description: "Open or close the clipboard history"
        onPressed: {
            samhie.closeOverview()
            clipboard.toggle()
        }
    }
    // Hình nền cho màn tổng quan (đường dẫn thật, không symlink). Lúc mở:
    // SAMHIE_WALLPAPER(_LIGHT) do samhie.lua đặt. App Settings (trang
    // Appearance) chọn hình / về mặc định → `wallpaper set <tối> <sáng>`.
    property string wallpaperDark: Quickshell.env("SAMHIE_WALLPAPER") || "/usr/share/backgrounds/samhie/samhie.png"
    property string wallpaperLight: Quickshell.env("SAMHIE_WALLPAPER_LIGHT") || "/usr/share/backgrounds/samhie/samhie-light.png"
    IpcHandler {
        target: "wallpaper"
        function set(dark: string, light: string): void {
            samhie.wallpaperDark = dark
            samhie.wallpaperLight = light
        }
    }

    // Sáng/tối từ app Settings (trang Appearance) — đổi giống bấm ô Dark mode
    // của quick settings (quicksettings/Settings.qml lưu + áp dụng).
    IpcHandler {
        target: "prefs"
        function setLightMode(on: bool): void {
            samhie.prefs.lightMode = on
        }
    }

    // Đổi múi giờ (app Settings gọi `clock refresh` sau timedatectl
    // set-timezone): SystemClock của đồng hồ thanh trên giữ giờ cũ tới khi
    // sang phút mới (máy ảo 2026-10-21) — báo để nó đọc lại (bar/Clock.qml).
    property int clockSerial: 0
    IpcHandler {
        target: "clock"
        function refresh(): void {
            samhie.clockSerial++
        }
    }

    IpcHandler {
        target: "clipboard"
        function toggle(): void { clipboard.toggle() }
    }

    // Gọi từ dòng lệnh: qs -p /usr/share/samhie/quickshell ipc call overview toggle
    IpcHandler {
        target: "overview"
        function toggle(): void { samhie.toggleOverview() }
        function open(): void { samhie.openOverview() }
        function close(): void { samhie.closeOverview() }
    }

    Variants {
        model: Quickshell.screens

        Scope {
            id: perScreen
            required property ShellScreen modelData

            Bar {
                shell: samhie
                screen: perScreen.modelData
            }

            Overview {
                id: overview
                shell: samhie
                screen: perScreen.modelData
                desktopDock: desktopDock
            }

            DesktopDock {
                id: desktopDock
                shell: samhie
                screen: perScreen.modelData
                overview: overview
            }
        }
    }
}
