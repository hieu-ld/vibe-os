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
máy ảo). Chỉ đổi 4 con số đó, không đụng glyph. OFL cho sửa; không có
Reserved Font Name nên giữ tên Dongle. sha256 bản đã sửa (trong repo):

```
ae43506b7a284e9b383077033926e2360f3a16a18a36111c4a52ff56a9486d0d  Dongle-Light.ttf
e522f787646f9cf44fa0287b77316c6ce8a9ba77928f8667e024dbb44d63c83b  Dongle-Regular.ttf
e4fe0bb36164bf52f90e73164600a9b3342963e9b90da06848907661b2f62831  Dongle-Bold.ttf
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
