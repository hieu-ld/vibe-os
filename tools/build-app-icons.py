#!/usr/bin/env python3
"""
Lắp icon APP user vẽ (theme/icons/apps/, nhiều màu) vào bộ icon Samhie.

    python3 tools/build-app-icons.py          # sinh lại
    python3 tools/build-app-icons.py --check  # chỉ kiểm file sinh ra khớp nguồn (CI chạy)

NGUỒN: user vẽ trên Figma, khung 32 (hình nằm trong 2..30; bản trước
2026-10-05 là khung 40, hình 6..34 — script nhận cả hai); xuất SVG đè vào
theme/icons/apps/<tên icon>.svg. Màu giữ nguyên (icon app, không tô lại).

ĐỔI GÌ so với bản Figma (user muốn 2026-10-01):
  * Khung cắt về ô 32 ở giữa (khung 40: "4 4 32 32"), width/height 32: dock
    vẽ icon 32px trong ô 40, hình 28px cách lề 2px như bản vẽ — 1 đơn vị = 1
    điểm ảnh, nét không nhoè.
  * VIỀN NGOÀI vẽ VÀO TRONG hình (kiểu "Inside" của Figma). Figma xuất nét 1px
    nằm GIỮA đường viền, ở toạ độ nguyên thì nét vắt ngang hai hàng điểm ảnh →
    2 hàng mờ nửa độ sáng (thấy cả trong Dock.png user xuất). Cách làm: nét dày
    gấp đôi, che (mask) theo chính hình đó → còn đúng nửa phía trong, sắc 1px.
    Mọi thứ khác cũng bị che theo hình, nên dáng icon = đúng đường viền.
    Dùng <mask>, KHÔNG dùng clipPath: QtSvg 6.11 (thứ Quickshell và KDE vẽ
    icon) bỏ qua clipPath — viền ra dày 2px lấn ra ngoài; mask thì Qt vẽ khớp
    rsvg (đo 2026-10-01: lệch ≤ 10 điểm ảnh, chỉ ở mép khử răng cưa).
  * VIỀN BẮT SÁNG như bản 1.0 (chỉ khi ICONS khai độ sáng — bản vẽ 2026-10-20
    đã tự có gradient ánh sáng nên KHÔNG thêm; so ảnh: thêm vào thì góc
    trên-trái trắng chồng lên ánh sáng user vẽ) (~/Storage/Archive/Distro, theme/icons/Samhie/apps):
    nét gradient trắng phủ lên viền ngoài — sáng ở góc trên-trái (ánh sáng
    -35°), mờ ở giữa, sáng lại nhẹ ở góc dưới-phải (phản xạ) — cộng một vệt
    "dày mép" trắng rất nhạt ngay phía trong.

Mỗi icon khai trong ICONS: các phần tử có viền cần vẽ vào trong (thứ tự
trong file, đếm mọi path/rect/circle/line kể cả trong <g>; phần tử ĐẦU là
viền ngoài — dáng icon) và độ sáng viền bắt sáng (None = không thêm, khi bản
vẽ đã tự có ánh sáng). Bản vẽ 2026-10-20 có nhiều lớp viền chồng nhau (Konsole:
viền xám + viền gradient + viền tối phía trong) — mỗi lớp vẽ vào trong theo
CHÍNH hình của nó, nên viền tối 3..29 nằm ngay trong viền ngoài 2..30 như bản
vẽ. File trong theme/icons/apps/ không có trong ICONS là báo lỗi.
"""

import copy
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "theme" / "icons" / "apps"
OUT = ROOT / "files/system/usr/share/icons/Samhie/apps/scalable"

SVG_NS = "http://www.w3.org/2000/svg"
ET.register_namespace("", SVG_NS)
SHAPES = {f"{{{SVG_NS}}}{t}" for t in ("path", "rect", "circle", "line", "ellipse")}

# Độ đục viền bắt sáng ở 4 điểm dọc đường chéo: góc trên-trái, 45%, 80%,
# góc dưới-phải — đúng số của bản 1.0 (Dolphin dịu hơn vì viền cyan đậm).
# Bản vẽ trước 2026-10-20 dùng: Dolphin SOFT, Konsole + bánh răng BRIGHT.
SOFT = (0.50, 0.08, 0.08, 0.30)
BRIGHT = (0.55, 0.10, 0.10, 0.35)
BEVEL = 0.06  # vệt "dày mép" phía trong

ICONS = {
    # tên icon: (thứ tự các phần tử viền vẽ vào trong, độ sáng viền, mô tả)
    "org.kde.dolphin": ((1,), None, "Dolphin — thư mục"),
    "utilities-terminal": ((0, 1, 4), None, "Konsole — terminal \"$_\""),
    "preferences-system": ((1,), None, "bánh răng — app Settings của Samhie (samhie-settings.desktop)"),
}

HEADER = """<!-- SINH TỰ ĐỘNG bởi tools/build-app-icons.py từ theme/icons/apps/{name}.svg
     ({desc}) — đừng sửa tay: sửa bản Figma rồi chạy lại script.
     Khung 32 ở giữa; viền ngoài vẽ vào trong + viền bắt sáng (xem script). -->
"""


def shapes_in_order(elem):
    return [e for e in elem.iter() if e.tag in SHAPES]


