#!/bin/bash
# Thử nhanh Quickshell của repo trên máy đang chạy Samhie, khỏi build image:
# chép files/system/usr/share/samhie/quickshell ra ~/.cache/samhie-qs-dev rồi
# chạy nó THAY bản trong image (cùng biến môi trường như samhie.lua).
#   tools/qs-dev.sh           chạy bản của repo
#   tools/qs-dev.sh restore   về lại bản trong image (khởi động lại máy cũng vậy)
# Log: ~/.cache/samhie-qs-dev.log
repo=$(cd "$(dirname "$0")/.." && pwd)
dev=$HOME/.cache/samhie-qs-dev
pkill -x quickshell; sleep 0.5
if [ "${1:-}" = restore ]; then
    p=/usr/share/samhie/quickshell
else
    rm -rf "$dev"
    cp -a "$repo/files/system/usr/share/samhie/quickshell" "$dev"
    p=$dev
fi
cd /
SAMHIE_SHELL=/usr/share/samhie/quickshell \
SAMHIE_WALLPAPER=$(readlink -f /usr/share/backgrounds/samhie/samhie.png) \
SAMHIE_WALLPAPER_LIGHT=$(readlink -f /usr/share/backgrounds/samhie/samhie-light.png) \
setsid -f quickshell -n -p "$p" >"$HOME/.cache/samhie-qs-dev.log" 2>&1 </dev/null
sleep 2
pgrep -a -x quickshell
