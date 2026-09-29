# Con trỏ Samhie — bản vẽ gốc

User (Rogg) vẽ trên Figma, 2026-09-29, xuất SVG thẳng từ Figma. Đây là NGUỒN;
bộ con trỏ thật sinh bằng `python3 tools/build-cursors.py` (xem đầu script).

| File | Khung | Ghi chú |
|---|---|---|
| `default.svg` | 24 | mũi tên dựng đứng — script xoay 27,41° (user chọn) |
| `pointer.svg` | 24 | |
| `text.svg` | 16 | user vẽ hai bản, chọn bản chữ I đơn (bản có "A"/"z" ở 24px chỉ còn vài chấm) |
| `not-allowed.svg` | 23 | khung 16 xoay 45° |
| `grab.svg`, `grabbing.svg` | 16 | |
| `move.svg`, `crosshair.svg` | 16 | |
| `help.svg` | 16 | chỉ dấu "?" — script ghép vào góc dưới-phải mũi tên |
| `ew-resize.svg` | 16 | script xoay ra ns / nwse / nesw |

`wait` và `progress` không có file: vòng 5 chấm vẽ bằng mã, cùng chuyển động
với vòng xoay Plymouth.

**Sửa so với bản xuất Figma** (user nhờ, 2026-09-29):
- `grab.svg`: viền 0,75 → 1 như các hình khác, `stroke-linejoin="round"`.
- `grabbing.svg`: cạnh trên (đốt ngón) vẽ lại thành 3 cung bán kính 1,6 đều
  nhau — cùng góc trên-phải là 4 đốt, cộng ngón cái; bản gốc to nhỏ không đều.
- `help.svg`, `not-allowed.svg`: thêm viền trắng (nét trắng rộng 3 nằm DƯỚI
  nét đỏ rộng 1) cho giống hàng xóm.

**Màu** trong file là màu Figma; script đổi sang bảng màu: `#003845` →
`shared.cursor`, `#FAFAFA` → `shared.cursor-outline`, `#BD2123` → `dark.danger`.
Màu khác là script báo lỗi. Vẽ lại trên Figma cứ dùng đúng ba màu đó.

**Vẽ thêm / sửa hình:** xuất SVG đè vào đây rồi chạy lại script, commit cả file
sinh ra. Đổi `default.svg` thì phải đo lại hằng số xoay/dịch `ARROW` trong script.
