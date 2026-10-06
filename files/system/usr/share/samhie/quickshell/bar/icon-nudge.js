// SINH TỰ ĐỘNG bởi tools/build-icons.py (bảng NUDGE) — đừng sửa tay.
.pragma library

// tên icon → [cỡ px, nửa điểm ảnh dời ngang, dọc]: −1 trái/lên, 1 phải/xuống.
var NUDGE = {
    "dialog-ok": [16, 0, -1],
    "document-edit": [16, -1, -1],
    "edit-clear": [16, -1, -1],
    "edit-clear-locationbar-ltr": [16, -1, -1],
    "edit-clear-locationbar-rtl": [16, -1, -1],
    "edit-entry": [16, -1, -1],
    "folder-network": [20, 1, -1],
    "folder-new": [16, 0, -1],
    "go-down": [20, -1, 0],
    "go-home": [20, 0, -1],
    "go-next": [20, 0, -1],
    "go-previous": [20, 0, -1],
    "go-up": [20, -1, 0],
    "network-bluetooth": [20, -1, -1],
    "network-wireless-connected-100": [20, -1, 1],
    "network-wireless-signal-excellent": [20, -1, 1],
    "network-wireless-signal-good": [20, -1, 1],
    "network-wireless-signal-none": [20, -1, 1],
    "network-wireless-signal-ok": [20, -1, 1],
    "network-wireless-signal-weak": [20, -1, 1],
    "notification-active": [18, -1, 0],
    "notification-inactive": [18, -1, 0],
    "notifications-disabled": [18, -1, 0],
    "samhie-do-not-disturb": [20, 0, -1],
    "samhie-folder-download": [20, -1, -1],
    "samhie-power": [18, -1, 0],
    "samhie-user-home": [20, 0, -1],
    "system-log-out": [20, 0, 1],
    "utilities-terminal": [16, -1, -1],
    "view-filter": [16, 0, -1],
    "view-list-icons": [22, -1, -1],
    "view-refresh": [16, -1, 0],
    "view-split-left-right": [16, -1, -1],
};

// {x, y} (−1/0/1) của icon ở đúng cỡ đó; cỡ khác thì {0, 0}.
function half(name, size) {
    const n = NUDGE[name];
    return n && n[0] === size ? { x: n[1], y: n[2] } : { x: 0, y: 0 };
}
