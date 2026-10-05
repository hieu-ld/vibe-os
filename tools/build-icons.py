#!/usr/bin/env python3
"""
Lắp icon user vẽ trên Figma (theme/icons/) vào bộ icon Samhie.

    python3 tools/build-icons.py          # sinh lại + so hình với bản gốc (cần rsvg-convert + Pillow)
    python3 tools/build-icons.py --check  # chỉ kiểm file sinh ra có khớp nguồn, thư viện chuẩn (CI chạy)

Chạy xong thì chạy tiếp tools/build-icon-theme.sh (sinh lại index.theme khi có
nhóm mới, kiểm từng SVG).

NGUỒN (theme/icons/, xem README ở đó): xuất thẳng từ Figma, tên file = tên
layer viết thường. Bảng MAP bên dưới nói mỗi hình thành những tên freedesktop
nào — một hình có thể mang nhiều tên (nhà → samhie-user-home và go-home). Hình nằm trong UNUSED là user vẽ sẵn, chưa có chỗ dùng. File không
nằm trong bảng nào là báo lỗi, để hình mới xuất không nằm im mà không ai biết.

ĐỔI GÌ so với bản Figma:
  * Màu. Figma ghi màu cứng #A1A1A1; KDE và SDDM cần `currentColor` + class
    ColorScheme-Text + khối <style id="current-color-scheme"> để tô lại theo
    bảng màu (cách Breeze làm — xem tools/import-tabler-icon.sh). Màu lạ là
    báo lỗi, theo bảng FIGMA.
  * Màu thứ hai #3B3C3C (lòng thùng rác đầy) KHÔNG phải một màu riêng mà là
    #A1A1A1 phủ mờ lên nền #0A0A0A của khung Figma. Ghi thành currentColor với
    fill-opacity tính ngược từ ba màu đó → trên nền tối ra đúng #3B3C3C, còn
    trên nền sáng vẫn là "một lớp mờ của màu chữ" thay vì mảng xám cứng.
  * clipPath bị gỡ. Figma bọc hình trong một clip đúng bằng khung — không cắt
    gì cả, mà QtSvg (thứ KDE/SDDM dùng vẽ icon) không hỗ trợ clipPath đầy đủ.
  * width/height bị gỡ, chỉ giữ viewBox — icon co giãn theo chỗ dùng.
  * `min`/`max` là khung 20x20 xoay 45° nên Figma xuất khung bao 28,28x28,28.
    Cắt lại đúng ô 20x20 ở giữa (CROP), để cùng cỡ với `close`/`minus`.
"""

import re
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "theme" / "icons"
THEME = ROOT / "files/system/usr/share/icons/Samhie"

SVG_NS = "http://www.w3.org/2000/svg"
ET.register_namespace("", SVG_NS)

# Màu dự phòng nếu KDE không thay khối style — trùng breeze-dark như các icon
# Tabler trong bộ, để icon ra sáng trên nền tối chứ không đen.
FALLBACK = "#fcfcfc"

# Màu Figma → độ đục của currentColor.
FIGMA_INK = "#a1a1a1"
FIGMA_BG = "#0a0a0a"      # nền khung "Samhie icons" trên Figma


def _alpha_over_bg(color):
    """Độ đục a sao cho a·INK + (1−a)·BG = color, đo trên cả ba kênh."""
    ink, bg, c = (bytes.fromhex(x[1:]) for x in (FIGMA_INK, FIGMA_BG, color))
    alphas = [(c[i] - bg[i]) / (ink[i] - bg[i]) for i in range(3)]
    if max(alphas) - min(alphas) > 0.02:
        sys.exit(f"{color} không phải {FIGMA_INK} phủ mờ lên {FIGMA_BG} (lệch màu giữa các kênh)")
    return round(sum(alphas) / 3, 4)


FIGMA = {
    FIGMA_INK: 1.0,
    "#3b3c3c": _alpha_over_bg("#3b3c3c"),   # lòng trash-full ≈ 0,32
}

