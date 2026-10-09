// Thao tác với cửa sổ, dùng chung cho thanh trên (bar/WindowControls.qml,
// bar/WindowList.qml) và dock (dock/Dock.qml): thu nhỏ, hiện lại, focus,
// đóng, phóng to.
//
// THU NHỎ kiểu Ubuntu (user 2026-10-02): Hyprland không có trạng thái này,
// nên thu nhỏ = chuyển cửa sổ vào màn ảo đặc biệt `special:minimized` — màn
// ẩn, không bao giờ hiện (không ai gọi togglespecialworkspace với nó). Cụm
// chấm workspace và màn tổng quan chỉ lấy workspace id > 0 nên không thấy
// nó. Nhớ cửa sổ đến từ màn ảo nào để HIỆN LẠI đúng chỗ đó; Quickshell chạy
// lại thì mất trí nhớ này → hiện ở màn ảo đang xem.
// Đo trong container (2026-10-02): thu nhỏ → cửa sổ còn lại tự nhận focus và
// giãn kín; `move … follow = true` từ màn ảo khác → Hyprland nhảy về màn đó,
// cửa sổ về đúng ô cũ và được focus.
//
// Lệnh viết kiểu Lua của Hyprland 0.56; nhắm cửa sổ bằng `address:0x…`
// (HyprlandToplevel.address không có "0x").
pragma Singleton
import QtQuick
import Quickshell
import Quickshell.Hyprland

Singleton {
    id: root

    readonly property string minimizedName: "special:minimized"

    // address → id màn ảo trước khi thu nhỏ.
    property var minimizedFrom: ({})

    // Thứ tự focus, mới nhất trước (address) — dock chọn cửa sổ dùng gần đây
    // nhất của app. Hyprland không cho hỏi cái này.
    property var focusOrder: []
    readonly property var active: Hyprland.activeToplevel
    onActiveChanged: {
        if (!active || active.address === "")
            return
        focusOrder = [active.address].concat(focusOrder.filter(a => a !== active.address))
    }

    // Cửa sổ thật: thuộc một màn ảo thường (id > 0) hoặc đã thu nhỏ. Quickshell
    // còn thấy cửa sổ không thuộc màn ảo nào — vd "Fcitx5 Input Window" (khung
    // gợi ý ẩn của bộ gõ, không có trong `hyprctl clients`) — bỏ đi. Màn đặc
    // biệt khác (scratchpad người dùng tự đặt) cũng bỏ.
    readonly property var windows: Hyprland.toplevels.values
        .filter(t => t.workspace !== null && (t.workspace.id > 0 || t.workspace.name === minimizedName))

    // Cửa sổ đang focus THẬT: Hyprland vẫn giữ cửa sổ cũ là active khi sang
    // màn ảo trống, hoặc khi vừa thu nhỏ cửa sổ cuối cùng.
    readonly property var focused: active && active.workspace && active.workspace.focused ? active : null

    function sel(t) {
        return '"address:0x' + t.address + '"'
    }

    function isMinimized(t) {
        return t !== null && t.workspace !== null && t.workspace.name === minimizedName
    }

    // Màn ảo "của" cửa sổ: cửa sổ thu nhỏ tính theo màn nó đến từ.
    function homeWorkspace(t) {
        if (isMinimized(t))
            return minimizedFrom[t.address] ?? 0
        return t.workspace ? t.workspace.id : 0
    }

    function minimize(t) {
        if (!t || isMinimized(t) || !t.workspace)
            return
        const m = Object.assign({}, minimizedFrom)
        m[t.address] = t.workspace.id
        minimizedFrom = m
        Hyprland.dispatch("hl.dsp.window.move({ window = " + sel(t)
            + ', workspace = "' + minimizedName + '", follow = false })')
    }

    // Đưa cửa sổ ra trước: đang thu nhỏ thì trả về màn cũ, ở màn ảo khác thì
    // Hyprland tự nhảy sang đó.
    function activate(t) {
        if (!t)
            return
        if (isMinimized(t)) {
            let ws = minimizedFrom[t.address]
            if (!ws) {
                const m = Hyprland.focusedMonitor
                ws = m && m.activeWorkspace ? m.activeWorkspace.id : 1
            }
            const m = Object.assign({}, minimizedFrom)
            delete m[t.address]
            minimizedFrom = m
            Hyprland.dispatch("hl.dsp.window.move({ window = " + sel(t)
                + ", workspace = " + ws + ", follow = true })")
        }
        Hyprland.dispatch("hl.dsp.focus({ window = " + sel(t) + " })")
        // focus KHÔNG đưa cửa sổ nổi lên trên (Hyprland 0.56: fullWindowFocus
        // chỉ đổi tab trong nhóm) — bấm Konsole trên dock mà nó vẫn nằm dưới
        // Dolphin (user 2026-10-09). Bấm chuột vào cửa sổ thì Hyprland tự đưa lên.
        Hyprland.dispatch('hl.dsp.window.alter_zorder({ mode = "top", window = ' + sel(t) + " })")
    }

    function close(t) {
        if (t)
            Hyprland.dispatch("hl.dsp.window.close({ window = " + sel(t) + " })")
    }

    // Phóng to = fullscreen kiểu "maximized": kín vùng làm việc, thanh trên
    // vẫn thấy.
    function toggleMaximize(t) {
        if (t)
            Hyprland.dispatch("hl.dsp.window.fullscreen({ window = " + sel(t)
                + ', mode = "maximized", action = "toggle" })')
    }

    // Cửa sổ của một app (file .desktop) — app id của cửa sổ không phải lúc
    // nào cũng trùng tên file (.desktop của VS Code là com.microsoft.VSCode,
    // cửa sổ là "code") nên tra qua heuristicLookup. Dùng gần đây nhất trước.
    function windowsOf(entry) {
        const list = windows.filter(t => {
            const id = t.wayland ? t.wayland.appId : ""
            if (id === "")
                return false
            const e = DesktopEntries.heuristicLookup(id)
            return e !== null && e.id === entry.id
        })
        return list.sort((a, b) => rank(a) - rank(b))
    }
    function rank(t) {
        const i = focusOrder.indexOf(t.address)
        return i < 0 ? 1e6 : i
    }
}
