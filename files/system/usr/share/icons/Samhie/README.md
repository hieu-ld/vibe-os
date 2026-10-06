# Bộ icon Samhie

Chỉ chứa những icon Samhie thay riêng. `index.theme` khai
`Inherits=breeze-dark,breeze,hicolor`, nên mọi tên icon không có ở đây KDE tự
lấy của Breeze — thay được bao nhiêu dùng bấy nhiêu.

## Nguồn

Ba nguồn:
- **Hugeicons** (bộ free, MIT — `LICENSE.hugeicons`) — mọi icon nét một màu.
  Nguồn ở `theme/icons/hugeicons/`, sinh bằng `python3 tools/build-icons.py`.
  File sinh ra có dòng chú thích "Sinh bằng tools/build-icons.py" ở đầu; đổi
  hình thì đổi ở `theme/icons/hugeicons/`.
- **Icon user tự vẽ** (Figma) — icon app nhiều màu (`apps/scalable`) và thư
  mục tô màu (`places/`), xem `theme/icons/README.md`.
- **Tabler Icons** — còn đúng một icon cũ (`battery-profile-performance`):

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

Từ 2026-10-06 là Hugeicons `wifi-zero`/`wifi-low`/`wifi-high`/`wifi-02`
(chấm + 0–3 vòng, vòng chưa đạt mức thì không vẽ). Bốn hình cho năm tên KDE:
`ok` và `good` cùng hình 2 vòng. Trang Wi-Fi của quick settings chỉ dùng bốn
mức (`WifiPage.qml`).