# Hình Figma → [(nhóm, tên freedesktop), …]
MAP = {
    # Nút nguồn + màn hình đăng nhập (SDDM đọc thẳng actions/, xem SamhieIcon.qml)
    "power":      [("actions", "system-shutdown")],
    "restart":    [("actions", "system-reboot")],
    "moon":       [("actions", "system-suspend")],
    # Mũi tên thẳng
    "next-2":     [("actions", "go-next")],
    "prev-2":     [("actions", "go-previous")],
    "up-2":       [("actions", "go-up")],
    "down-2":     [("actions", "go-down")],
    # Mũi tên nhọn (chevron)
    "ar-right":   [("actions", "arrow-right")],
    "ar-left":    [("actions", "arrow-left")],
    "ar-top":     [("actions", "arrow-up")],
    "ar-bottom":  [("actions", "arrow-down")],
    # Nút cửa sổ
    "close":      [("actions", "window-close-symbolic")],
    "minus":      [("actions", "window-minimize-symbolic")],
    "max":        [("actions", "window-maximize-symbolic")],
    "min":        [("actions", "window-restore-symbolic")],
    # Nút "+" trên thanh tab Konsole — cũng là "New Tab" trong menu (tên đo
    # trong libkonsoleprivate/libkonsoleapp của image, 2026-09-30)
    "plus":       [("actions", "tab-new")],
    # Sidebar Dolphin (tên lấy từ usr/share/samhie/user-places.xbel.in). Nhà
    # và tải về mang tên RIÊNG: user-home/folder-download là thư mục TO
    # (tools/build-folder-icons.py) — bộ scalable chỉ có một file cho mọi cỡ.
    "smart-home": [("places", "samhie-user-home"), ("actions", "go-home")],
    "download":   [("places", "samhie-folder-download")],
    "network":    [("places", "folder-network")],
    "trash":      [("places", "user-trash")],
    "trash-full": [("places", "user-trash-full")],
    "hdd":        [("devices", "drive-harddisk")],
    "recent":     [("actions", "document-open-recent")],
    # Thanh công cụ Dolphin (2026-10-03). Tên đo bằng chuỗi trong dolphin,
    # libdolphinprivate và KIO/KF6 của image: Copy Location = edit-copy-path,
    # nút sửa đường dẫn của KUrlNavigator = edit-entry, nút ☰ = application-menu.
    "copy":       [("actions", "edit-copy"), ("actions", "edit-copy-path")],
    "edit":       [("actions", "document-edit"), ("actions", "edit-entry")],
    "filter":     [("actions", "view-filter")],
    "list":       [("actions", "view-list-icons")],
    # Nút chế độ Details (user vẽ 2026-10-05, thay icon Tabler cũ).
    "details":    [("actions", "view-list-details")],
    "menu":       [("actions", "application-menu")],
    # Nút lùi/tiến hình tròn của Dolphin (user vẽ 2026-10-03). Tên RIÊNG, không
    # đè go-previous/go-next: hai tên đó còn dùng ở menu khay, lịch, nút đăng
    # nhập SDDM. dolphin-ui.sh gán tên này cho go_back/go_forward.
    "prev-circle": [("actions", "samhie-go-back")],
    "next-circle": [("actions", "samhie-go-forward")],
    # Chuông, loa — cho thanh trên (giai đoạn 3). Bản -symbolic của ba tên
    # chuông là symlink có sẵn trong repo, trỏ về đây.
    "bell":       [("status", "notification-inactive")],
    "bell-ring":  [("status", "notification-active")],
    "bell-off":   [("status", "notifications-disabled")],
    "volume":     [("status", "audio-volume-high"), ("status", "audio-volume-medium"),
                   ("status", "audio-volume-low")],
    "volume-mute": [("status", "audio-volume-muted")],
}

# Vẽ sẵn, chưa lắp. `folder` (nét mảnh): tên "folder" đã là thư mục TO tô màu
# (tools/build-folder-icons.py, bộ scalable dùng một file cho mọi cỡ). Còn lại
# chưa có chỗ dùng.
UNUSED = {"folder", "document", "desktop", "next", "prev", "up", "down"}

# Cắt khung: tên → viewBox mới. min/max: khung 20x20 xoay 45° quanh tâm
# (14,1421; 14,1421) — tâm đo từ clipPath trong bản xuất.
CROP = {
    "min": "4.1421 4.1421 20 20",
    "max": "4.1421 4.1421 20 20",
}


def q(tag):
    return f"{{{SVG_NS}}}{tag}"


