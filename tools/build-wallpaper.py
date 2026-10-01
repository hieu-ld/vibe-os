#!/usr/bin/env python3
"""
Vẽ hình nền mặc định của Samhie: núi đêm, tông lạnh — dựng bằng mã, không lấy
ảnh của ai, nên Samhie giữ trọn quyền phát hành lại (ảnh núi user thích trên
4kwallpapers chỉ được "personal use", xem docs/PLAN.md mục Wallpaper).

    python3 tools/build-wallpaper.py            # sinh SVG + PNG 3840x2160
    python3 tools/build-wallpaper.py --svg-only # chỉ SVG (xem nhanh)
    python3 tools/build-wallpaper.py --check    # SVG trong repo còn khớp palette? (CI)

SVG là NGUỒN, nằm ở theme/wallpaper/ (không vào image). PNG render từ nó mới
vào image, ở /usr/share/backgrounds/samhie/. Commit cả hai. Cùng lúc sinh
samhie-blur.png — bản LÀM MỜ SẴN cho màn hình đăng nhập (SDDM): làm mờ lúc
dựng thay vì bằng shader lúc chạy, máy không có tăng tốc đồ hoạ vẫn hiện đúng. --check chỉ so
SVG (chỉ cần thư viện chuẩn), nên sửa SVG thì nhớ chạy lại để ra PNG mới.

MÀU: mọi màu là PHA giữa ba màu của theme/palette.toml — shared.black,
shared.cyan, shared.white. Không có mã màu nào khác trong file này. Cyan chỉ
chiếm phần nhỏ (vầng sáng sau đỉnh núi, viền sáng) đúng tinh thần "~8% cyan".

HÌNH: seed cố định nên chạy lại ra đúng hình cũ. Đường núi dựng bằng chia đôi
đoạn thẳng rồi xô lệch ngẫu nhiên (midpoint displacement), càng xa càng mờ
và càng ngả màu trời (phối cảnh không khí). Trăng chiếu từ trái: sườn trái
của đỉnh chính sáng, sườn phải chìm trong tối.

Cần rsvg-convert (gói librsvg2-tools trên Fedora, librsvg2-bin trên Ubuntu)
và Pillow (bản làm mờ).
"""

import io
import math
import random
import subprocess
import sys
import tomllib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PALETTE = ROOT / "theme" / "palette.toml"
SVG_PATH = ROOT / "theme/wallpaper/samhie.svg"
PNG_PATH = ROOT / "files/system/usr/share/backgrounds/samhie/samhie.png"
BLUR_PATH = PNG_PATH.with_name("samhie-blur.png")
# Bản mờ nhỏ 960x540 rồi để SDDM phóng lên: đã mờ thì độ phân giải không còn
# thấy được, file ~85 KB thay vì hàng MB. Bán kính 16 ở cỡ này ≈ 64px ở 4K —
# mạnh cỡ hyprlock cũ (blur_passes 4, blur_size 8): còn thấy dáng núi, hết chi tiết.
BLUR_W, BLUR_H, BLUR_RADIUS = 960, 540, 16
W, H = 1920, 1080          # hệ toạ độ của SVG; PNG xuất gấp đôi (4K)
SEED = 2026


def load_colors():
    with PALETTE.open("rb") as f:
        shared = tomllib.load(f)["shared"]
    return {k: tuple(int(shared[k][i:i + 2], 16) for i in (1, 3, 5))
            for k in ("black", "cyan", "white")}


C = load_colors()


def mix(cyan=0.0, white=0.0):
    """Pha: bắt đầu từ black, kéo về cyan một phần `cyan`, về white một phần `white`."""
    r = [C["black"][i] + (C["cyan"][i] - C["black"][i]) * cyan for i in range(3)]
    r = [r[i] + (C["white"][i] - r[i]) * white for i in range(3)]
    return "#%02x%02x%02x" % tuple(round(v) for v in r)


