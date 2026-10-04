#!/usr/bin/env python3
"""
Lắp bộ thư mục Dolphin user vẽ (theme/icons/folders/) vào bộ icon Samhie.

    python3 tools/build-folder-icons.py          # sinh lại
    python3 tools/build-folder-icons.py --check  # chỉ kiểm file sinh ra khớp nguồn (CI chạy)

NGUỒN: user vẽ trên Figma 2026-10-04, khung 64, CHỈ CÓ VIỀN #A1A1A1 — hai
nét vỏ thư mục (nắp phía sau, thân phía trước) giống hệt nhau ở mọi hình, cộng
một ký hiệu ở giữa. Tên file = tên icon bỏ "folder-" (download.svg →
folder-download). Màu ở theme/palette.toml, bảng [folders].

ĐỔI GÌ (user muốn: tô màu đè lên viền, như thư mục Dolphin — ảnh mẫu 2026-10-04):
  * Nắp và thân tô kín (fill + stroke cùng màu, nên hình to đúng bằng mép
    ngoài nét vẽ). Thân màu [0], nắp màu [1] tối hơn.
  * Ký hiệu đổi #A1A1A1 → màu nắp.
  * Nắp bị CẮT, chỉ còn phần phía trên y=26. Nắp gốc kéo xuống tận đáy, trùng
    mép trái/phải/dưới với thân; hai mép trùng nhau khử răng cưa thì lộ một
    viền tối mảnh quanh thân (đo bằng rsvg ở 64px). Cắt bằng hình học chứ
    không dùng clipPath/mask: QtSvg bỏ qua clipPath (xem build-app-icons.py).
    Ở y=26 thân đã phủ kín bề ngang (góc bo của thân hết ở y=25).
  * Bỏ width/height, giữ viewBox — co giãn theo chỗ dùng.

Thư mục không ký hiệu (folder, các màu, …) sinh từ hai nét vỏ thư mục.
Tên icon đo trong image 2026-10-04 (chuỗi trong libKF6KIOCore,
libdolphinprivate, setfoldericonitemaction — xem docs/PLAN.md).
"""

import importlib.util
import sys
import tomllib
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "theme" / "icons" / "folders"
OUT = ROOT / "files/system/usr/share/icons/Samhie/places/scalable"
PALETTE = ROOT / "theme" / "palette.toml"

SVG_NS = "http://www.w3.org/2000/svg"
ET.register_namespace("", SVG_NS)
INK = "#a1a1a1"

# Hai nét vỏ thư mục đúng như Figma xuất. Hình nào khác là báo lỗi — vẽ lại vỏ
# thì sửa ở đây (và BACK_CUT).
BACK = ("M4 14C4 9.58172 7.58172 6 12 6H28.3944C30.647 6 32.7505 7.12577 34 9"
        "C35.2495 10.8742 37.353 12 39.6056 12H52C56.4183 12 60 15.5817 60 20V50"
        "C60 54.4183 56.4183 58 52 58H12C7.58172 58 4 54.4183 4 50V14Z")
FRONT = ("M4 25C4 20.5817 7.58172 17 12 17H32H52C56.4183 17 60 20.5817 60 25V50"
         "C60 54.4183 56.4183 58 52 58H12C7.58172 58 4 54.4183 4 50V25Z")
BACK_CUT = BACK.replace("V50C60 54.4183 56.4183 58 52 58H12C7.58172 58 4 54.4183 4 50V14Z",
                        "V26H4V14Z")

# Thư mục không ký hiệu, màu default. folder-open: Dolphin dùng (chuỗi trong
# libdolphinprivate) — để Breeze thì lúc đó hiện thư mục xanh của Breeze.
# user-home: KIO dùng cho thư mục home khi xem dạng thư mục to (vd /var/home);
# user chưa vẽ ký hiệu nhà nên tạm là thư mục trơn.
PLAIN = ["folder", "inode-directory", "folder-open", "user-home"]
# Màu trong menu "màu thư mục" của Dolphin (setfoldericonitemaction).
COLORS = ["red", "yellow", "orange", "green", "cyan", "blue", "violet", "brown", "grey"]

HEADER = """<?xml version='1.0' encoding='utf-8'?>
<!-- Samhie — thư mục user vẽ, {src}, màu [folders].{color} của
     theme/palette.toml. Sinh bằng tools/build-folder-icons.py, đừng sửa tay. -->
"""


