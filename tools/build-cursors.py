#!/usr/bin/env python3
"""
Sinh bộ con trỏ chuột Samhie từ SVG user vẽ (theme/cursors/) và bảng màu.

    python3 tools/build-cursors.py          # sinh lại mọi thứ (cần Pillow + rsvg-convert)
    python3 tools/build-cursors.py --check  # chỉ kiểm, thư viện chuẩn (CI chạy)

RA HAI ĐỊNH DẠNG, cùng một theme tên Samhie-cursors:
  * hyprcursor — Hyprland tự vẽ con trỏ bằng nó (và vẽ hộ mọi app Wayland biết
    giao thức cursor-shape: Qt 6, GTK 4…). Hyprcursor ăn thẳng SVG nên ở đây chỉ
    sinh "working state" dạng chữ: files/system/usr/share/samhie/cursors/
    (manifest.hl + hyprcursors/<hình>/{*.svg, meta.hl}). Lúc build image,
    files/scripts/cursors.sh nén nó bằng hyprcursor-util (có sẵn trong image,
    gói hyprcursor) ra /usr/share/icons/Samhie-cursors/.
  * XCursor — cho app XWayland và app Wayland tự nạp con trỏ. Định dạng ảnh
    điểm, mỗi cỡ một ảnh, nên phải vẽ ra PNG: việc đó cần rsvg-convert, làm trên
    máy rồi COMMIT file sinh ra (files/system/usr/share/icons/Samhie-cursors/
    cursors/). index.theme ghi mã băm của toàn bộ SVG + tên; --check so mã băm
    đó để biết XCursor đã cũ mà không cần vẽ lại (CI không có rsvg).

NGUỒN (theme/cursors/, xem README ở đó): xuất thẳng từ Figma. Màu trong file là
màu Figma, script đổi sang vai trò trong palette.toml theo bảng FIGMA — màu lạ
là báo lỗi, để không màu nào lọt vào image mà không qua bảng màu.

KHUNG: mọi con trỏ sinh ra ở khung 24x24 (XCURSOR_SIZE / HYPRCURSOR_SIZE = 24
là cỡ thật trên màn hình không phóng to). Hình user vẽ ở khung 16 được phóng
1,5 lần — viền 1 thành 1,5, khớp đúng default/pointer vẽ ở khung 24.

MŨI TÊN user vẽ dựng đứng, góc mũi ~55°. Đo 5 bộ con trỏ thật (Adwaita, Yaru,
DMZ, whiteglass, Breeze — 2026-09-29): cạnh trái đều dựng đứng 0°, cạnh phải
45°. User chọn xoay 27,41° (= nửa góc mũi) để cạnh trái dựng đứng như chúng.
"""

import hashlib
import math
import re
import shutil
import struct
import subprocess
import sys
import tomllib
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PALETTE = ROOT / "theme" / "palette.toml"
SRC = ROOT / "theme" / "cursors"
WORK = ROOT / "files/system/usr/share/samhie/cursors"          # hyprcursor, dạng chữ
THEME = ROOT / "files/system/usr/share/icons/Samhie-cursors"   # XCursor + index.theme
NAME = "Samhie-cursors"

SVG_NS = "http://www.w3.org/2000/svg"
CANVAS = 24
XSIZES = (24, 30, 36, 48)      # cỡ XCursor: tỉ lệ màn hình 1 / 1,25 / 1,5 / 2

# Màu Figma của user → (mục, vai trò) trong palette.toml.
FIGMA = {
    "#003845": ("shared", "cursor"),
    "#fafafa": ("shared", "cursor-outline"),
    "#bd2123": ("dark", "danger"),      # "đỏ Samhie" — con trỏ không đổi theo chế độ
}

# ── Cách đặt từng lớp vào khung 24 ────────────────────────────────────────────
S16 = "scale(1.5)"
# Xoay quanh đỉnh (12, 1) rồi dịch cho mép ngoài của viền cách góc trên-trái
# 0,5. Hằng số đo bằng rsvg (100 điểm ảnh/đơn vị) — đổi default.svg thì đo lại.
ARROW = ("default.svg", "translate(-9.94 0.42) rotate(-27.41 12 1)")
ARROW_HOT = (2.06, 1.43)       # đỉnh mũi tên sau khi xoay

