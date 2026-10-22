// Hiệu ứng mở/đóng chung cho popup của thanh trên (lịch, quick settings,
// thông báo, menu nguồn, danh sách cửa sổ, menu khay, bảng Claude) — user
// chốt 2026-10-22: "mượt như mở app trên dock". App mở: Hyprland phóng
// 87% → 100% + mờ dần (windowsIn/fadeIn, samhie.lua); popup làm nhẹ hơn
// vì nhỏ và nằm sát nút vừa bấm: 95% → 100% + mờ dần, 200 ms; đóng chạy
// ngược, nhanh hơn (120 ms) như cửa sổ app.
// Popup đặt `showing: open` thay cho `visible: open`; `transformOrigin` theo
// chỗ neo (mép phải → TopRight…) để popup "mọc" ra từ phía nút.
// PopupLayer giữ lớp hiện thêm một nhịp sau khi đóng (CLOSE_MS) cho kịp chạy.
import QtQuick

Item {
    id: motion

    property bool showing: false
    property real shown: 0

    readonly property int openMs: 200
    readonly property int closeMs: 120

    visible: shown > 0
    opacity: shown
    scale: 0.95 + 0.05 * shown
    transformOrigin: Item.Top

    onShowingChanged: {
        anim.stop()
        anim.to = showing ? 1 : 0
        anim.duration = showing ? openMs : closeMs
        anim.easing.type = showing ? Easing.OutCubic : Easing.InCubic
        anim.start()
    }
    NumberAnimation {
        id: anim
        target: motion
        property: "shown"
    }
}
