# Icon nét — Hugeicons (bộ free)

| | |
|---|---|
| Bộ gốc | **Hugeicons** — https://hugeicons.com, repo `github.com/hugeicons/hugeicons` |
| Kiểu | Stroke Rounded (kiểu duy nhất của bản free), khung 24, nét 1,5 |
| Giấy phép | **MIT** — `LICENSE` cạnh file này, chép nguyên từ `LICENSE.md` của repo |
| Đối chiếu | commit `9c48f3723dfb243909fa83501aa7c6423ab972c0` (2026-09-23), thư mục `icons/` |
| sha256 | `SHA256SUMS` |

User tải từ trang web 2026-10-06 (lọc chỉ icon free), đặt tên theo chỗ dùng;
thêm `folder`, `context-menu`, `interface`, `general`, `content-display` và
mười icon cho trang Context Menu ngày 2026-10-07 (đối chiếu cùng commit,
khớp đủ mọi nét).
Mỗi file đã đối chiếu với thư mục `icons/` của repo ở commit trên: nét vẽ
(`d`, toạ độ) trùng khớp một icon free — nên không có icon Pro nào lọt vào
(icon Pro KHÔNG được đưa vào mã nguồn mở, xem README của repo đó).
Hai file đã sửa (MIT cho sửa):
- `trash-full`: `waste` thêm ba vạch dọc user tự vẽ.
- `volume-35`: user tải `volume` (thân loa đứng giữa khung, đuôi loa vẽ
  khác), Claude thay bằng đúng thân loa của `volume-full` (`volume-high`),
  bỏ hai vòng sóng — để kéo âm lượng loa đứng yên (user 2026-10-06).

`SHA256SUMS` là của file trong repo (đã gồm hai bản sửa).

Ảnh đóng vào image là bản phái sinh, nên `LICENSE` được chép thành
`files/system/usr/share/icons/Samhie/LICENSE.hugeicons`;
`tools/build-icon-theme.sh` từ chối chạy nếu hai file khác nhau.

## Thêm / đổi hình

1. Tải từ hugeicons.com: Stroke Rounded, nét 1,5, size 24 (màu gì cũng được —
   script đổi về màu chữ của theme). Lưu đè vào đây, tên viết thường.
2. Thêm tên vào `MAP` (và `SIZE` nếu không phải cỡ 16) trong
   `tools/build-icons.py`.
3. `python3 tools/build-icons.py --tune` → chép bảng in ra vào `NUDGE`.
4. `python3 tools/build-icons.py && ./tools/build-icon-theme.sh`, rồi
   `sha256sum *.svg > SHA256SUMS` ở đây. Commit cả file sinh ra (CI chạy `--check`).

## Tên file → tên trên Hugeicons

| File | Hugeicons |
|---|---|
| `ar-bottom` | `chevron-down` |
| `ar-left` | `chevron-left` |
| `ar-right` | `chevron-right` |
| `ar-top` | `chevron-up` |
| `backspace` | `arrow-big-left-dash` |
| `bell` | `bell` |
| `bell-off` | `bell-off` |
| `bluetooth` | `bluetooth-connected` |
| `bookmark` | `bookmark-02` |
| `bookmark-plus` | `bookmark-add-02` |
| `check` | `check` |
| `compress` | `folder-zip` |
| `close` | `circle-x` |
| `content-display` | `content-writing` |
| `context-menu` | `mouse-right-click-04` |
| `copy` | `copy-02` |
| `cut` | `square-bottom-dashed-scissors` |
| `dark-mode` | `moon-star` |
| `details` | `list` |
| `disk-image` | `software` |
| `down-2` | `arrow-down-02` |
| `download` | `square-arrow-down-03` |
| `duplicate` | `copy-01` |
| `edit` | `pencil-edit-02` |
| `extract` | `folder-archive` |
| `filter` | `filter` |
| `folder` | `folder-01` |
| `general` | `left-to-right-list-triangle` |
| `git` | `git-branch` |
| `hide` | `eye-closed` |
| `folder-new` | `folder-plus` |
| `icons` | `grid-view` |
| `interface` | `dashboard-browsing` |
| `lock` | `lock-keyhole` |
| `login` | `login-02` |
| `logout` | `logout-02` |
| `makefile-action` | `gears` |
| `max` | `circle-arrow-expand-01` |
| `menu` | `menu` |
| `min` | `circle-arrow-shrink-01` |
| `minus` | `circle-minus` |
| `network` | `house-wifi` |
| `new-windows` | `layer-add` |
| `next` | `circle-arrow-right-02` |
| `next-2` | `arrow-right-02` |
| `night-light` | `sun-moon` |
| `noti-off` | `bell-off` |
| `paste` | `file-paste` |
| `pin` | `pin` |
| `plus` | `circle-plus` |
| `power` | `power` |
| `prev` | `circle-arrow-left-02` |
| `prev-2` | `arrow-left-02` |
| `recent` | `history` |
| `refresh` | `refresh-ccw` |
| `rename` | `pencil-edit-01` |
| `restart` | `rotate-ccw` |
| `search` | `search-01` |
| `settings` | `settings-01` |
| `shut-down` | `shut-down` |
| `sleep` | `moon-02` |
| `sort` | `arrow-data-transfer-vertical` |
| `smart-home` | `home-02` |
| `split` | `square-split-horizontal` |
| `storage` | `hdd` |
| `terminal` | `square-terminal` |
| `trash-delete` | `delete-01` |
| `trash-empty` | `waste` |
| `trash-full` | `waste` (+ 3 vạch dọc user thêm) |
| `up-2` | `arrow-up-02` |
| `volume-35` | `volume-high` bỏ sóng (user tải `volume`) |
| `volume-70` | `volume-01` |
| `volume-full` | `volume-high` |
| `volume-mute` | `volume-mute-02` |
| `wifi` | `wifi-02` |
| `wifi-0` | `wifi-zero` |
| `wifi-1` | `wifi-low` |
| `wifi-2` | `wifi-high` |
| `wifi-lock` | `wifi-lock` |
| `wired` | `computer-ethernet` |
| `x` | `x` |
| `zoom-default` | `search-replace` |
| `zoom-in` | `zoom-in` |
| `zoom-out` | `zoom-out` |

Chỗ dùng của từng hình (tên icon freedesktop, cỡ): bảng `MAP` và `SIZE` trong
`tools/build-icons.py`.