# Dấu "?" (help) nằm góc dưới-phải — vùng trống bên phải đuôi mũi tên đã xoay.
# Thu nhỏ còn 0,7 thì nét cũng mảnh theo, nên nhân nét lên lại cho bằng hàng xóm.
HELP_BADGE = ("help.svg", "translate(14.8 12.3) scale(0.7)", 1.5 / 0.7)

# Vòng 5 chấm — cùng công thức chuyển động với tools/build-plymouth-throbber.py
# (chậm ở đỉnh, nhanh ở đáy). 24 khung × 83 ms ≈ 2 giây một vòng như Plymouth.
FRAMES = 24
DELAY = 83
DOTS, LAG, EASE = 5, 0.07, 0.55
WAIT_RING = dict(cx=12, cy=12, ring=7.5, dot=2.0, outline=1.0)
PROGRESS_RING = dict(cx=19.6, cy=18.4, ring=3.0, dot=1.15, outline=0.75)

# Tên con trỏ: tên đầu là hình chính, còn lại là tên khác trỏ về nó (XCursor:
# symlink; hyprcursor: define_override). Gom theo đúng cách bộ Breeze trong
# image gom (đọc symlink của /usr/share/icons/breeze_cursors, 2026-09-29) — kể
# cả các tên băm Qt/X11 đời cũ. Tên Breeze có mà ở đây không có (pencil,
# color-picker, zoom-in…) thì XCursor lấy của Breeze qua Inherits.
SHAPES = {
    "default": dict(layers=[ARROW], hot=ARROW_HOT, names=[
        "arrow", "left_ptr", "top_left_arrow", "context-menu", "alias", "link",
        "copy", "dnd-copy", "3085a0e285430894940527032f8b26df",
        "640fb0e74195791501fd1ed57b41487f", "a2a266d0498c3104214a47bd64ab0fc8",
        "1081e37283d90000800003c07f3ef6bf", "6407b0e94181790501fd1e167b474872",
        "b66166c04f8c3109214a4fbd64a50fc8"]),
    "pointer": dict(layers=[("pointer.svg", "")], hot=(4.92, 0.74), names=[
        "hand1", "hand2", "pointing_hand",
        "9d800788f1b08800ae810202380a0822", "e29285e634086352946a0e7090d73106"]),
    "text": dict(layers=[("text.svg", S16)], hot=(12, 12), names=["xterm", "ibeam"]),
    "vertical-text": dict(layers=[("text.svg", "rotate(90 12 12) " + S16)], hot=(12, 12), names=[]),
    "wait": dict(spinner=WAIT_RING, hot=(12, 12), names=["watch"]),
    "progress": dict(layers=[ARROW], spinner=PROGRESS_RING, hot=ARROW_HOT, names=[
        "half-busy", "left_ptr_watch", "00000000000000020006000e7e9ffc3f",
        "08e8e1c95fe2fc01f976f1e063a24ccd", "3ecb610c1bf2410f44200f48c40d3599"]),
    "help": dict(layers=[ARROW, HELP_BADGE], hot=ARROW_HOT, names=[
        "question_arrow", "whats_this", "left_ptr_help",
        "5c6cd98b3f3ebcb1f9c7f1c204630408", "d9ce0ab605698f320427677b458ad60b"]),
    # not-allowed.svg vẽ ở khung 23 (khung 16 xoay 45°), tâm (11,3137).
    "not-allowed": dict(layers=[("not-allowed.svg", "scale(1.5) translate(-3.3137 -3.3137)")],
                        hot=(12, 12), names=[
        "crossed_circle", "circle", "no-drop", "forbidden", "dnd-no-drop",
        "03b6e0fcb3499374a867c041f52298f0"]),
    "grab": dict(layers=[("grab.svg", S16)], hot=(12, 12), names=["openhand"]),
    "grabbing": dict(layers=[("grabbing.svg", S16)], hot=(12, 12), names=[
        "closedhand", "dnd-move", "dnd-none", "4498f0e0c1937ffe01fd06f973665830",
        "9081237383d90e509aa00f00170e968f", "fcf21c00b30f7e3f83fe0dfd12e71cff"]),
    "move": dict(layers=[("move.svg", S16)], hot=(12, 12), names=["fleur", "size_all", "all-scroll"]),
    "crosshair": dict(layers=[("crosshair.svg", S16)], hot=(12, 12), names=["cross", "tcross", "cell", "plus"]),
    # Một mũi tên hai đầu user vẽ, xoay ra bốn hướng.
    "ew-resize": dict(layers=[("ew-resize.svg", S16)], hot=(12, 12), names=[
        "e-resize", "w-resize", "size_hor", "h_double_arrow", "sb_h_double_arrow",
        "col-resize", "split_h", "left_side", "right_side"]),
    "ns-resize": dict(layers=[("ew-resize.svg", "rotate(90 12 12) " + S16)], hot=(12, 12), names=[
        "n-resize", "s-resize", "size_ver", "v_double_arrow", "sb_v_double_arrow",
        "row-resize", "split_v", "top_side", "bottom_side", "00008160000006810000408080010102"]),
    "nwse-resize": dict(layers=[("ew-resize.svg", "rotate(45 12 12) " + S16)], hot=(12, 12), names=[
        "nw-resize", "se-resize", "size_fdiag", "top_left_corner", "bottom_right_corner"]),
    "nesw-resize": dict(layers=[("ew-resize.svg", "rotate(-45 12 12) " + S16)], hot=(12, 12), names=[
        "ne-resize", "sw-resize", "size_bdiag", "top_right_corner", "bottom_left_corner"]),
}


