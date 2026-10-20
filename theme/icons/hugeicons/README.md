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
mười lăm icon cho trang Context Menu ngày 2026-10-07 (đối chiếu cùng commit,
khớp đủ mọi nét).
Hai mươi sáu icon cho hộp thoại Configure Keyboard Shortcuts (2026-10-07,
user nhờ tìm): Claude lấy thẳng từ thư mục `icons/` của repo ở commit trên,
chỉ đổi màu `#141B34` của repo thành `currentColor` (như bản tải từ web).
`browse` (nút Browse… của hộp chọn icon) user tải 2026-10-07, đối chiếu cùng
commit: trùng nét `folder-search`.
Mười bốn `symbol-*` (ký hiệu thư mục + menu Symbol, 2026-10-08): user tải
mười ba, `symbol-tar` Claude lấy `file-zip` từ repo (đổi `#141B34` →
`currentColor`); đối chiếu cùng commit, trùng nét. Vài hình trùng file khác
(`symbol-download` = `download`, `symbol-print` = `print`) — để riêng vì cỡ
hiển thị và bảng NUDGE khác nhau.
`open-with` (dòng "Other Application…" của menu Open With, 2026-10-08,
user nhờ tìm): Claude lấy `dashboard-square-add` từ repo, đổi `#141B34` →
`currentColor`.
`sidebar-close` (nút « thu khung Projects của Kate, 2026-10-12, user nhờ
chọn): Claude lấy `sidebar-left-01` từ repo ở commit trên, đổi `#141B34` →
`currentColor`.
`txt` (file .txt) và `star` (sao Rating) user tải 2026-10-08 cho hộp
Properties, đối chiếu cùng commit: trùng nét `txt-02` và `star`.
Ba mươi icon LOẠI FILE (2026-10-09, user chọn kiểu "nhãn + nhóm" từ bảng
Claude đề xuất): Claude lấy thẳng từ repo ở commit trên, đổi `#141B34` →
`currentColor`. Có nhãn: `pdf` `doc` `xls` `ppt` `csv` `zip` `rar` `jpg`
`png` `svg` `gif` `wav` `xml` = `<tên>-02`; `html` = `html-file-02`, `css` =
`css-file-02`. Theo nhóm (giữ tên gốc): `file-text` `file-code`
`file-terminal` `file-braces` `file-cog` `file-image` `file-video`
`file-music` `file-zip` `file-box` `file-database` `file-type` `file-key`
`file-digit` `file-question-mark`; `files` = `files-01` (chọn nhiều file).
Mỗi file đã đối chiếu với thư mục `icons/` của repo ở commit trên: nét vẽ
(`d`, toạ độ) trùng khớp một icon free — nên không có icon Pro nào lọt vào
(icon Pro KHÔNG được đưa vào mã nguồn mở, xem README của repo đó).
Konsole (2026-10-09, user nhờ tìm): hai mươi mốt icon cho hộp Configure
Konsole, Configure Keyboard Shortcuts, Configure Notifications và sáu icon
cho hộp Edit Profile (`add`, `keyboard`, `konsole-scrolling`,
`konsole-mouse`, `konsole-advanced`, `text-font`) — Claude lấy thẳng từ repo
ở commit trên, đổi `#141B34` → `currentColor`. `konsole-mouse` trùng nét
`mouse` (cỡ 32 thay 16, bảng NUDGE khác).
Kate (rà Kate đợt 2, 2026-10-12, user: "mấy cái dùng lại thì dùng lại, mấy
cái chưa có thì tìm giúp"): icon cho hộp Configure Kate (16px), cột icon trái
(32px), nút đáy cửa sổ và nút Replace của panel Search (16px). Dùng lại thẳng
hình đã có, cùng cỡ: Color Themes = `symbol-design`, Editing = `edit`,
Documents = `folder`, Plugins = `plugins`, LSP Client = `symbol-development`.
Bản chép vì khác cỡ/kiểu với hình gốc: `kate-session` = `history` (như
`recent`, cỡ 20), `kate-projects` = `files-01` (như `files`, nhưng không thu
nhỏ/nét mảnh như icon loại file), `kate-git` = `git-branch` (như `git`).
Hình mới: `save` (`floppy-disk`), `kate-behavior` (`cursor-in-window`),
`kate-feedback` (`comment-01`), `kate-outline` (`workflow-square-03`),
`kate-diagnostics` (`alert-02`), `kate-output` (`note-01`), `replace`,
`replace-all`. Claude lấy thẳng từ repo ở commit trên, đổi `#141B34` →
`currentColor`.
Panel Search của Kate gọn như VS Code (2026-10-13, user nhờ sắp xếp):
`match-case` (`case-sensitive`), `regex` (`regex`), `search-details`
(`more-horizontal`) — Claude lấy thẳng từ repo ở commit trên, đổi `#141B34`
→ `currentColor`. Nút ›/⌄ mở ô Replace dùng lại `ar-right`/`ar-bottom`.
Ô tích + nút tròn của theme Kvantum (2026-10-20, user chọn): `kvantum-checkbox`
(`square`), `kvantum-checkbox-checked` (`checkmark-square-02`),
`kvantum-radio` (`circle`), `kvantum-radio-checked` (`radio-button`); thêm
`kvantum-checkbox-tristate` (`minus-sign-square`, Claude chọn cho ô "tích một
nửa" cùng kiểu). Claude lấy thẳng từ repo ở commit trên, đổi `#141B34` →
`currentColor`. KHÔNG qua build-icons.py: `tools/kvantum.py` đọc nét từ các
file này, tô màu theo palette (xem chú thích ở đó).
Ba file đã sửa (MIT cho sửa):
- `star-full`: `star` tô kín (fill `currentColor`) — sao Rating đã chọn.
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
| `add` | `add-01` |
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
| `browse` | `folder-search` |
| `bug` | `bug-01` |
| `cancel` | `cancel-circle` |
| `check` | `check` |
| `clear-history` | `eraser` |
| `close` | `circle-x` |
| `close-tab` | `cancel-square` |
| `compare` | `file-diff` |
| `compress` | `folder-zip` |
| `content-display` | `content-writing` |
| `context-menu` | `mouse-right-click-04` |
| `copy` | `copy-02` |
| `copy-link` | `copy-link` |
| `cut` | `square-bottom-dashed-scissors` |
| `dark-mode` | `moon-star` |
| `detach` | `square-arrow-up-right` |
| `details` | `list` |
| `disk-image` | `software` |
| `disk-usage` | `pie-chart` |
| `donate` | `favourite` |
| `down-2` | `arrow-down-02` |
| `download` | `square-arrow-down-03` |
| `download-service` | `download-04` |
| `duplicate` | `copy-01` |
| `edit` | `pencil-edit-02` |
| `encoding` | `language-square` |
| `exchange` | `arrow-data-transfer-horizontal` |
| `extract` | `folder-archive` |
| `file-new` | `file-add` |
| `filter` | `filter` |
| `folder` | `folder-01` |
| `folder-edit` | `folder-edit` |
| `folder-export` | `folder-export` |
| `folder-new` | `folder-plus` |
| `fullscreen` | `full-screen` |
| `general` | `left-to-right-list-triangle` |
| `git` | `git-branch` |
| `handbook` | `book-open-01` |
| `help` | `help-circle` |
| `hide` | `eye-closed` |
| `icons` | `grid-view` |
| `info` | `information-circle` |
| `interface` | `dashboard-browsing` |
| `invert-selection` | `contrast` |
| `kate-behavior` | `cursor-in-window` |
| `kate-diagnostics` | `alert-02` |
| `kate-feedback` | `comment-01` |
| `kate-git` | `git-branch` |
| `kate-outline` | `workflow-square-03` |
| `kate-output` | `note-01` |
| `kate-projects` | `files-01` |
| `kate-session` | `history` |
| `keyboard` | `keyboard` |
| `konsole-advanced` | `sliders-horizontal` |
| `konsole-general` | `computer-terminal-01` |
| `konsole-mouse` | `mouse-01` |
| `konsole-profiles` | `paint-board` |
| `konsole-scrolling` | `scroll-vertical` |
| `lock` | `lock-keyhole` |
| `login` | `login-02` |
| `logout` | `logout-02` |
| `makefile-action` | `gears` |
| `match-case` | `case-sensitive` |
| `max` | `circle-arrow-expand-01` |
| `memory` | `ram-memory` |
| `menu` | `menu` |
| `menubar` | `layout-top` |
| `min` | `circle-arrow-shrink-01` |
| `minus` | `circle-minus` |
| `monitor` | `view` |
| `mouse` | `mouse-01` |
| `network` | `house-wifi` |
| `new-windows` | `layer-add` |
| `next` | `circle-arrow-right-02` |
| `next-2` | `arrow-right-02` |
| `night-light` | `sun-moon` |
| `noti-off` | `bell-off` |
| `open-folder` | `folder-open` |
| `open-with` | `dashboard-square-add` |
| `paste` | `file-paste` |
| `pin` | `pin` |
| `place-folder` | `folder-01` (bản chép của `folder`, căn lưới cỡ 20) |
| `play` | `play-circle` |
| `play-square` | `play-square` |
| `plugins` | `puzzle` |
| `plus` | `circle-plus` |
| `power` | `power` |
| `prev` | `circle-arrow-left-02` |
| `prev-2` | `arrow-left-02` |
| `preview` | `image-01` |
| `print` | `printer` |
| `properties` | `information-square` |
| `quick-commands` | `command` |
| `quit` | `logout-04` |
| `recent` | `history` |
| `redo` | `redo-03` |
| `refresh` | `refresh-ccw` |
| `regex` | `regex` |
| `rename` | `pencil-edit-01` |
| `replace` | `replace` |
| `replace-all` | `replace-all` |
| `restart` | `rotate-ccw` |
| `restore` | `undo` |
| `save` | `floppy-disk` |
| `search` | `search-01` |
| `search-details` | `more-horizontal` |
| `select-all` | `tick-double-02` |
| `select-text` | `cursor-text` |
| `settings` | `settings-01` |
| `show-hidden` | `view` |
| `shut-down` | `shut-down` |
| `sidebar` | `view-sidebar-left` |
| `sidebar-close` | `sidebar-left-01` |
| `sleep` | `moon-02` |
| `smart-home` | `home-02` |
| `sort` | `arrow-data-transfer-vertical` |
| `split` | `square-split-horizontal` |
| `split-auto` | `layout-grid` |
| `split-top-bottom` | `layout-2-row` |
| `star` | `star` |
| `star-full` | `star` (tô kín) |
| `stop` | `stop-circle` |
| `storage` | `hdd` |
| `swap` | `arrow-data-transfer-horizontal` |
| `symbol-cloud` | `cloudy` |
| `symbol-design` | `palette` |
| `symbol-development` | `code-xml` |
| `symbol-documents` | `doc-01` |
| `symbol-download` | `square-arrow-down-03` |
| `symbol-important` | `alert-circle` |
| `symbol-library` | `libraries` |
| `symbol-mail` | `mailbox-01` |
| `symbol-music` | `music-02` |
| `symbol-pictures` | `image-02` |
| `symbol-print` | `printer` |
| `symbol-tar` | `file-zip` |
| `symbol-temp` | `file-clock` |
| `symbol-videos` | `film-01` |
| `tab-bar` | `browser` |
| `temp-files` | `folder-clock` |
| `terminal` | `square-terminal` |
| `terminal-panel` | `command-line` |
| `text-font` | `text-font` |
| `thumbnails` | `image-01` |
| `trash-delete` | `delete-01` |
| `trash-empty` | `waste` |
| `trash-full` | `waste` (+ 3 vạch dọc user thêm) |
| `txt` | `txt-02` |
| `undo` | `undo-03` |
| `unlock` | `lock-keyhole-open` |
| `up-2` | `arrow-up-02` |
| `volume-35` | `volume-high` bỏ sóng (user tải `volume`) |
| `volume-70` | `volume-01` |
| `volume-full` | `volume-high` |
| `volume-mute` | `volume-mute-02` |
| `web-search` | `global-search` |
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
