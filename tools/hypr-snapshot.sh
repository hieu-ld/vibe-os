#!/bin/bash
# Chụp trạng thái Hyprland + Quickshell khi màn hình "kẹt" (vd cửa sổ Kate
# vẫn là cửa sổ đang chọn nhưng không thấy, Super và bấm vào app không ăn).
# Chạy được cả từ TTY (Ctrl+Alt+F3, đăng nhập, rồi chạy script; Ctrl+Alt+F1/F2
# để về): tự tìm phiên Hyprland đang chạy.
#   tools/hypr-snapshot.sh   → ~/.cache/samhie-snapshot-<giờ>/
set -u
out=$HOME/.cache/samhie-snapshot-$(date +%Y%m%d-%H%M%S)
mkdir -p "$out"
export XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-/run/user/$(id -u)}
if [ -z "${HYPRLAND_INSTANCE_SIGNATURE:-}" ]; then
    HYPRLAND_INSTANCE_SIGNATURE=$(ls -t "$XDG_RUNTIME_DIR/hypr" | head -1)
    export HYPRLAND_INSTANCE_SIGNATURE
fi
export WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-wayland-1}

for c in clients activewindow activeworkspace workspaces layers monitors devices binds; do
    hyprctl -j "$c" >"$out/$c.json" 2>&1
done
# Thuộc tính từng cửa sổ (độ mờ, bị ẩn…) — thứ clients không in.
hyprctl -j clients 2>/dev/null | jq -r '.[].address' | while read -r a; do
    for p in opacity opacity_override opaque no_focus; do
        printf '%s %s %s\n' "$a" "$p" "$(hyprctl getprop "address:$a" "$p" 2>&1)"
    done
done >"$out/props.txt"
grim "$out/screen.png" 2>"$out/grim.err"
tail -n 300 "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" >"$out/hyprland.log.tail"
journalctl --user --since "-10min" --no-pager >"$out/journal.txt" 2>&1
ps -eo pid,stat,etime,pcpu,rss,comm --sort=-pcpu | head -30 >"$out/ps.txt"
echo "$out"