def displace(a, b, rough, depth, rng, decay=0.55):
    """Đường gãy từ a tới b: chia đôi, xô điểm giữa VUÔNG GÓC với đoạn (sườn dốc
    cũng gồ ghề, không chỉ sườn thoải), lặp `depth` lần."""
    pts = [a, b]
    amp = rough
    for _ in range(depth):
        out = [pts[0]]
        for p, q in zip(pts, pts[1:]):
            dx, dy = q[0] - p[0], q[1] - p[1]
            n = math.hypot(dx, dy) or 1
            k = rng.uniform(-amp, amp)
            out += [((p[0] + q[0]) / 2 - dy / n * k, (p[1] + q[1]) / 2 + dx / n * k), q]
        pts = out
        amp *= decay
    return pts


def path(points, close_bottom=None):
    d = "M" + " L".join(f"{x:.1f},{y:.1f}" for x, y in points)
    if close_bottom is not None:
        d += f" L{points[-1][0]:.1f},{close_bottom} L{points[0][0]:.1f},{close_bottom} Z"
    return d


def range_layer(rng, y_base, peaks, rough):
    """Một dãy núi ngang màn hình: đi qua các đỉnh (x, y) cho sẵn, xô lệch giữa chúng."""
    anchors = [(-20, y_base)] + peaks + [(W + 20, y_base)]
    pts = []
    for a, b in zip(anchors, anchors[1:]):
        seg = displace(a, b, rough, 6, rng)
        pts += seg if not pts else seg[1:]
    return pts


def main_peak(rng):
    """Đỉnh chính: bóng núi + mặt sáng (sườn trái) tách bởi một sống núi gãy."""
    apex = (1000, 238)
    left = displace((400, 960), apex, 34, 8, rng, 0.6)
    right = displace(apex, (1640, 960), 30, 8, rng, 0.6)
    outline = left + right[1:]
    # Sống núi từ đỉnh chạy xuống hơi chếch phải — ranh giới sáng/tối.
    ridge = displace(apex, (1110, 960), 26, 8, rng, 0.6)
    lit = left + ridge[1:]
    return outline, lit, ridge, apex


def couloirs(rng, ridge, n):
    """Khe tối trên mặt sáng: vệt thon hai đầu từ sống núi xiên xuống trái.
    Giữa các khe là dải tuyết sáng — đó là thứ làm mặt núi ra đá, không ra tường."""
    out = []
    for i in range(n):
        t = 0.06 + 0.86 * (i + rng.uniform(0, 0.9)) / n
        sx, sy = ridge[int(t * (len(ridge) - 1))]
        length = rng.uniform(120, 320) * (1 - t * 0.4)
        ang = math.radians(rng.uniform(110, 130))
        center = displace((sx, sy), (sx + math.cos(ang) * length, sy + math.sin(ang) * length), 10, 5, rng)
        width = rng.uniform(4, 11)
        a, b = [], []
        for k, (x, y) in enumerate(center):
            u = k / (len(center) - 1)
            w = width * math.sin(math.pi * u) ** 0.6 * (1 - 0.5 * u)
            nx, ny = -math.sin(ang), math.cos(ang)
            a.append((x + nx * w, y + ny * w))
            b.append((x - nx * w, y - ny * w))
        out.append((a + b[::-1], rng.uniform(0.4, 0.8)))
    return out


