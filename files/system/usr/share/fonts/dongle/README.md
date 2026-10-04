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

OFL cho sửa; không có Reserved Font Name nên giữ tên Dongle. sha256 bản đã
sửa (trong repo):

```
3b40367de2661c7c94130b92380f97f33921a52ea18327d8dbe6a6bb85e6176f  Dongle-Light.ttf
746c03ae7c4f09d5a3683bbc385564f58f8abd7db62fa62957e9a4230fba0193  Dongle-Regular.ttf
591f49ed587da0eb4e23e3b66f2e31e0afca6ca698309098b1297a74cdd85068  Dongle-Bold.ttf
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