def q(tag):
    return f"{{{SVG_NS}}}{tag}"


def palette():
    with PALETTE.open("rb") as f:
        folders = tomllib.load(f).get("folders", {})
    want = {"default", *COLORS}
    if set(folders) != want:
        sys.exit(f"[folders] trong palette.toml phải có đúng: {', '.join(sorted(want))}")
    for name, pair in folders.items():
        if not (isinstance(pair, list) and len(pair) == 2
                and all(isinstance(c, str) and len(c) == 7 and c.startswith("#")
                        and c == c.lower() for c in pair)):
            sys.exit(f"[folders] {name}: phải là [\"#thân\", \"#nắp\"] chữ thường")
    return folders


def build(src_name, text, colors, color_name):
    body, lid = colors
    root = ET.fromstring(text)
    if root.get("viewBox") != "0 0 64 64":
        sys.exit(f"{src_name}: khung phải là 0 0 64 64")
    for a in ("width", "height"):
        root.attrib.pop(a, None)

    shapes = list(root)
    if len(shapes) < 2 or [s.get("d") for s in shapes[:2]] != [BACK, FRONT]:
        sys.exit(f"{src_name}: hai phần tử đầu phải là nắp + thân thư mục như các hình khác")
    for el, d, color in ((shapes[0], BACK_CUT, lid), (shapes[1], FRONT, body)):
        el.set("d", d)
        el.set("fill", color)
        el.set("stroke", color)

    for el in shapes[2:]:
        for sub in el.iter():
            for prop in ("stroke", "fill"):
                v = sub.get(prop)
                if v is None or v == "none":
                    continue
                if v.lower() != INK:
                    sys.exit(f"{src_name}: màu lạ {v} — ký hiệu chỉ vẽ bằng {INK}")
                sub.set(prop, lid)

    ET.indent(root, space="  ")
    return HEADER.format(src=src_name, color=color_name) + ET.tostring(root, encoding="unicode") + "\n"


def plain_source():
    """Hình chỉ có vỏ thư mục (lấy từ một hình bất kỳ, bỏ ký hiệu)."""
    return (f'<svg width="64" height="64" viewBox="0 0 64 64" fill="none" xmlns="{SVG_NS}">'
            f'<path d="{BACK}" stroke="{INK}" stroke-width="2"/>'
            f'<path d="{FRONT}" stroke="{INK}" stroke-width="2"/></svg>')


def outputs():
    folders = palette()
    out = {}
    plain = plain_source()
    for icon in PLAIN:
        out[OUT / f"{icon}.svg"] = build("vỏ thư mục", plain, folders["default"], "default")
    for color in COLORS:
        out[OUT / f"folder-{color}.svg"] = build("vỏ thư mục", plain, folders[color], color)
    for src in sorted(SRC.glob("*.svg")):
        out[OUT / f"folder-{src.stem}.svg"] = build(
            f"theme/icons/folders/{src.name}", src.read_text(), folders["default"], "default")

    # build-icons.py ghi chung thư mục places/ — trùng tên là hai script ghi đè
    # nhau (vd sidebar dùng user-home nét mảnh, thư mục to cũng user-home).
    spec = importlib.util.spec_from_file_location("build_icons", ROOT / "tools/build-icons.py")
    other = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(other)
    clash = sorted(p.name for p in out if p in other.outputs())
    if clash:
        sys.exit(f"Trùng tên với tools/build-icons.py: {', '.join(clash)}")
    return out


def main():
    check = "--check" in sys.argv[1:]
    out = outputs()
    if check:
        stale = [p for p, s in out.items() if not p.exists() or p.read_text() != s]
        for p in stale:
            print(f"CŨ: {p.relative_to(ROOT)}", file=sys.stderr)
        if stale:
            sys.exit("Thư mục sinh ra không khớp nguồn — chạy python3 tools/build-folder-icons.py rồi commit.")
        print(f"thư mục: {len(out)} file khớp nguồn")
        return
    OUT.mkdir(parents=True, exist_ok=True)
    for p, s in out.items():
        p.write_text(s)
        print(f"  {p.relative_to(ROOT)}")
    print(f"\n{len(out)} file.")


if __name__ == "__main__":
    main()