def aurora(rng):
    """Cực quang: vài dải lượn chéo trời. Mỗi dải là một tấm màn — mép dưới sáng,
    nhạt dần lên trên — cộng các tia dọc mảnh cho ra kết cấu màn, rồi làm mờ."""
    out = []
    bands = [  # (y mép dưới ở x=0, độ dốc, biên độ lượn, cao màn, độ đục)
        (330, -0.10, 38, 170, 0.34),
        (250, -0.07, 30, 120, 0.22),
        (410, -0.12, 24, 90, 0.16),
    ]
    for n, (y0, slope, amp, height, op) in enumerate(bands):
        ph1, ph2 = rng.uniform(0, 6.3), rng.uniform(0, 6.3)
        xs = list(range(-40, W + 41, 20))

        def bottom(x):
            return (y0 + slope * x + amp * math.sin(x / 260 + ph1)
                    + amp * 0.4 * math.sin(x / 97 + ph2))

        def top(x):
            return bottom(x) - height * (0.6 + 0.4 * math.sin(x / 180 + ph2) ** 2)

        # Hai đầu dải nhạt dần, không cắt ngang cạnh màn hình.
        out.append(f'<linearGradient id="aurx{n}" x1="0" y1="0" x2="1" y2="0">'
                   f'<stop offset="0" stop-color="#fff" stop-opacity="0"/>'
                   f'<stop offset="0.25" stop-color="#fff" stop-opacity="1"/>'
                   f'<stop offset="0.8" stop-color="#fff" stop-opacity="1"/>'
                   f'<stop offset="1" stop-color="#fff" stop-opacity="0"/></linearGradient>'
                   f'<mask id="aurm{n}"><rect x="-40" y="0" width="{W + 80}" height="{H}" fill="url(#aurx{n})"/></mask>')
        curtain = ("M" + " L".join(f"{x},{bottom(x):.1f}" for x in xs)
                   + " L" + " L".join(f"{x},{top(x):.1f}" for x in reversed(xs)) + " Z")
        out.append(f'<g mask="url(#aurm{n})" opacity="{op}">')
        out.append(f'<path d="{curtain}" fill="url(#aur)" filter="url(#blur14)"/>')
        # Tia dọc: vạch mảnh từ mép dưới lên, dài ngắn khác nhau.
        rays = []
        for x in range(-40, W + 41, 7):
            if rng.random() < 0.55:
                b = bottom(x)
                length = (b - top(x)) * rng.uniform(0.4, 1.1)
                rays.append(f"M{x},{b:.1f} L{x + length * 0.08:.1f},{b - length:.1f}")
        out.append(f'<path d="{" ".join(rays)}" stroke="{mix(cyan=0.9, white=0.35)}" stroke-width="2.2" '
                   f'fill="none" opacity="0.35" filter="url(#blur3)"/>')
        # Mép dưới sáng rõ nhất.
        edge = "M" + " L".join(f"{x},{bottom(x):.1f}" for x in xs)
        out.append(f'<path d="{edge}" stroke="{mix(cyan=0.9, white=0.5)}" stroke-width="5" fill="none" '
                   f'opacity="0.55" filter="url(#blur8)"/>')
        out.append("</g>")
    return out