# ---------------------------------------------------------------- màu & nguồn

def palette_colors():
    with PALETTE.open("rb") as f:
        pal = tomllib.load(f)
    return {figma: pal[section][role] for figma, (section, role) in FIGMA.items()}


def load_source(filename, colors, stroke_mul=1.0):
    """Các phần tử vẽ của một SVG Figma, đã đổi màu — bỏ khung clip của Figma."""
    path = SRC / filename
    root = ET.parse(path).getroot()
    out = []

    def walk(el):
        tag = el.tag.removeprefix("{%s}" % SVG_NS)
        if tag in ("defs", "clipPath"):
            return
        if tag == "g":
            extra = set(el.attrib) - {"clip-path"}
            if extra:
                sys.exit(f"{path.relative_to(ROOT)}: <g> có {sorted(extra)} — script chưa biết xử lý")
            for child in el:
                walk(child)
            return
        if tag not in ("path", "circle", "rect", "ellipse", "line"):
            sys.exit(f"{path.relative_to(ROOT)}: phần tử <{tag}> chưa hỗ trợ")
        attrs = dict(el.attrib)
        for key in ("fill", "stroke"):
            value = attrs.get(key)
            if value is None or value == "none":
                continue
            if value.lower() not in colors:
                sys.exit(f"{path.relative_to(ROOT)}: màu {value} không có trong bảng FIGMA "
                         "của tools/build-cursors.py — thêm vai trò vào palette.toml trước")
            attrs[key] = colors[value.lower()]
        if "stroke" in attrs and stroke_mul != 1.0:
            attrs["stroke-width"] = f"{float(attrs.get('stroke-width', 1)) * stroke_mul:.3g}"
        body = " ".join(f'{k}="{v}"' for k, v in attrs.items())
        out.append(f"<{tag} {body}/>")

    for child in root:
        walk(child)
    # Figma ghi fill="none" ở thẻ <svg> gốc cho mọi phần tử con thừa hưởng; bỏ
    # thẻ gốc thì hình tròn/đường không khai fill bị tô ĐEN (mặc định của SVG).
    root_fill = root.get("fill")
    if root_fill and root_fill != "none":
        sys.exit(f"{path.relative_to(ROOT)}: fill ở thẻ gốc là {root_fill} — chưa hỗ trợ")
    return out, root_fill


