#!/usr/bin/env python3
"""
Sinh bộ thư mục Dolphin tô màu của Samhie (Samhie/places/scalable/).

    python3 tools/build-folder-icons.py          # sinh lại
    python3 tools/build-folder-icons.py --check  # chỉ kiểm file sinh ra khớp nguồn (CI chạy)

VỎ THƯ MỤC: user vẽ trên Figma 2026-10-04, khung 64 — hai nét (nắp phía sau,
thân phía trước) ghi thẳng ở BACK/FRONT bên dưới. Nắp và thân tô kín (fill +
stroke cùng màu, hình to đúng bằng mép ngoài nét vẽ): thân màu [0], nắp màu
[1] tối hơn (bảng [folders] của theme/palette.toml). Nắp bị CẮT, chỉ còn phần
phía trên y=26: nắp gốc kéo xuống tận đáy, trùng mép với thân, khử răng cưa
thì lộ viền tối mảnh quanh thân (đo rsvg 64px). Cắt bằng hình học — QtSvg bỏ
qua clipPath (xem build-app-icons.py).

KÝ HIỆU (2026-10-08, thay bộ user tự vẽ): Hugeicons bản free, khung 24, nét
1,5 — theme/icons/hugeicons/symbol-<tên>.svg (giấy phép + nguồn: README ở đó;
cùng file sinh icon menu Symbol qua tools/build-icons.py). Ô 24 đặt vào
thân thư mục phóng 32/24 (x 16..48, y 21..53 — thân 17..58), nét 1,5 → 2 như
bản vẽ tay cũ (user chọn 2026-10-05). Màu ký hiệu = màu [2] của thư mục đó,
trừ ký hiệu có màu riêng ([folder-symbols], vd important đỏ) — riêng trên thư
mục cùng tông (SAME_HUE) thì dùng màu [2] của thư mục, không thì chìm.

TÊN ICON: folder-<ký hiệu> (màu default — trùng tên KDE dùng cho thư mục XDG
và menu Other), folder-<ký hiệu>-<màu> (Samhie riêng: hàng màu thư mục của
Dolphin đổi màu mà giữ ký hiệu — style "samhie" mục 47), folder-<màu>.
Sinh đủ 9 màu: thư mục đã tô màu đã bỏ khỏi hàng màu (cam, xanh lá…) vẫn ghép
được ký hiệu. user-home: thư mục home khi xem dạng to (vd /var/home).
"""

import sys
import tomllib
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "theme" / "icons" / "hugeicons"
OUT = ROOT / "files/system/usr/share/icons/Samhie/places/scalable"
PALETTE = ROOT / "theme" / "palette.toml"

SVG_NS = "http://www.w3.org/2000/svg"
ET.register_namespace("", SVG_NS)

# Hai nét vỏ thư mục đúng như Figma xuất (khung 64).
BACK = ("M4 14C4 9.58172 7.58172 6 12 6H28.3944C30.647 6 32.7505 7.12577 34 9"
        "C35.2495 10.8742 37.353 12 39.6056 12H52C56.4183 12 60 15.5817 60 20V50"
        "C60 54.4183 56.4183 58 52 58H12C7.58172 58 4 54.4183 4 50V14Z")
FRONT = ("M4 25C4 20.5817 7.58172 17 12 17H32H52C56.4183 17 60 20.5817 60 25V50"
         "C60 54.4183 56.4183 58 52 58H12C7.58172 58 4 54.4183 4 50V25Z")
BACK_CUT = BACK.replace("V50C60 54.4183 56.4183 58 52 58H12C7.58172 58 4 54.4183 4 50V14Z",
                        "V26H4V14Z")

# Thư mục không ký hiệu, màu default. folder-open: Dolphin dùng (chuỗi trong
# libdolphinprivate) — để Breeze thì lúc đó hiện thư mục xanh của Breeze.
PLAIN = ["folder", "inode-directory", "folder-open"]
# Màu trong bảng [folders] ngoài default (hàng màu Dolphin chỉ còn 4 — mục 47
# style "samhie" — nhưng sinh đủ cho thư mục đã tô từ trước).
COLORS = ["red", "yellow", "orange", "green", "cyan", "blue", "violet", "brown", "grey"]
# Ký hiệu: tên → file theme/icons/hugeicons/symbol-<tên>.svg. Trùng danh sách
# menu Symbol của style "samhie" (FOLDER_SYMBOLS) — thêm/bớt thì sửa cả hai.
SYMBOLS = ["cloud", "design", "development", "documents", "download", "important",
           "library", "mail", "music", "pictures", "print", "tar", "temp", "videos"]
# Ký hiệu chỉ có bản default, tên icon riêng: hình Hugeicons → tên icon.
EXTRA = {"smart-home": "user-home"}
# Ô 24 của Hugeicons → x 16..48, y 21..53 trong khung 64.
SYMBOL_TRANSFORM = "translate(16 21) scale(1.333333)"
# Ký hiệu có màu riêng không dùng màu đó trên thư mục cùng tông.
SAME_HUE = {"important": {"red"}}
# Màu trong file Hugeicons (trang web ghi currentColor, repo ghi #141B34).
INK = {"currentcolor", "#141b34", "black", "#000000"}

