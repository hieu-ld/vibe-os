# Dongle — font giao diện của Samhie

Fedora không đóng gói Dongle nên Samhie mang file font theo image.

| | |
|---|---|
| Tác giả | Yanghee Ryu — https://github.com/yangheeryu/Dongle |
| Giấy phép | **SIL Open Font License 1.1** — `OFL.txt` cạnh file này, PHẢI đi theo image |
| Lấy từ | https://github.com/google/fonts/tree/main/ofl/dongle (google/fonts, tới commit `7380f7c`) |
| Bản | 2.000 (upstream commit `f7127c4`) |

OFL cho đóng kèm vào phần mềm (điều 2) miễn kèm giấy phép + dòng bản quyền;
chỉ cấm bán riêng file font. Không có Reserved Font Name.

**Đã sửa số đo dọc** (2026-10-01, `tools/fix-dongle-metrics.py` — đọc đầu
script): ascent/descent (hhea + usWin) 850/598 → 922/526, tổng chiều cao dòng
giữ nguyên. Bản gốc làm chữ nằm lệch lên ~2,5px so với icon cạnh nó (đo trong
máy ảo). Chỉ đổi 4 con số đó, không đụng glyph.

**Thêm 5 ký tự vô hình rộng 0** (2026-10-04, cùng script): U+200B, U+200C,
U+200D, U+2060, U+FEFF — glyph rỗng, không nét vẽ. Dongle gốc thiếu, Qt phải
mượn font khác → tên file có `+ _ .`… trong Dolphin tụt xuống 12px (KDE chèn
U+200B sau các dấu đó để tên dài xuống dòng được). Glyph cũ không đổi.

**Thêm 4 khoảng trắng hẹp** (2026-10-05, cùng script): U+202F (rộng 100),
U+2009 (100), U+200A (50), U+2008 (= dấu chấm) — glyph rỗng. Giờ kiểu Mỹ có
U+202F trước AM/PM → cột Modified của Dolphin ("Today at 10:58 AM") tụt
xuống ~13px vì Qt mượn font khác cho ký tự đó.

OFL cho sửa; không có Reserved Font Name nên giữ tên Dongle. sha256 bản đã
sửa (trong repo):

```
ac334d924e5351228bcdc4f4aad18df12c0ab6b062f9b624ba8a85e8f632d756  Dongle-Light.ttf
a1268087c4eed215bdf508486cf3ff71e23d48e2865f3adb73c63ee760b56cd8  Dongle-Regular.ttf
0cf8dcd69612b1529b1012c7bae3922fe2c373adacf5eff390b685ad1103a76f  Dongle-Bold.ttf
```

Đã kiểm (2026-09-28): cả ba file đủ 203/203 ký tự tiếng Việt (mọi chữ có dấu,
hoa + thường) bằng `fc-query`. sha256 bản GỐC tải từ google/fonts:

```
bb04e03dbe44fb0c619434f31565e62b5bac67229d5ed72eeac69c2e74e9a45c  Dongle-Light.ttf
2966e7a9d312a868e2b5477bf9a3e59b8ce16beb76e7e52305ab5897d1ce9741  Dongle-Regular.ttf
944c498c0d1a1832ab36f173b1b3aa5ae77b2a914e00c4d79e05338fae36472d  Dongle-Bold.ttf
```

Đủ ba độ đậm để chữ đậm (tiêu đề, mục đang chọn) vẫn là Dongle thật. Mức 350
KHÔNG dựng được: 8.435/12.554 glyph giữa master Light và Regular không khớp
điểm, gồm cả `Ư`, dấu hỏi, dấu trăng (thử `fontmake` 3.10.1 trên
`sources/Dongle.glyphs`, 2026-09-28).
