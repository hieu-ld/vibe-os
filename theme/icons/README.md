# Icon Samhie — bản vẽ gốc

User (Rogg) vẽ trên Figma (khung "Samhie icons"), 2026-09-30, xuất SVG thẳng
từ Figma, tên file = tên layer viết thường. Đây là NGUỒN; file thật trong bộ
icon (`files/system/usr/share/icons/Samhie/`) sinh bằng
`python3 tools/build-icons.py` — bảng ghép tên (`MAP`) nằm đầu script đó.

Khung 20 (nét 1,25) hoặc 16 (nét 1) — cùng tỉ lệ nét/khung nên co giãn về
cùng một cỡ là nét dày bằng nhau.

| Hình | Tên icon |
|---|---|
| `power` `restart` `moon` | `system-shutdown` `system-reboot` `system-suspend` |
| `next-2` `prev-2` `up-2` `down-2` | `go-next` `go-previous` `go-up` `go-down` |
| `ar-right` `ar-left` `ar-top` `ar-bottom` | `arrow-right` `arrow-left` `arrow-up` `arrow-down` |
| `close` `minus` `max` `min` | `window-close/minimize/maximize/restore-symbolic` |
| `plus` | `tab-new` (nút `+` và "New Tab" của Konsole) |
| `smart-home` | `user-home`, `go-home` |
| `download` `network` `hdd` `recent` | `folder-download` `folder-network` `drive-harddisk` `document-open-recent` |
| `trash` `trash-full` | `user-trash` `user-trash-full` |
| `bell` `bell-ring` `bell-off` | `notification-inactive` `notification-active` `notifications-disabled` |
| `volume` `volume-mute` | `audio-volume-high/medium/low` `audio-volume-muted` |

**Vẽ sẵn, chưa lắp** (`UNUSED` trong script): `folder`, `document`, `desktop`,
`next`, `prev`, `up`, `down` (bốn ô vuông có mũi tên). `folder` chưa lắp vì bộ
Samhie là SVG co giãn — một file `folder` thay cho MỌI cỡ, kể cả thư mục to có
màu trong Dolphin.

**Sửa so với bản xuất Figma:**
- `plus.svg` (2026-10-01): Konsole vẽ nút `+` ở đúng 16px. Bản gốc có vạch
  dấu cộng ở toạ độ 8,0 — nằm trên ranh giới hai điểm ảnh, nên nét 1px thành
  2 hàng mờ (user thấy nhoè trong máy ảo). Dời cả hình về tâm 7,5 (vạch rơi
  đúng giữa điểm ảnh) và nới vòng tròn ra 15px cho to hơn (user thấy nhỏ):
  vòng `x=0.5 w=14`, dấu cộng dài 6 từ 4,5 tới 10,5.

**Màu:** vẽ bằng `#A1A1A1` (nét) trên nền `#0A0A0A`. Script đổi thành
`currentColor` để KDE/SDDM tô theo màu chữ của bảng màu — nên trên máy icon ra
**màu chữ** (sáng hơn xám Figma), giống icon Tabler trong bộ. Chỗ tô mờ thì vẽ
bằng một màu nằm giữa hai màu đó (vd `#3B3C3C` ở lòng `trash-full`) — script
tính ngược ra độ mờ (≈ 0,32) và thêm vào bảng `FIGMA`. Màu lạ là script báo lỗi.

**Vẽ thêm / sửa hình:** xuất SVG đè vào đây (tên viết thường), thêm tên vào
`MAP` hoặc `UNUSED` trong script, chạy `python3 tools/build-icons.py` rồi
`./tools/build-icon-theme.sh`, commit cả file sinh ra. CI chạy `--check`.

## Icon app (`apps/`) — nhiều màu, KHÔNG qua build-icons.py

User vẽ trên Figma (khung 40, hình trong 6..34, xuất tên `hover*.svg`), bản
mới 2026-10-01 tối. Lưu ở đây theo TÊN ICON rồi chạy
`python3 tools/build-app-icons.py` → `files/system/usr/share/icons/Samhie/apps/scalable/`.
Màu giữ nguyên như bản vẽ. Script đổi: khung `4 4 32 32` (icon 32px trong ô
dock 40, cách lề 2px), viền ngoài vẽ VÀO TRONG cho sắc 1px, thêm viền bắt
sáng kiểu bản 1.0 — giải thích đủ ở đầu script. CI chạy `--check`.

| Hình | Tên icon |
|---|---|
| `apps/org.kde.dolphin.svg` (hover-1) | `org.kde.dolphin` |
| `apps/utilities-terminal.svg` (hover-2) | `utilities-terminal` (Konsole) |
| `apps/preferences-system.svg` (hover) | `preferences-system` — chưa app nào dùng, dock tạm ẩn |
