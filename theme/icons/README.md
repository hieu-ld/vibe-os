# Icon Samhie — nguồn

Ba bộ, ba script:

- **Icon nét một màu** (`hugeicons/`) — bộ free của [Hugeicons](https://hugeicons.com),
  user chọn và tải 2026-10-06 thay cho bộ tự vẽ trên Figma (nét 1px của bộ vẽ
  tay mỏng và lấm tấm ở đường cong — đo ở `docs/NOTES.md`). Sinh bằng
  `python3 tools/build-icons.py`; giấy phép, nguồn và cách thêm hình: xem
  `hugeicons/README.md`. Bộ vẽ tay cũ còn trong lịch sử git (commit `4805dac`).
- **Icon app nhiều màu** (`apps/`) — user vẽ, mục bên dưới.
- **Thư mục Dolphin tô màu** — vỏ user vẽ + ký hiệu Hugeicons, mục bên dưới.

## Icon app (`apps/`) — nhiều màu, KHÔNG qua build-icons.py

User vẽ trên Figma (khung 32, hình trong 2..30 — bản cũ khung 40, script nhận
cả hai). Bản 2026-10-20 (user vẽ lại cả ba, xuất `folder .svg`,
`terminal-dock.svg`, `settings .svg` trong ~/Downloads): tự có gradient ánh
sáng nên script KHÔNG thêm viền bắt sáng nữa; Dolphin + Konsole cao 26 (thấp
hơn bề ngang 2px — user cố ý). Lưu ở đây theo TÊN ICON rồi chạy
`python3 tools/build-app-icons.py` → `files/system/usr/share/icons/Samhie/apps/scalable/`.
Màu giữ nguyên như bản vẽ. Script đổi: khung `4 4 32 32` (icon 32px trong ô
dock 40, cách lề 2px), viền ngoài vẽ VÀO TRONG cho sắc 1px, thêm viền bắt
sáng kiểu bản 1.0 khi ICONS khai (bản 2026-10-20 không) — giải thích đủ ở đầu script. CI chạy `--check`.

| Hình | Tên icon |
|---|---|
| `apps/org.kde.dolphin.svg` (hover) | `org.kde.dolphin` |
| `apps/utilities-terminal.svg` (hover-2) | `utilities-terminal` (Konsole) |
| `apps/preferences-system.svg` (hover-1) | `preferences-system` — chưa app nào dùng, dock tạm ẩn |

## Thư mục Dolphin tô màu — KHÔNG qua build-icons.py

Vỏ thư mục user vẽ 2026-10-04 (khung 64, ghi thẳng trong
`tools/build-folder-icons.py`); ký hiệu ở giữa là Hugeicons
`hugeicons/symbol-<tên>.svg` (2026-10-08, thay bộ ký hiệu user tự vẽ — bản cũ
còn trong lịch sử git, thư mục `theme/icons/folders/`). Chạy
`python3 tools/build-folder-icons.py` → `Samhie/places/scalable/`: tô kín
nắp/thân, ký hiệu phóng 32/24 (nét 2), màu ở `theme/palette.toml` `[folders]`
(ký hiệu `important` màu riêng `[folder-symbols]`). Sinh `folder-<ký hiệu>`,
`folder-<ký hiệu>-<màu>` (đổi màu giữ ký hiệu — style "samhie" mục 47),
`folder-<màu>`, `user-home` (hình `smart-home`). Thêm ký hiệu: thêm file
`symbol-<tên>.svg` + tên vào `SYMBOLS` của script, `MAP` của build-icons.py và
`FOLDER_SYMBOLS` của `style/samhie-style.cpp`.
