# Bộ icon Samhie

Chỉ chứa những icon Samhie thay riêng. `index.theme` khai
`Inherits=breeze-dark,breeze,hicolor`, nên mọi tên icon không có ở đây KDE tự
lấy của Breeze — thay được bao nhiêu dùng bấy nhiêu.

## Nguồn

Hai nguồn:
- **Icon user tự vẽ** (Figma) — nguồn ở `theme/icons/`, sinh bằng
  `python3 tools/build-icons.py`. File sinh ra có dòng chú thích
  "Sinh bằng tools/build-icons.py" ở đầu; sửa hình thì sửa ở `theme/icons/`.
- **Tabler Icons** — những icon còn lại:

| | |
|---|---|
| Bộ gốc | **Tabler Icons** — https://tabler.io/icons |
| Phiên bản | **v3.46.0** |
| Giấy phép | **MIT** — xem `LICENSE` cạnh file này |

`LICENSE` **phải đi theo** vào image: đóng icon vào image công khai rồi đưa người khác tải
chính là phát hành lại, mà MIT bắt giữ thông báo bản quyền.
`tools/build-icon-theme.sh` từ chối chạy nếu thiếu file đó.

## File ở đây KHÔNG phải bản tải thẳng từ tabler.io

Đã qua `tools/import-tabler-icon.sh`. Bản gốc của Tabler khai
`stroke="currentColor"` mà không định nghĩa `color` ở đâu — trên web thì lấy màu
chữ của thẻ cha, còn trong KDE thì rơi về **đen**, tức icon đen trên dock đen và
không có lỗi nào được in ra.

Script chép theo đúng cách Breeze làm: thêm khối
`<style id="current-color-scheme">` và đặt `class="ColorScheme-Text"` lên từng
phần tử vẽ. Khối đó là cái móc để KDE thay màu theo bảng màu đang dùng; giá trị
mặc định để `#fcfcfc` (trùng breeze-dark) làm đường lui.

Muốn thêm icon mới thì **chạy script, đừng chép tay**:

```bash
./tools/import-tabler-icon.sh <file.svg> <nhóm> <tên-freedesktop>
./tools/build-icon-theme.sh
```

### Mức sóng Wi-Fi (`status/network-wireless-signal-*`)

`network-wireless-signal-excellent` là Tabler `wifi` (v3.46.0) qua script trên.
Bốn mức còn lại **suy ra từ file đó**: vòng ngoài không đạt mức thì thêm
`opacity="0.3"` (như Windows 11) — `good` = đủ 3 vòng, `ok` mờ vòng ngoài
cùng, `weak` mờ 2 vòng, `none` chỉ còn chấm. Dùng ở trang Wi-Fi của quick
settings; app khác (khay hệ thống) cũng tìm đúng các tên này.
