#!/usr/bin/env python3
"""
Hình nền mặc định của Samhie: ảnh user đưa (2026-10-20, logo "S" trên nền
sao), thay tranh núi đêm vẽ bằng mã (2026-09-28 — script cũ còn trong lịch sử
git, commit trước 2026-10-20). Nguồn + giấy phép: theme/wallpaper/README.md.

    python3 tools/build-wallpaper.py            # sinh PNG + bản mờ vào image
    python3 tools/build-wallpaper.py --check    # file sinh ra còn khớp nguồn? (CI)

NGUỒN là theme/wallpaper/samhie.webp — giữ nguyên byte user đưa (file tên
.png nhưng thật là WebP). Sinh ra, vào image ở /usr/share/backgrounds/samhie/:
  * samhie.png — cùng cỡ ảnh gốc, PNG vì swaybg (đang vẽ hình nền) đọc chắc
    PNG/JPG; WebP thì tuỳ gdk-pixbuf có bộ đọc không. `swaybg -m fill` tự cắt
    vừa màn hình (ảnh 16:10, màn 16:9 mất một ít trên/dưới).
  * samhie-blur.png — bản LÀM MỜ SẴN cho màn hình đăng nhập (SDDM) và màn hình
    khoá: cắt giữa về 16:9 rồi làm mờ lúc dựng thay vì bằng shader lúc chạy,
    máy không có tăng tốc đồ hoạ vẫn hiện đúng.
  * samhie-light.png — bản SÁNG (user đưa 2026-10-20, theme/wallpaper/
    samhie-light.webp), hiện khi bật chế độ sáng (samhie.lua). Không có bản
    mờ: màn đăng nhập + màn khoá luôn tối.

--check chỉ dùng thư viện chuẩn: so sha256 của nguồn và hai file sinh ra với
theme/wallpaper/SHA256SUMS (script ghi lúc sinh). Đổi ảnh nguồn mà quên chạy
lại → sha256 nguồn lệch → CI dừng. Sinh cần Pillow (có WebP).
"""

import hashlib
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "theme/wallpaper/samhie.webp"
SRC_LIGHT = ROOT / "theme/wallpaper/samhie-light.webp"
SUMS = ROOT / "theme/wallpaper/SHA256SUMS"
PNG_PATH = ROOT / "files/system/usr/share/backgrounds/samhie/samhie.png"
BLUR_PATH = PNG_PATH.with_name("samhie-blur.png")
LIGHT_PATH = PNG_PATH.with_name("samhie-light.png")
FILES = (SRC, PNG_PATH, BLUR_PATH, SRC_LIGHT, LIGHT_PATH)
# Bản mờ nhỏ 960x540 rồi để SDDM phóng lên: đã mờ thì độ phân giải không còn
# thấy được, file nhỏ. Bán kính 16 ở cỡ này ≈ 64px ở 4K — mạnh cỡ hyprlock cũ
# (blur_passes 4, blur_size 8): còn thấy dáng hình, hết chi tiết.
BLUR_W, BLUR_H, BLUR_RADIUS = 960, 540, 16


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def sums_text():
    return "".join(f"{sha256(p)}  {p.relative_to(ROOT)}\n" for p in FILES)


def build():
    from PIL import Image, ImageFilter
    with Image.open(SRC) as im:
        im = im.convert("RGB")
        PNG_PATH.parent.mkdir(parents=True, exist_ok=True)
        im.save(PNG_PATH, optimize=True)
        print(f"đã sinh {PNG_PATH.relative_to(ROOT)} ({im.width}x{im.height})", file=sys.stderr)
        # Cắt giữa về đúng tỉ lệ bản mờ (như `fill`), rồi thu nhỏ + làm mờ.
        w = min(im.width, round(im.height * BLUR_W / BLUR_H))
        h = round(w * BLUR_H / BLUR_W)
        box = ((im.width - w) // 2, (im.height - h) // 2, (im.width + w) // 2, (im.height + h) // 2)
        small = im.crop(box).resize((BLUR_W, BLUR_H), Image.LANCZOS)
        small.filter(ImageFilter.GaussianBlur(BLUR_RADIUS)).save(BLUR_PATH, optimize=True)
        print(f"đã sinh {BLUR_PATH.relative_to(ROOT)} ({BLUR_W}x{BLUR_H}, mờ {BLUR_RADIUS})", file=sys.stderr)
    with Image.open(SRC_LIGHT) as im:
        im.convert("RGB").save(LIGHT_PATH, optimize=True)
        print(f"đã sinh {LIGHT_PATH.relative_to(ROOT)} ({im.width}x{im.height})", file=sys.stderr)
    SUMS.write_text(sums_text())


def main():
    if "--check" in sys.argv[1:]:
        if not all(p.exists() for p in (SUMS, *FILES)) or SUMS.read_text() != sums_text():
            print("Hình nền trong image không khớp theme/wallpaper/*.webp — "
                  "chạy python3 tools/build-wallpaper.py", file=sys.stderr)
            return 1
        return 0
    build()
    return 0


if __name__ == "__main__":
    sys.exit(main())
