# Icon Samhie — nguồn

Ba bộ, ba script:

- **Icon nét một màu** (`hugeicons/`) — bộ free của [Hugeicons](https://hugeicons.com),
  user chọn và tải 2026-10-06 thay cho bộ tự vẽ trên Figma (nét 1px của bộ vẽ
  tay mỏng và lấm tấm ở đường cong — đo ở `docs/PLAN.md`). Sinh bằng
  `python3 tools/build-icons.py`; giấy phép, nguồn và cách thêm hình: xem
  `hugeicons/README.md`. Bộ vẽ tay cũ còn trong lịch sử git (commit `4805dac`).
- **Icon app nhiều màu** (`apps/`) — user vẽ, mục bên dưới.
- **Thư mục Dolphin tô màu** (`folders/`) — user vẽ, mục bên dưới.

## Icon app (`apps/`) — nhiều màu, KHÔNG qua build-icons.py

User vẽ trên Figma (khung 32, hình trong 2..30 — bản cũ khung 40, script nhận
cả hai; xuất tên `hover*.svg`). Bản 2026-10-05: viền Dolphin đậm hơn
(`#006175`), thân Konsole sáng hơn (`#3B3C3C`); bánh răng chưa đổi. Lưu ở đây theo TÊN ICON rồi chạy
`python3 tools/build-app-icons.py` → `files/system/usr/share/icons/Samhie/apps/scalable/`.
Màu giữ nguyên như bản vẽ. Script đổi: khung `4 4 32 32` (icon 32px trong ô
dock 40, cách lề 2px), viền ngoài vẽ VÀO TRONG cho sắc 1px, thêm viền bắt
sáng kiểu bản 1.0 — giải thích đủ ở đầu script. CI chạy `--check`.

| Hình | Tên icon |
|---|---|
| `apps/org.kde.dolphin.svg` (hover) | `org.kde.dolphin` |
| `apps/utilities-terminal.svg` (hover-2) | `utilities-terminal` (Konsole) |
| `apps/preferences-system.svg` (hover-1) | `preferences-system` — chưa app nào dùng, dock tạm ẩn |

## Bộ thư mục Dolphin (`folders/`) — tô màu, KHÔNG qua build-icons.py

User vẽ 2026-10-04 (khung 64, chỉ viền `#A1A1A1`: nắp + thân thư mục giống
nhau ở mọi hình, ký hiệu ở giữa). Lưu ở đây theo tên icon bỏ `folder-`, rồi
chạy `python3 tools/build-folder-icons.py` → `Samhie/places/scalable/`. Script
tô kín nắp/thân, ký hiệu một màu tối hơn nắp; màu ở `theme/palette.toml` `[folders]`.
Ký hiệu user vẽ trong ô 24×24 (tâm 32, 37); script phóng lên 32 (nét 1,5 → 2,
user 2026-10-05) — vẽ ký hiệu mới thì vẫn vẽ trong ô 24 như cũ. `important`
tô màu riêng (`[folder-symbols]` trong palette) thay vì màu nắp.
**Bản vẽ khung 16** (`folders/16/<tên>.svg`, chỉ ký hiệu, nét 1): menu Other
dùng nguyên hình đó ở 16px thay vì thu nhỏ hình 64. Hiện có `important` (user
vẽ lại 2026-10-05); hình 64 của nó là cùng hình đó ×1,5 đặt vào ô 24 (tâm 32,
37, nét 1,5) — vẽ lại thì sửa cả hai. Chỉnh so với bản user: chấm + thân dấu
than là ô vuông bo 0,5 (bản gốc bo tròn hẳn → mờ ở 16px).
Thư mục trơn (`folder`, 9 màu) sinh từ vỏ thư mục, không cần file riêng.

**Sửa so với bản xuất Figma:** `details.svg` (2026-10-05) — ba chấm tròn
r 0,5 thành ô vuông 1px: chấm tròn chỉ phủ ~78% điểm ảnh, mờ hơn các vạch.
`music.svg` (2026-10-04) — bản xuất mất hai
nét chân nốt nhạc (có trong ảnh Figma user chụp). Thêm lại hai nét dọc dày
1,5 ở x = 25,25 (40,5 → 46) và 31,25 (39,5 → 45,25), đo trên ảnh Figma: nối
cạnh thanh ngang xuống tới đầu nốt.

| Hình | Bản xuất của user | Tên icon |
|---|---|---|
| `documents` `download` `music` `pictures` `videos` | `folder-1` `-0` `-2` `-3` `-4` | `folder-<tên>` (thư mục XDG) |
| `bookmark` `cloud` `development` `games` `mail` | `folder-13` `-5` `-12` `-11` `-10` | `folder-<tên>` (menu màu → Other) |
| `print` `tar` `temp` `important` | `folder-9` `-8` `-6` `-7` | `folder-<tên>` (`tar` = nén, `temp` = tạm) |
| `home` | `folder-14` | `user-home` (thư mục home, xem dạng to) |
