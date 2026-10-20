# Hình nền mặc định

| | |
|---|---|
| File | `samhie.webp` — logo "S" của Samhie trên nền sao, 2560×1600, WebP |
| Nguồn | user (Rogg/duchieu) đưa 2026-10-20 (`~/Downloads/wallpaper.png` — tên .png nhưng là WebP), giữ nguyên byte. **Nền sao do AI tạo** bằng Muse của Meta (https://muse.ai/); chữ "S" user tự vẽ trên Figma |
| Giấy phép | Điều khoản Muse (https://muse.ai/terms, đọc 2026-10-20): 2.1 người dùng giữ mọi quyền sở hữu trí tuệ với nội dung mình tạo; không hạn chế dùng thương mại / phát hành lại thành phẩm; 7.5 người dùng tự chịu trách nhiệm về thành phẩm; 4.1.12 không được nói thành phẩm AI là do người làm → **ghi rõ ở đây là ảnh có phần AI tạo**. Phát hành cùng Samhie |
| Bản sáng | `samhie-light.webp` — cùng logo trên nền sáng (xanh xám nhạt → be), 2560×1600, WebP; user đưa 2026-10-20, giữ nguyên byte. Hiện khi bật chế độ sáng |
| sha256 | `SHA256SUMS` (hai nguồn + ba file sinh ra) |

Sinh `files/system/usr/share/backgrounds/samhie/samhie.png`,
`samhie-blur.png` (màn đăng nhập + màn khoá, luôn tối) và `samhie-light.png`:
`python3 tools/build-wallpaper.py`.
Đổi ảnh: chép đè `samhie.webp` (hoặc đổi `SRC` trong script nếu khác định
dạng), chạy lại script, commit cả ba file + `SHA256SUMS`.

Tranh núi đêm vẽ bằng mã (2026-09-28 → 2026-10-20) còn trong lịch sử git
(`theme/wallpaper/samhie.svg`, `tools/build-wallpaper.py` bản cũ).