def spinner(t, ring, colors):
    """Năm chấm ở thời điểm t (0..1) của chu kỳ."""
    fill, outline = colors["#003845"], colors["#fafafa"]
    dots = []
    for i in range(DOTS):
        p = (t - i * LAG) % 1.0
        a = -math.pi / 2 + p * 2 * math.pi - EASE * math.sin(p * 2 * math.pi)
        x = ring["cx"] + ring["ring"] * math.cos(a)
        y = ring["cy"] + ring["ring"] * math.sin(a)
        dots.append(f'<circle cx="{x:.3f}" cy="{y:.3f}" r="{ring["dot"]}" fill="{fill}" '
                    f'stroke="{outline}" stroke-width="{ring["outline"]}"/>')
    return dots


def frames_of(shape, colors):
    """Danh sách SVG (một khung, hoặc FRAMES khung nếu có vòng xoay)."""
    static = []
    for layer in shape.get("layers", []):
        filename, transform, *mul = layer
        elements, root_fill = load_source(filename, colors, *mul)
        attrs = (f' transform="{transform}"' if transform else "") + (' fill="none"' if root_fill else "")
        static.append(f"<g{attrs}>")
        static += ["  " + e for e in elements]
        static.append("</g>")
    ring = shape.get("spinner")
    times = [n / FRAMES for n in range(FRAMES)] if ring else [None]
    out = []
    for t in times:
        lines = [f'<svg xmlns="{SVG_NS}" width="{CANVAS}" height="{CANVAS}" viewBox="0 0 {CANVAS} {CANVAS}">',
                 "<!-- SINH TỰ ĐỘNG bởi tools/build-cursors.py từ theme/cursors/ + palette.toml — đừng sửa tay. -->",
                 *static]
        if ring:
            lines += spinner(t, ring, colors)
        lines.append("</svg>")
        out.append("\n".join(lines) + "\n")
    return out


# ---------------------------------------------------------------- hyprcursor

def work_files(colors):
    """{đường dẫn tương đối trong WORK: nội dung} — toàn bộ working state."""
    files = {"manifest.hl": (
        "# SINH TỰ ĐỘNG bởi tools/build-cursors.py — đừng sửa tay.\n"
        f"name = {NAME}\n"
        "description = Con trỏ Samhie (user vẽ trên Figma)\n"
        "version = 1\n"
        "cursors_directory = hyprcursors\n")}
    for key, shape in SHAPES.items():
        frames = frames_of(shape, colors)
        hx, hy = shape["hot"]
        meta = ["# SINH TỰ ĐỘNG bởi tools/build-cursors.py — đừng sửa tay.",
                "resize_algorithm = bilinear",
                f"hotspot_x = {hx / CANVAS:.4f}",
                f"hotspot_y = {hy / CANVAS:.4f}",
                f"define_override = {key}",
                *(f"define_override = {n}" for n in shape["names"])]
        if len(frames) == 1:
            files[f"hyprcursors/{key}/{key}.svg"] = frames[0]
            meta.append(f"define_size = {CANVAS}, {key}.svg")
        else:
            for n, svg in enumerate(frames, 1):
                files[f"hyprcursors/{key}/{key}-{n:02d}.svg"] = svg
                meta.append(f"define_size = {CANVAS}, {key}-{n:02d}.svg, {DELAY}")
        files[f"hyprcursors/{key}/meta.hl"] = "\n".join(meta) + "\n"
    return files


def digest(files):
    h = hashlib.sha256()
    for rel in sorted(files):
        h.update(rel.encode() + b"\0" + files[rel].encode() + b"\0")
    h.update(repr(XSIZES).encode())
    return h.hexdigest()


def index_theme(files):
    return ("# SINH TỰ ĐỘNG bởi tools/build-cursors.py — đừng sửa tay.\n"
            f"# nguồn: {digest(files)}\n"
            "# (--check so dòng trên để biết cursors/ đã cũ so với theme/cursors + palette)\n"
            "[Icon Theme]\n"
            "Name=Samhie\n"
            "Comment=Con trỏ Samhie\n"
            # Tên Samhie không vẽ (pencil, zoom-in…) thì lấy của Breeze.
            "Inherits=breeze_cursors\n")