def bare(elem):
    """Bản chép hình, không màu — làm mask / khung cho nét mới."""
    shape = copy.deepcopy(elem)
    for attr in ("fill", "stroke", "stroke-width", "stroke-opacity", "fill-opacity"):
        shape.attrib.pop(attr, None)
    return shape


def build(name):
    outlines, stops, desc = ICONS[name]
    tree = ET.parse(SRC / f"{name}.svg")
    root = tree.getroot()
    frame = float(root.get("viewBox").split()[2])
    if frame not in (32, 40):
        sys.exit(f"{name}: khung {frame:g} — chỉ nhận khung 32 hoặc 40")
    o = (frame - 32) / 2   # gốc ô 32 ở giữa khung

    defs = root.find(f"{{{SVG_NS}}}defs")
    if defs is None:
        defs = ET.SubElement(root, f"{{{SVG_NS}}}defs")

    def mask_of(shape, mid):
        mask = ET.SubElement(defs, f"{{{SVG_NS}}}mask", {
            "id": mid, "maskUnits": "userSpaceOnUse",
            "x": "0", "y": "0", "width": f"{frame:g}", "height": f"{frame:g}"})
        white = copy.deepcopy(shape)
        white.set("fill", "#FFFFFF")
        mask.append(white)

    all_shapes = shapes_in_order(root)
    shape = width = None
    for k, index in enumerate(outlines):
        outline = all_shapes[index]
        stroke = outline.get("stroke")
        w = float(outline.get("stroke-width", "1"))
        if not stroke or stroke == "none":
            sys.exit(f"{name}: phần tử viền #{index} không có nét")
        own = bare(outline)
        if k == 0:
            shape, width = own, w   # viền ngoài: dáng icon, che cả icon theo nó
        # Nét gốc (nằm giữa đường viền) → bỏ; vẽ lại dày gấp đôi, che theo
        # chính hình đó → còn đúng nửa phía trong.
        for attr in ("stroke", "stroke-width"):
            outline.attrib.pop(attr, None)
        border = copy.deepcopy(own)
        border.set("stroke", stroke)
        border.set("stroke-width", f"{2 * w:g}")
        if k:
            mask_of(own, f"samhie-in{k}")
            border.set("mask", f"url(#samhie-in{k})")
        parent = next(p for p in root.iter() if outline in list(p))
        parent.insert(list(parent).index(outline) + 1, border)

    # Mask theo dáng icon (tô trắng = giữ) + gradient ánh sáng (-35°, chéo
    # qua hình: ô 2..30 của khung 32, 6..34 của khung 40).
    mask_of(shape, "samhie-shape")
    if stops:
        grad = ET.SubElement(defs, f"{{{SVG_NS}}}linearGradient", {
            "id": "samhie-rim", "gradientUnits": "userSpaceOnUse",
            "x1": f"{o + 2:g}", "y1": f"{o + 2:g}", "x2": f"{o + 30:g}", "y2": f"{o + 30:g}"})
        for offset, opacity in zip((0, 0.45, 0.8, 1), stops):
            ET.SubElement(grad, f"{{{SVG_NS}}}stop", {
                "offset": f"{offset:g}", "stop-color": "#FFFFFF", "stop-opacity": f"{opacity:g}"})

    # Mọi thứ đã vẽ (trừ defs) gom vào một nhóm che theo hình, rồi thêm viền
    # bắt sáng + vệt dày mép lên trên cùng (cũng trong nhóm, nên cũng bị che).
    group = ET.Element(f"{{{SVG_NS}}}g", {"mask": "url(#samhie-shape)"})
    for child in list(root):
        if child is not defs:
            root.remove(child)
            group.append(child)
    if stops:
        rim = copy.deepcopy(shape)
        rim.set("stroke", "url(#samhie-rim)")
        rim.set("stroke-width", f"{2 * width:g}")
        group.append(rim)
        bevel = copy.deepcopy(shape)
        bevel.set("stroke", "#FFFFFF")
        bevel.set("stroke-opacity", f"{BEVEL:g}")
        bevel.set("stroke-width", f"{4 * width:g}")
        group.append(bevel)
    root.append(group)
    # defs lên đầu cho dễ đọc.
    root.remove(defs)
    root.insert(0, defs)

    root.set("viewBox", f"{o:g} {o:g} 32 32")
    root.set("width", "32")
    root.set("height", "32")
    ET.indent(tree, "  ")
    body = ET.tostring(root, encoding="unicode")
    return HEADER.format(name=name, desc=desc) + body + "\n"


def main():
    check = "--check" in sys.argv
    sources = sorted(p.stem for p in SRC.glob("*.svg"))
    unknown = [s for s in sources if s not in ICONS]
    missing = [n for n in ICONS if n not in sources]
    if unknown or missing:
        sys.exit(f"theme/icons/apps không khớp ICONS — thừa: {unknown}, thiếu: {missing}")

    stale = []
    for name in ICONS:
        text = build(name)
        target = OUT / f"{name}.svg"
        if check:
            if not target.exists() or target.read_text() != text:
                stale.append(str(target.relative_to(ROOT)))
        else:
            target.write_text(text)
            print(f"  {target.relative_to(ROOT)}")
    if stale:
        sys.exit("Icon app chưa sinh lại (chạy python3 tools/build-app-icons.py):\n  " + "\n  ".join(stale))


if __name__ == "__main__":
    main()
