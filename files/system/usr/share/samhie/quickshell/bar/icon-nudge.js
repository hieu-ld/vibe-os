// SINH TỰ ĐỘNG bởi tools/build-icons.py (bảng NUDGE) — đừng sửa tay.
.pragma library

// tên icon → [cỡ px, nửa điểm ảnh dời ngang, dọc]: −1 trái/lên, 1 phải/xuống.
var NUDGE = {
    "arrow-down": [16, 0, -1],
    "arrow-left": [16, -1, 0],
    "arrow-right": [16, -1, 0],
    "arrow-up": [16, 0, -1],
    "audio-volume-high": [20, -1, -1],
    "audio-volume-low": [20, -1, -1],
    "audio-volume-medium": [20, -1, -1],
    "audio-volume-muted": [20, -1, -1],
    "drive-harddisk": [20, -1, -1],
    "folder-network": [20, -1, 0],
    "go-down": [20, -1, 0],
    "go-home": [20, -1, 0],
    "go-next": [20, 0, -1],
    "go-previous": [20, 0, -1],
    "go-up": [20, -1, 0],
    "samhie-folder-download": [20, -1, -1],
    "samhie-user-home": [20, -1, 0],
    "system-reboot": [20, 1, 1],
    "system-shutdown": [18, -1, 1],
    "user-trash": [20, 0, -1],
    "user-trash-full": [20, -1, -1],
    "window-close-symbolic": [18, -1, -1],
    "window-maximize-symbolic": [18, -1, -1],
    "window-minimize-symbolic": [18, 1, -1],
    "window-restore-symbolic": [18, -1, -1],
};

// {x, y} (−1/0/1) của icon ở đúng cỡ đó; cỡ khác thì {0, 0}.
function half(name, size) {
    const n = NUDGE[name];
    return n && n[0] === size ? { x: n[1], y: n[2] } : { x: 0, y: 0 };
}
