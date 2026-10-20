# Hình nền mặc định

| | |
|---|---|
| File | `samhie.webp` — logo "S" của Samhie trên nền sao, 2560×1600, WebP |
| Nguồn | user (Rogg/duchieu) đưa 2026-10-20 (`~/Downloads/wallpaper.png` — tên .png nhưng là WebP), giữ nguyên byte |
| Giấy phép | **CHỜ user xác nhận** tự làm / làm bằng công cụ nào — phải rõ trước khi repo công khai (bài học tranh núi 4kwallpapers, docs/PLAN.md mục Wallpaper) |
| sha256 | `SHA256SUMS` (nguồn + hai file sinh ra) |

Sinh `files/system/usr/share/backgrounds/samhie/samhie.png` và
`samhie-blur.png` (màn đăng nhập + màn khoá): `python3 tools/build-wallpaper.py`.
Đổi ảnh: chép đè `samhie.webp` (hoặc đổi `SRC` trong script nếu khác định
dạng), chạy lại script, commit cả ba file + `SHA256SUMS`.

Tranh núi đêm vẽ bằng mã (2026-09-28 → 2026-10-20) còn trong lịch sử git
(`theme/wallpaper/samhie.svg`, `tools/build-wallpaper.py` bản cũ).
