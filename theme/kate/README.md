# Theme màu editor Kate — nền Breeze Dark

| | |
|---|---|
| File | `breeze-dark.theme` — chép nguyên, không sửa |
| Nguồn | KSyntaxHighlighting `github.com/KDE/syntax-highlighting` (= `invent.kde.org/frameworks/syntax-highlighting`), `data/themes/breeze-dark.theme` |
| Phiên bản | tag `v6.30.0`, commit `065ef8580096caea079fddc22d420c786cf53f92` — cùng bản KF6 của Fedora 44 |
| Giấy phép | **MIT** (ghi trong `metadata` của file: SPDX-License-Identifier) |
| sha256 | `da9758d3b8692b172dc6a735dbf64be9ddc5409bbbf3e9c6a8066b491afaddc2` |

Fedora không có file này trên đĩa: Breeze Dark được biên dịch vào thư viện
KSyntaxHighlighting (tài nguyên Qt), nên phải chép vào repo.

`tools/gen-theme.py` đọc file này, giữ nguyên màu cú pháp (`text-styles`,
`custom-styles`), thay các màu NỀN trong `editor-colors` bằng vai trò của
`theme/palette.toml` rồi ghi theme **Samhie** vào
`files/system/usr/share/org.kde.syntax-highlighting/themes/samhie.theme`.
`/etc/xdg/katerc` chọn theme đó.

Lên bản KF6 mới: chép lại file từ tag tương ứng, sửa bảng trên, chạy
`python3 tools/gen-theme.py` và xem diff.