# ---------------------------------------------------------------- XCursor

def render(svg, size):
    from PIL import Image   # chỉ cần khi sinh, --check không đụng tới
    import io
    png = subprocess.run(["rsvg-convert", "-w", str(size), "-h", str(size)],
                         input=svg.encode(), capture_output=True, check=True).stdout
    return Image.open(io.BytesIO(png)).convert("RGBA")


def argb_premultiplied(im):
    # XCursor: mỗi điểm một số 32-bit ARGB, alpha NHÂN SẴN, little-endian.
    out = bytearray()
    for r, g, b, a in im.getdata():
        out += bytes((b * a // 255, g * a // 255, r * a // 255, a))
    return bytes(out)


def xcursor(frames, hot):
    chunks = []
    for size in XSIZES:
        xh = min(size - 1, round(hot[0] * size / CANVAS))
        yh = min(size - 1, round(hot[1] * size / CANVAS))
        delay = DELAY if len(frames) > 1 else 0
        for svg in frames:
            pixels = argb_premultiplied(render(svg, size))
            chunks.append((size, struct.pack("<9I", 36, 0xFFFD0002, size, 1, size, size, xh, yh, delay) + pixels))
    header = struct.pack("<4s3I", b"Xcur", 16, 0x10000, len(chunks))
    pos = 16 + 12 * len(chunks)
    toc, body = b"", b""
    for size, chunk in chunks:
        toc += struct.pack("<3I", 0xFFFD0002, size, pos + len(body))
        body += chunk
    return header + toc + body


# ---------------------------------------------------------------- chạy

def main():
    names = [n for key, shape in SHAPES.items() for n in (key, *shape["names"])]
    dup = sorted({n for n in names if names.count(n) > 1})
    if dup:
        sys.exit(f"tên con trỏ trùng giữa hai hình: {dup}")
    colors = palette_colors()
    files = work_files(colors)
    theme = index_theme(files)

    if "--check" in sys.argv[1:]:
        stale = [rel for rel, text in files.items()
                 if not (WORK / rel).exists() or (WORK / rel).read_text() != text]
        extra = sorted(str(p.relative_to(WORK)) for p in WORK.rglob("*")
                       if p.is_file() and str(p.relative_to(WORK)) not in files)
        index = THEME / "index.theme"
        if not index.exists() or index.read_text() != theme:
            stale.append(str(index.relative_to(ROOT)) + " (XCursor cũ)")
        if stale or extra:
            print("Con trỏ đã cũ so với theme/cursors + palette.toml — chạy "
                  "python3 tools/build-cursors.py:", *stale, *(f"thừa: {e}" for e in extra),
                  sep="\n  ", file=sys.stderr)
            return 1
        return 0

    if shutil.which("rsvg-convert") is None:
        sys.exit("cần rsvg-convert (Ubuntu: librsvg2-bin, Fedora: librsvg2-tools)")
    shutil.rmtree(WORK, ignore_errors=True)
    for rel, text in files.items():
        (WORK / rel).parent.mkdir(parents=True, exist_ok=True)
        (WORK / rel).write_text(text)

    xdir = THEME / "cursors"
    shutil.rmtree(xdir, ignore_errors=True)
    xdir.mkdir(parents=True)
    for key, shape in SHAPES.items():
        (xdir / key).write_bytes(xcursor(frames_of(shape, colors), shape["hot"]))
        for alias in shape["names"]:
            (xdir / alias).symlink_to(key)
    (THEME / "index.theme").write_text(theme)
    total = sum(p.stat().st_size for p in xdir.iterdir() if not p.is_symlink())
    print(f"{len(SHAPES)} hình, {sum(len(s['names']) for s in SHAPES.values())} tên phụ, "
          f"{len(files)} file hyprcursor, XCursor {total / 1024:.0f} KB", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