def sparkles(rng, n):
    """Vài ngôi sao sáng có vầng và tia chữ thập — điểm nhấn, không phải nền."""
    out = []
    star = mix(cyan=0.15, white=0.92)
    for _ in range(n):
        x, y = rng.uniform(60, W - 60), rng.uniform(30, 470)
        s = rng.uniform(5, 11)
        out.append(f'<g opacity="{rng.uniform(0.6, 0.95):.2f}">'
                   f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{s * 0.9:.1f}" fill="{star}" opacity="0.25" filter="url(#blur3)"/>'
                   f'<path d="M{x - s:.1f},{y:.1f} L{x + s:.1f},{y:.1f} M{x:.1f},{y - s:.1f} L{x:.1f},{y + s:.1f}" '
                   f'stroke="{star}" stroke-width="0.7" stroke-linecap="round" opacity="0.8"/>'
                   f'<circle cx="{x:.1f}" cy="{y:.1f}" r="1.5" fill="{star}"/></g>')
    return out


def svg():
    rng = random.Random(SEED)
    sky_top, sky_low = mix(cyan=0.05, white=0.03), mix(cyan=0.15, white=0.08)
    haze = mix(cyan=0.12, white=0.16)
    parts = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}">',
             "<!-- SINH TỰ ĐỘNG bởi tools/build-wallpaper.py từ theme/palette.toml — đừng sửa tay. -->",
             "<defs>",
             f'<linearGradient id="sky" x1="0" y1="0" x2="0" y2="1">'
             f'<stop offset="0" stop-color="{sky_top}"/><stop offset="0.72" stop-color="{sky_low}"/>'
             f'<stop offset="1" stop-color="{sky_top}"/></linearGradient>',
             f'<radialGradient id="glow" cx="0.52" cy="0.66" r="0.42">'
             f'<stop offset="0" stop-color="{mix(cyan=0.55, white=0.1)}" stop-opacity="0.22"/>'
             f'<stop offset="1" stop-color="{mix(cyan=0.55)}" stop-opacity="0"/></radialGradient>',
             f'<linearGradient id="lit" x1="0" y1="0" x2="1" y2="0.35">'
             f'<stop offset="0" stop-color="{mix(cyan=0.16, white=0.42)}"/>'
             f'<stop offset="0.75" stop-color="{mix(cyan=0.12, white=0.20)}"/>'
             f'<stop offset="1" stop-color="{mix(cyan=0.10, white=0.03)}"/></linearGradient>',
             f'<linearGradient id="shade" x1="0" y1="0" x2="0" y2="1">'
             f'<stop offset="0" stop-color="{mix(cyan=0.09, white=0.05)}"/>'
             f'<stop offset="1" stop-color="{mix(cyan=0.03)}"/></linearGradient>',
             f'<linearGradient id="fog" x1="0" y1="0" x2="0" y2="1">'
             f'<stop offset="0" stop-color="{haze}" stop-opacity="0"/>'
             f'<stop offset="0.55" stop-color="{haze}" stop-opacity="0.35"/>'
             f'<stop offset="1" stop-color="{haze}" stop-opacity="0"/></linearGradient>',
             '<filter id="blur8" x="-10%" y="-50%" width="120%" height="200%"><feGaussianBlur stdDeviation="8"/></filter>',
             '<filter id="blur2"><feGaussianBlur stdDeviation="1.2"/></filter>',
             '<filter id="blur30" x="-20%" y="-100%" width="140%" height="300%"><feGaussianBlur stdDeviation="30"/></filter>',
             f'<linearGradient id="aur" x1="0" y1="1" x2="0" y2="0">'
             f'<stop offset="0" stop-color="{mix(cyan=0.9, white=0.3)}" stop-opacity="0.9"/>'
             f'<stop offset="0.5" stop-color="{mix(cyan=0.8)}" stop-opacity="0.4"/>'
             f'<stop offset="1" stop-color="{mix(cyan=0.6)}" stop-opacity="0"/></linearGradient>',
             '<filter id="blur14" x="-10%" y="-50%" width="120%" height="200%"><feGaussianBlur stdDeviation="14"/></filter>',
             '<filter id="blur3" x="-50%" y="-50%" width="200%" height="200%"><feGaussianBlur stdDeviation="3"/></filter>',
             "</defs>",
             f'<rect width="{W}" height="{H}" fill="url(#sky)"/>']

    # Sao: thưa, mờ dần về phía chân trời.
    for _ in range(170):
        x, y = rng.uniform(0, W), rng.uniform(0, 560) ** 1.0
        fade = 1 - y / 600
        r = rng.choice([0.5, 0.6, 0.7, 0.9, 1.2])
        parts.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="{mix(white=0.9)}" '
                     f'opacity="{rng.uniform(0.15, 0.7) * fade:.2f}"/>')

    # Cực quang + sao thêm dùng bộ ngẫu nhiên RIÊNG: `rng` ở trên còn dựng núi,
    # rút thêm số từ nó là dáng núi (đã duyệt 2026-09-28) đổi theo.
    sky = random.Random(SEED + 1)
    for _ in range(140):
        x, y = sky.uniform(0, W), sky.uniform(0, 520)
        parts.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{sky.choice([0.6, 0.8, 1.0, 1.3])}" '
                     f'fill="{mix(white=0.9)}" opacity="{sky.uniform(0.25, 0.85) * (1 - y / 560):.2f}"/>')
    parts += aurora(sky)
    parts += sparkles(sky, 9)

    parts.append(f'<rect width="{W}" height="{H}" fill="url(#glow)"/>')

    # Dãy xa nhất → gần dần. Mỗi lớp: (đáy, các đỉnh, độ gồ ghề, màu, sương phía trước).
    far = range_layer(rng, 700, [(170, 575), (430, 610), (700, 560), (1330, 600), (1600, 548), (1760, 600)], 26)
    parts.append(f'<path d="{path(far, H)}" fill="{mix(cyan=0.09, white=0.11)}" opacity="0.9"/>')
    parts.append(f'<rect y="560" width="{W}" height="220" fill="url(#fog)"/>')

    mid = range_layer(rng, 760, [(90, 640), (330, 690), (560, 655), (1420, 690), (1660, 630), (1880, 675)], 30)
    parts.append(f'<path d="{path(mid, H)}" fill="{mix(cyan=0.06, white=0.06)}"/>')
    parts.append(f'<rect y="650" width="{W}" height="220" fill="url(#fog)" opacity="0.8"/>')

    outline, lit, ridge, apex = main_peak(rng)
    parts.append(f'<path d="{path(outline, H)}" fill="url(#shade)"/>')
    parts.append(f'<path d="{path(lit)} Z" fill="url(#lit)" opacity="0.9"/>')
    parts.append('<clipPath id="litclip"><path d="' + path(lit) + ' Z"/></clipPath>')
    parts.append('<g clip-path="url(#litclip)">')
    for pts, op in couloirs(rng, ridge, 22):
        parts.append(f'<path d="{path(pts)} Z" fill="{mix(cyan=0.05, white=0.02)}" opacity="{op:.2f}" filter="url(#blur2)"/>')
    parts.append("</g>")
    # Viền trăng trên sống trái: nét sáng mảnh.
    left_edge = outline[:len(outline) // 2 + 1]
    parts.append(f'<path d="{path(left_edge)}" fill="none" stroke="{mix(cyan=0.3, white=0.5)}" '
                 f'stroke-width="1.4" opacity="0.55" filter="url(#blur2)"/>')


    near = range_layer(rng, 900, [(250, 820), (620, 860), (1300, 870), (1700, 815)], 20)
    parts.append(f'<path d="{path(near, H)}" fill="{mix(cyan=0.05, white=0.02)}"/>')
    parts.append(f'<path d="{path(near)}" fill="none" stroke="{mix(cyan=0.4, white=0.25)}" '
                 f'stroke-width="1" opacity="0.25" filter="url(#blur2)"/>')

    # Sương dày dưới chân đỉnh chính — tách đỉnh khỏi dãy đồi gần.
    parts.append(f'<ellipse cx="1150" cy="865" rx="760" ry="45" fill="{haze}" opacity="0.4" filter="url(#blur30)"/>')

    # Sườn tối tiền cảnh bên trái, như ảnh tham khảo.
    fg = [(x, 590 + (H + 30 - 590) * (max(x, 0) / 1150) ** 1.6) for x in range(-20, 1151, 25)]
    fg = [q for a, b in zip(fg, fg[1:]) for q in displace(a, b, 3, 3, rng)[:-1]] + [fg[-1]]
    parts.append(f'<path d="{path(fg)} L-20,{H + 30} Z" fill="{mix(cyan=0.015)}"/>')
    parts.append(f'<path d="{path(fg)}" fill="none" stroke="{mix(cyan=0.5, white=0.2)}" '
                 f'stroke-width="1.2" opacity="0.35" filter="url(#blur2)"/>')

    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def main():
    content = svg()
    if "--check" in sys.argv[1:]:
        if not SVG_PATH.exists() or SVG_PATH.read_text() != content:
            print(f"{SVG_PATH.relative_to(ROOT)} đã cũ so với theme/palette.toml — "
                  "chạy python3 tools/build-wallpaper.py", file=sys.stderr)
            return 1
        return 0
    SVG_PATH.parent.mkdir(parents=True, exist_ok=True)
    SVG_PATH.write_text(content)
    print(f"đã sinh {SVG_PATH.relative_to(ROOT)}", file=sys.stderr)
    if "--svg-only" in sys.argv[1:]:
        return 0
    PNG_PATH.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["rsvg-convert", "-w", str(W * 2), "-h", str(H * 2),
                    "-o", str(PNG_PATH), str(SVG_PATH)], check=True)
    print(f"đã sinh {PNG_PATH.relative_to(ROOT)} ({W * 2}x{H * 2})", file=sys.stderr)

    from PIL import Image, ImageFilter
    small = subprocess.run(["rsvg-convert", "-w", str(BLUR_W), "-h", str(BLUR_H), str(SVG_PATH)],
                           check=True, capture_output=True).stdout
    with Image.open(io.BytesIO(small)) as im:
        im.convert("RGB").filter(ImageFilter.GaussianBlur(BLUR_RADIUS)).save(BLUR_PATH, optimize=True)
    print(f"đã sinh {BLUR_PATH.relative_to(ROOT)} ({BLUR_W}x{BLUR_H}, mờ {BLUR_RADIUS})", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
