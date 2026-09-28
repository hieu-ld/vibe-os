#!/usr/bin/env python3
"""
Sinh vòng xoay của màn hình khởi động (Plymouth) — kiểu 5 chấm đuổi nhau quanh
vòng tròn như lúc Windows khởi động. User chọn kiểu này ở bản 1.0 (2026-09-24)
trong ba bản thử (chấm trắng / vòng cung cyan / vòng cung trắng). Mang nguyên
từ ~/Projects/Distro/scripts/build-plymouth-throbber.py (commit a47f18e), chỉ
đổi chỗ lấy màu và chỗ ghi file.

    python3 tools/build-plymouth-throbber.py          # sinh lại 60 khung
    python3 tools/build-plymouth-throbber.py --check  # chỉ kiểm màu khung có khớp bảng màu

Ra 60 file throbber-0001.png … -0060.png, 64x64, nền trong suốt, ở thư mục
theme files/system/usr/share/plymouth/themes/samhie/. Commit cả khung sinh ra.

MÀU CHẤM = vai trò `text` của bản tối trong theme/palette.toml. Script này cần
Pillow (python3-pil) nên gen-theme.py (chỉ thư viện chuẩn) không gọi nó: đổi
`dark.text` thì chạy lại script này. samhie.plymouth do gen-theme.py sinh có
ghi màu chấm trong chú thích, nên diff của lần đổi màu sẽ nhắc.

VÌ SAO LÀ KHUNG HÌNH chứ không vẽ bằng mã: theme Samhie dùng module "two-step"
của Plymouth (xem samhie.plymouth), module đó chỉ biết chiếu lần lượt các ảnh
throbber-NNNN.png. Đổi hình = đổi ảnh, không đụng gì tới Plymouth.

CHUYỂN ĐỘNG: mỗi chấm chạy một vòng mỗi chu kỳ, góc = p·2π − 0.55·sin(p·2π) —
chậm ở đỉnh, nhanh ở đáy, nên năm chấm dồn cụm lại ở trên rồi toả ra khi lao
xuống, đúng cái nhịp quen mắt của Windows. Chấm sau trễ chấm trước 0.07 chu kỳ.
Góc tăng đúng 2π mỗi chu kỳ nên khung 60 nối liền khung 1, không giật khi lặp.

Vẽ ở 8x rồi thu nhỏ (LANCZOS) để chấm tròn mịn, không răng cưa.
"""

import math
import sys
import tomllib
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent
PALETTE = ROOT / "theme" / "palette.toml"
OUT_DIR = ROOT / "files/system/usr/share/plymouth/themes/samhie"

# Plymouth chiếu TRỌN bộ throbber trong 2 giây bất kể có bao nhiêu khung
# (THROBBER_DURATION 2.0 trong ply-throbber.c), vẽ lại 30 lần/giây. 60 khung
# thì mỗi lần vẽ đúng một khung mới — ít hơn là nhảy cóc, nhiều hơn là phí.
FRAMES = 60
SIZE = 64            # cùng cỡ bộ nan hoa của theme spinner mà nó thay
SUPER = 8            # vẽ to gấp 8 rồi thu nhỏ
RING = 22            # bán kính quỹ đạo chấm (px, ở cỡ 64)
DOT = 2.8            # bán kính chấm
DOTS = 5
LAG = 0.07           # độ trễ giữa hai chấm, tính bằng phần chu kỳ
EASE = 0.55          # độ "chậm ở đỉnh, nhanh ở đáy"; 0 là quay đều


def dot_color():
    with PALETTE.open("rb") as f:
        hex_color = tomllib.load(f)["dark"]["text"]
    return tuple(int(hex_color[i:i + 2], 16) for i in (1, 3, 5)) + (255,)


def frame(t, color):
    big = SIZE * SUPER
    im = Image.new("RGBA", (big, big), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    c, R, r = big / 2, RING * SUPER, DOT * SUPER
    for i in range(DOTS):
        p = (t - i * LAG) % 1.0
        a = -math.pi / 2 + p * 2 * math.pi - EASE * math.sin(p * 2 * math.pi)
        x, y = c + R * math.cos(a), c + R * math.sin(a)
        d.ellipse([x - r, y - r, x + r, y + r], fill=color)
    return im.resize((SIZE, SIZE), Image.LANCZOS)


def check(color):
    # Vẽ lại từng khung trong bộ nhớ rồi so ĐIỂM ẢNH (không so byte file: cùng
    # ảnh nhưng zlib khác bản là ra file PNG khác). Không đọc màu từ ảnh được —
    # LANCZOS vọt quá ở tâm chấm (#e8e8e8 ra #f2f2f2).
    for n in range(FRAMES):
        path = OUT_DIR / f"throbber-{n + 1:04d}.png"
        if not path.exists() or (list(Image.open(path).convert("RGBA").getdata())
                                 != list(frame(n / FRAMES, color).getdata())):
            print(f"{path.relative_to(ROOT)} lệch bảng màu — "
                  "chạy python3 tools/build-plymouth-throbber.py", file=sys.stderr)
            return 1
    return 0


def main():
    color = dot_color()
    if "--check" in sys.argv[1:]:
        return check(color)
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for n in range(FRAMES):
        frame(n / FRAMES, color).save(OUT_DIR / f"throbber-{n + 1:04d}.png")
    print(f"{FRAMES} khung → {OUT_DIR.relative_to(ROOT)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