def convert(name, text):
    """Trả về SVG đã chuyển đổi (chuỗi), hoặc thoát với lỗi."""
    root = ET.fromstring(text)
    if "viewBox" not in root.attrib:
        sys.exit(f"{name}.svg: thiếu viewBox")
    for a in ("width", "height"):
        root.attrib.pop(a, None)
    if name in CROP:
        root.set("viewBox", CROP[name])

    # Gỡ clipPath (và <defs> nếu chỉ chứa clipPath).
    for defs in root.findall(q("defs")):
        for child in list(defs):
            if child.tag != q("clipPath"):
                sys.exit(f"{name}.svg: <defs> có <{child.tag.split('}')[-1]}> — script chưa biết xử lý")
            defs.remove(child)
        root.remove(defs)
    for el in root.iter():
        el.attrib.pop("clip-path", None)
        el.attrib.pop("id", None)

    painted = 0
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
                continue
            alpha = FIGMA.get(v.lower())
            if alpha is None:
                sys.exit(f"{name}.svg: màu lạ {v} — chỉ nhận {', '.join(FIGMA)}")
            decls.append(f"{prop}:currentColor")
            if alpha != 1.0:
                decls.append(f"{prop}-opacity:{alpha}")
        if decls:
            el.set("style", ";".join(decls))
            el.set("class", "ColorScheme-Text")
            painted += 1
    if painted == 0:
        sys.exit(f"{name}.svg: không có phần tử vẽ nào")

    style = ET.Element(q("style"), {"id": "current-color-scheme", "type": "text/css"})
    style.text = f".ColorScheme-Text {{ color: {FALLBACK}; }}"
    defs = ET.Element(q("defs"))
    defs.append(style)
    root.insert(0, defs)

    ET.indent(root, space="  ")
    body = ET.tostring(root, encoding="unicode")
    return (
        "<?xml version='1.0' encoding='utf-8'?>\n"
        f"<!-- Samhie — user vẽ trên Figma, nguồn theme/icons/{name}.svg.\n"
        "     Sinh bằng tools/build-icons.py, đừng sửa tay. -->\n"
        f"{body}\n"
    )


def outputs():
    """{đường dẫn đích: nội dung} cho mọi icon, sau khi kiểm bảng MAP/UNUSED."""
    names = {p.stem for p in SRC.glob("*.svg")}
    stray = names - MAP.keys() - UNUSED
    missing = MAP.keys() - names
    if stray:
        sys.exit(f"theme/icons/ có hình chưa nằm trong MAP hay UNUSED: {', '.join(sorted(stray))}")
    if missing:
        sys.exit(f"MAP nhắc tới hình không có trong theme/icons/: {', '.join(sorted(missing))}")
    out = {}
    for name, targets in sorted(MAP.items()):
        svg = convert(name, (SRC / f"{name}.svg").read_text())
        for group, icon in targets:
            out[THEME / group / "scalable" / f"{icon}.svg"] = svg
    return out


def compare(name, dest):
    """Vẽ bản Figma và bản đã chuyển trên nền Figma, so từng điểm ảnh."""
    from PIL import Image, ImageChops

    src = (SRC / f"{name}.svg").read_text()
    if name in CROP:
        # Chỉ sửa thẻ <svg> gốc — <rect> của clipPath cũng có width/height.
        head, tail = src.split(">", 1)
        head = re.sub(r'viewBox="[^"]*"', f'viewBox="{CROP[name]}"', head)
        head = re.sub(r'\s(width|height)="[^"]*"', "", head)
        src = head + ">" + tail
    new = dest.read_text().replace(FALLBACK, FIGMA_INK)
    with tempfile.TemporaryDirectory() as tmp:
        pngs = []
        for tag, text in (("goc", src), ("moi", new)):
            svg, png = Path(tmp, f"{tag}.svg"), Path(tmp, f"{tag}.png")
            svg.write_text(text)
            subprocess.run(["rsvg-convert", "-w", "128", "-h", "128", "-b", FIGMA_BG,
                            "-o", str(png), str(svg)], check=True)
            pngs.append(Image.open(png).convert("RGB"))
        diff = ImageChops.difference(*pngs).convert("L")
        bad = sum(1 for v in diff.getdata() if v > 24)
    return bad


def main():
    check = "--check" in sys.argv[1:]
    out = outputs()

    if check:
        stale = [p for p, s in out.items() if not p.exists() or p.read_text() != s]
        for p in stale:
            print(f"CŨ: {p.relative_to(ROOT)}", file=sys.stderr)
        if stale:
            sys.exit("Icon sinh ra không khớp theme/icons/ — chạy python3 tools/build-icons.py rồi commit.")
        print(f"icon: {len(out)} file khớp nguồn ({len(MAP)} hình, {len(UNUSED)} vẽ sẵn chưa lắp)")
        return

    for p, s in out.items():
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(s)

    # So hình: mỗi hình một lần (các tên phụ cùng nội dung).
    worst = 0
    for name, targets in sorted(MAP.items()):
        group, icon = targets[0]
        bad = compare(name, THEME / group / "scalable" / f"{icon}.svg")
        worst = max(worst, bad)
        mark = "" if bad == 0 else f"  ← lệch {bad} điểm ảnh"
        print(f"  {name:12} → {', '.join(i for _, i in targets)}{mark}")
    print(f"\n{len(out)} file từ {len(MAP)} hình. Vẽ sẵn chưa lắp: {', '.join(sorted(UNUSED))}")
    if worst > 0:
        sys.exit("Có hình vẽ ra khác bản Figma (ở 128px, trên nền Figma) — xem lại trước khi commit.")


if __name__ == "__main__":
    main()
