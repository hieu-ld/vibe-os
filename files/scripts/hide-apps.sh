#!/usr/bin/env bash
# Ẩn công cụ hệ thống khỏi ô tìm app (màn tổng quan) — app VẪN CÒN, chỉ thêm
# NoDisplay=true vào file .desktop (user chốt 2026-10-22: "có những app mình
# không biết"). Gõ lệnh trong Konsole vẫn chạy được.
#   htop, nvtop              xem tiến trình / GPU — image nền cài, chạy trong terminal
#   uuctl                    quản lý unit của uwsm (đi kèm hyprland-uwsm)
#   kvantummanager           đổi theme Qt — bấm nhầm là phá giao diện Samhie
#   kbd-layout-viewer5       xem sơ đồ phím (đi kèm fcitx5-configtool)
#   fcitx5-configtool        đã mở được từ Settings → Mouse & Keyboard
set -oue pipefail

for app in htop nvtop uuctl kvantummanager kbd-layout-viewer5 fcitx5-configtool; do
    f=/usr/share/applications/$app.desktop
    # Gói đổi tên file thì dừng build — đừng lặng lẽ hiện lại app.
    [[ -f $f ]] || { echo "hide-apps.sh: thiếu $f" >&2; exit 1; }
    sed -i '/^NoDisplay=/d; /^\[Desktop Entry\]/a NoDisplay=true' "$f"
done