HEADER = """<?xml version='1.0' encoding='utf-8'?>
<!-- Samhie — vỏ thư mục user vẽ{symbol}, màu [folders].{color} của
     theme/palette.toml. Sinh bằng tools/build-folder-icons.py, đừng sửa tay. -->
"""


def q(tag):
    return f"{{{SVG_NS}}}{tag}"


def is_hex(c):
    return isinstance(c, str) and len(c) == 7 and c.startswith("#") and c == c.lower()


def palette():
    with PALETTE.open("rb") as f:
        pal = tomllib.load(f)
    folders = pal.get("folders", {})
    want = {"default", *COLORS}
    if set(folders) != want:
        sys.exit(f"[folders] trong palette.toml phải có đúng: {', '.join(sorted(want))}")
    for name, trio in folders.items():
        if not (isinstance(trio, list) and len(trio) == 3 and all(is_hex(c) for c in trio)):
            sys.exit(f"[folders] {name}: phải là [\"#thân\", \"#nắp\", \"#ký hiệu\"] chữ thường")
    own = pal.get("folder-symbols", {})
    for name, c in own.items():
        if name not in SYMBOLS:
            sys.exit(f"[folder-symbols] {name}: không có trong SYMBOLS")
        if not is_hex(c):
            sys.exit(f"[folder-symbols] {name}: phải là \"#rrggbb\" chữ thường")
    return folders, own


def symbol_group(name, color):
    """<g> chứa ký hiệu Hugeicons `name`, tô `color`, đặt vào thân thư mục."""
    src = SRC / f"{name}.svg"
    root = ET.fromstring(src.read_text())
    if root.get("viewBox") != "0 0 24 24":
        sys.exit(f"{src.name}: khung phải là 0 0 24 24")
    group = ET.Element(q("g"), {"transform": SYMBOL_TRANSFORM})
    # Thuộc tính vẽ ở thẻ gốc (fill="none", stroke, nét…) chuyển xuống nhóm.
    for attr in ("fill", "stroke", "stroke-width", "stroke-linecap", "stroke-linejoin"):
        if attr in root.attrib:
            group.set(attr, root.get(attr))
    for el in [group, *root.iter()]:
        for attr in ("fill", "stroke"):
            v = el.get(attr)
            if v is None or v == "none":
                continue
            if v.lower() not in INK:
                sys.exit(f"{src.name}: màu lạ {v}")
            el.set(attr, color)
    for el in list(root):
        group.append(el)
    return group


def folder(color_name, colors, symbol=None, symbol_color=None):
    body, lid, ink = colors
    root = ET.Element(q("svg"), {"viewBox": "0 0 64 64", "fill": "none"})
    for d, c in ((BACK_CUT, lid), (FRONT, body)):
        ET.SubElement(root, q("path"), {"d": d, "stroke": c, "stroke-width": "2", "fill": c})
    if symbol:
        root.append(symbol_group(symbol, symbol_color or ink))
    ET.indent(root, space="  ")
    label = f", ký hiệu theme/icons/hugeicons/{symbol}.svg" if symbol else ""
    return HEADER.format(symbol=label, color=color_name) + ET.tostring(root, encoding="unicode") + "\n"


def outputs():
    folders, own = palette()
    out = {}
    for icon in PLAIN:
        out[OUT / f"{icon}.svg"] = folder("default", folders["default"])
    for color in COLORS:
        out[OUT / f"folder-{color}.svg"] = folder(color, folders[color])
    for name in SYMBOLS:
        for color in ["default", *COLORS]:
            ink = None if color in SAME_HUE.get(name, ()) else own.get(name)
            icon = f"folder-{name}" if color == "default" else f"folder-{name}-{color}"
            out[OUT / f"{icon}.svg"] = folder(color, folders[color], f"symbol-{name}", ink)
    for src, icon in EXTRA.items():
        out[OUT / f"{icon}.svg"] = folder("default", folders["default"], src)

    # build-icons.py ghi chung thư mục places/ — trùng tên là hai script ghi đè nhau.
    import importlib.util
    spec = importlib.util.spec_from_file_location("build_icons", ROOT / "tools/build-icons.py")
    other = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(other)
    clash = sorted(p.name for p in out if p in other.outputs())
    if clash:
        sys.exit(f"Trùng tên với tools/build-icons.py: {', '.join(clash)}")
    return out


def stale_outputs(out):
    """File do script này sinh trước đây mà giờ không còn (bỏ ký hiệu, bỏ bản 16)."""
    mark = "Sinh bằng tools/build-folder-icons.py"
    theme = OUT.parent.parent
    return [p for p in theme.rglob("*.svg") if p not in out and mark in p.read_text()]


def main():
    check = "--check" in sys.argv[1:]
    out = outputs()
    stale = stale_outputs(out)
    if check:
        old = [p for p, s in out.items() if not p.exists() or p.read_text() != s]
        for p in old + stale:
            print(f"CŨ: {p.relative_to(ROOT)}", file=sys.stderr)
        if old or stale:
            sys.exit("Thư mục sinh ra không khớp nguồn — chạy python3 tools/build-folder-icons.py rồi commit.")
        print(f"thư mục: {len(out)} file khớp nguồn")
        return
    for p, s in out.items():
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(s)
    for p in stale:
        p.unlink()
        print(f"  xoá {p.relative_to(ROOT)}")
    print(f"{len(out)} file.")


if __name__ == "__main__":
    main()
