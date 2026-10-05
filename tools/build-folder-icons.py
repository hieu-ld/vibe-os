#!/usr/bin/env python3
"""
Lắp bộ thư mục Dolphin user vẽ (theme/icons/folders/) vào bộ icon Samhie.

    python3 tools/build-folder-icons.py          # sinh lại
    python3 tools/build-folder-icons.py --check  # chỉ kiểm file sinh ra khớp nguồn (CI chạy)

NGUỒN: user vẽ trên Figma 2026-10-04, khung 64, CHỈ CÓ VIỀN #A1A1A1 — hai
nét vỏ thư mục (nắp phía sau, thân phía trước) giống hệt nhau ở mọi hình, cộng
một ký hiệu ở giữa. Tên file = tên icon bỏ "folder-" (download.svg →
folder-download), trừ các hình trong RENAME (home.svg → user-home). Màu ở theme/palette.toml, bảng [folders].

ĐỔI GÌ (user muốn: tô màu đè lên viền, như thư mục Dolphin — ảnh mẫu 2026-10-04):
  * Nắp và thân tô kín (fill + stroke cùng màu, nên hình to đúng bằng mép
    ngoài nét vẽ). Thân màu [0], nắp màu [1] tối hơn.
  * Ký hiệu đổi #A1A1A1 → màu [2], tối hơn nữa (màu nắp như ảnh mẫu thì lem
    ở 64px — xem chú thích [folders] trong palette.toml).
  * Nắp bị CẮT, chỉ còn phần phía trên y=26. Nắp gốc kéo xuống tận đáy, trùng
    mép trái/phải/dưới với thân; hai mép trùng nhau khử răng cưa thì lộ một
    viền tối mảnh quanh thân (đo bằng rsvg ở 64px). Cắt bằng hình học chứ
    không dùng clipPath/mask: QtSvg bỏ qua clipPath (xem build-app-icons.py).
    Ở y=26 thân đã phủ kín bề ngang (góc bo của thân hết ở y=25).
  * Ký hiệu phóng 32/24 quanh tâm ô vẽ (SYMBOL_SCALE): user vẽ trong ô 24,
    muốn 32 cho rõ ở cỡ 48 (Details) — 2026-10-05. Nét to theo (1,5 → 2),
    user chọn sau khi so với bản giữ nét 1,5.
  * Ký hiệu important màu `danger` của bảng tối (SYMBOL_ROLE, user 2026-10-05)
    thay vì màu nắp — cả bản 16px.
  * Bỏ width/height, giữ viewBox — co giãn theo chỗ dùng.

Thư mục không ký hiệu (folder, các màu, …) sinh từ hai nét vỏ thư mục.

BẢN 16px CHỈ CÓ KÝ HIỆU (places/16/, user muốn 2026-10-04): menu chuột phải →
màu thư mục → Other vẽ icon 16px, thư mục nhỏ xíu có ký hiệu thì không đọc
được. Bộ icon được có bản vẽ riêng theo cỡ (Breeze cũng vậy): places/16 khai
Type=Fixed Size=16, đứng TRƯỚC places/scalable trong Directories — KIconLoader
chọn nó ở đúng 16px, cỡ khác vẫn lấy thư mục tô màu (đo, xem docs/PLAN.md).
Cái giá: chỗ nào vẽ ĐÚNG 16px (Details cỡ nhỏ nhất, hộp thoại mở file) cũng ra
ký hiệu trơn — nên chỉ làm cho tên có trong menu Other (SYMBOL16), không làm
cho Docs/Download/Pictures/Videos/Home. Ký hiệu tô currentColor theo màu chữ
(như icon nét khác của bộ), khung cắt chung CROP16 để mọi ký hiệu to bằng nhau.
Bản 16 KHÔNG phóng (CROP16 đã cắt sát ký hiệu); ký hiệu có SYMBOL_ROLE thì tô
currentColor lớp ColorScheme-NegativeText (KDE thay bằng ForegroundNegative
= danger).
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
OUT16 = ROOT / "files/system/usr/share/icons/Samhie/places/16"
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
PLAIN = ["folder", "inode-directory", "folder-open"]
# Hình mà tên icon KHÔNG phải folder-<tên hình>. user-home: KIO dùng cho thư
# mục home khi xem dạng thư mục to (vd /var/home).
RENAME = {"home": "user-home"}
# Màu trong menu "màu thư mục" của Dolphin (setfoldericonitemaction).
COLORS = ["red", "yellow", "orange", "green", "cyan", "blue", "violet", "brown", "grey"]
# Ký hiệu trong menu Other (setfoldericonitemaction) — có bản 16px trơn.
SYMBOL16 = ["bookmark", "cloud", "development", "games", "mail", "music", "print",
            "tar", "temp", "important"]
# Ký hiệu nằm trong x 21,5..42,5, y 26,5..47,5 (đo bằng rsvg; cloud lấn
# 0,3) — cắt ô 22 quanh đó.
CROP16 = "21 26 22 22"
# Ô vẽ ký hiệu của user: 24×24, tâm (32, 37) → phóng thành 32×32 (y 21..53,
# thân thư mục 17..58 — còn chỗ). Đo lại nếu user vẽ ký hiệu ra ngoài ô 24.
SYMBOL_CENTER = (32, 37)
SYMBOL_SCALE = 32 / 24
# Ký hiệu tô theo vai trò của bảng tối thay vì màu [folders] — tên hình → vai
# trò. Bộ icon Samhie chỉ dùng ở bản tối (palette.toml [kde] icon-theme).
SYMBOL_ROLE = {"important": "danger"}
FALLBACK = "#fcfcfc"  # như build-icons.py: KDE thay bằng màu chữ của bảng màu

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
        if not (isinstance(pair, list) and len(pair) == 3
                and all(isinstance(c, str) and len(c) == 7 and c.startswith("#")
                        and c == c.lower() for c in pair)):
            sys.exit(f"[folders] {name}: phải là [\"#thân\", \"#nắp\", \"#ký hiệu\"] chữ thường")
    return folders


def dark_role(role):
    with PALETTE.open("rb") as f:
        return tomllib.load(f)["dark"][role]


def build(src_name, text, colors, color_name, symbol_color=None):
    body, lid, symbol = colors
    symbol = symbol_color or symbol
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
                sub.set(prop, symbol)
    if len(shapes) > 2:
        cx, cy = SYMBOL_CENTER
        group = ET.SubElement(root, q("g"), {
            "transform": f"translate({cx} {cy}) scale({SYMBOL_SCALE:.6f}) translate({-cx} {-cy})"})
        for el in shapes[2:]:
            root.remove(el)
            group.append(el)

    ET.indent(root, space="  ")
    return HEADER.format(src=src_name, color=color_name) + ET.tostring(root, encoding="unicode") + "\n"


def symbol16(src_name, text, role=None):
    """Chỉ ký hiệu, tô currentColor (class ColorScheme-Text), khung CROP16.
    role: vai trò bảng tối → lớp ColorScheme-NegativeText (chỉ có danger)."""
    scheme_class, fallback = "ColorScheme-Text", FALLBACK
    if role:
        if role != "danger":
            sys.exit(f"{src_name}: bản 16px chưa có lớp ColorScheme cho vai trò {role}")
        scheme_class, fallback = "ColorScheme-NegativeText", dark_role(role)
    root = ET.fromstring(text)
    for a in ("width", "height"):
        root.attrib.pop(a, None)
    shapes = list(root)
    if [s.get("d") for s in shapes[:2]] != [BACK, FRONT]:
        sys.exit(f"{src_name}: hai phần tử đầu phải là nắp + thân thư mục")
    for el in shapes[:2]:
        root.remove(el)
    for el in root.iter():
        if el is root:
            continue
        decls = []
        for prop in ("stroke", "fill"):
            v = el.attrib.pop(prop, None)
            if v is None:
                continue
            if v == "none":
                decls.append(f"{prop}:none")
            elif v.lower() == INK:
                decls.append(f"{prop}:currentColor")
            else:
                sys.exit(f"{src_name}: màu lạ {v} — ký hiệu chỉ vẽ bằng {INK}")
        if decls:
            el.set("style", ";".join(decls))
            el.set("class", scheme_class)
    root.set("viewBox", CROP16)
    defs = ET.Element(q("defs"))
    style = ET.SubElement(defs, q("style"), {"id": "current-color-scheme", "type": "text/css"})
    style.text = f".{scheme_class} {{ color: {fallback}; }}"
    root.insert(0, defs)
    ET.indent(root, space="  ")
    return ("<?xml version='1.0' encoding='utf-8'?>\n"
            f"<!-- Samhie — ký hiệu thư mục cỡ 16 (menu màu thư mục), {src_name}.\n"
            "     Sinh bằng tools/build-folder-icons.py, đừng sửa tay. -->\n"
            + ET.tostring(root, encoding="unicode") + "\n")


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
        icon = RENAME.get(src.stem, f"folder-{src.stem}")
        role = SYMBOL_ROLE.get(src.stem)
        out[OUT / f"{icon}.svg"] = build(
            f"theme/icons/folders/{src.name}", src.read_text(), folders["default"], "default",
            dark_role(role) if role else None)

    for name in SYMBOL16:
        src = SRC / f"{name}.svg"
        out[OUT16 / f"folder-{name}.svg"] = symbol16(f"theme/icons/folders/{src.name}", src.read_text(),
                                                      SYMBOL_ROLE.get(name))

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
    for p, s in out.items():
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(s)
        print(f"  {p.relative_to(ROOT)}")
    print(f"\n{len(out)} file.")


if __name__ == "__main__":
    main()
