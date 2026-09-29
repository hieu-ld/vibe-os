#!/usr/bin/env bash
# Nén bộ con trỏ Samhie sang định dạng hyprcursor.
#
# tools/build-cursors.py sinh "working state" dạng chữ (SVG + meta.hl) ở
# /usr/share/samhie/cursors; ở đây hyprcursor-util (gói hyprcursor, cùng nguồn
# với Hyprland) nén nó thành /usr/share/icons/Samhie-cursors/manifest.hl +
# hyprcursors/*.hlc — nằm cạnh bản XCursor (cursors/, index.theme) đã commit.
# Nén lúc build để repo không phải giữ file nén mà CI không kiểm được.
set -oue pipefail

src=/usr/share/samhie/cursors
dest=/usr/share/icons/Samhie-cursors
out=$(mktemp -d)

hyprcursor-util --create "$src" --output "$out"
# hyprcursor-util ghi vào thư mục con theme_<tên trong manifest.hl>
cp -r "$out/theme_Samhie-cursors/manifest.hl" "$out/theme_Samhie-cursors/hyprcursors" "$dest/"
rm -rf "$out" "$src"

# Mỗi hình trong working state phải ra đúng một file .hlc
[[ $(ls "$dest/hyprcursors" | wc -l) -eq 16 ]]
