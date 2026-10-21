"""Theme Kvantum "Samhie" — gen-theme.py gọi hai hàm kvconfig() và svg().

Kvantum vẽ mỗi widget Qt (nút, tab, ô nhập, menu…) bằng các phần tử trong MỘT
file SVG, tra theo id. Phần tử dạng khung (nút, tab…) cắt 9 mảnh:

    <tên>-<trạng thái>-topleft   <tên>-<trạng thái>-top     …-topright
    <tên>-<trạng thái>-left      <tên>-<trạng thái>  (ruột)  …-right
    <tên>-<trạng thái>-bottomleft …-bottom                  …-bottomright

Góc giữ nguyên cỡ, cạnh và ruột kéo dãn. Cỡ góc (px) là frame.top/left… của
mục tương ứng trong kvconfig — bằng bán kính bo (RADIUS bên dưới). Trạng thái:
normal, focused (di chuột qua), pressed, toggled (đang chọn/bật), disabled.

Bản vẽ của user (Figma 2026-10-03, Konsole + Dolphin): tab đang mở cùng màu
nền nội dung (card) liền xuống vùng chữ, bo hai góc trên; tab khác không nền;
ô đường dẫn/ô tìm viền 1px sub-secondary. Số đo nằm ở RADIUS/kvconfig().

Hai bản, cùng thư mục /usr/share/Kvantum/Samhie (2026-10-20):
    Samhie.kvconfig/.svg       — bản SÁNG (vai trò [light])
    SamhieDark.kvconfig/.svg   — bản TỐI  (vai trò [dark])
Kvantum 1.1.6 tự có cơ chế này: style "kvantum-dark" nạp "<theme>Dark" nằm
trong thư mục của theme sáng, "kvantum" nạp bản sáng (Kvantum.cpp setTheme).
Style "samhie" chọn một trong hai theo chế độ sáng/tối (style/samhie-style.cpp).
Cùng một dáng (FRAMES, RADIUS…), chỉ khác bảng vai trò.
"""

import xml.etree.ElementTree as ET
from pathlib import Path

HEADER = "SINH TỰ ĐỘNG từ theme/palette.toml bởi tools/gen-theme.py — đừng sửa tay."
HUGEICONS = Path(__file__).resolve().parent.parent / "theme" / "icons" / "hugeicons"

# Bán kính bo (px) của từng loại khung. Cửa sổ Hyprland bo 12.
RADIUS = {
    "button": 8,
    "tbutton": 8,
    "lineedit": 10,     # ô tìm Dolphin: bản vẽ cao 41, bo tròn mềm
    "tab": 8,           # bản vẽ: góc trên tab ~8px
    "tabframe": 8,
    "common": 8,
    "itemview": 10,     # ô file Dolphin: bản vẽ ô hover bo ~10 (khung focus của style "samhie" theo số này)
    "menu": 10,
    "menuitem": 6,
    "tooltip": 6,
    "scrollbarslider": 3,
    "scrollbargroove": 3,
    "progress": 3,
    "progress-pattern": 3,
    "slider": 2,
    "header": 1,
    "splitter": 0,
    "menubaritem": 6,
}
# Khe trong suốt phía trên tab (bản vẽ Konsole: dải tab 40, tab 36 → khe 4).
TAB_INSET = 4

# Phần tử vẽ bằng "frame.expansion" của Kvantum: cỡ khung khai trong kvconfig
# (dùng để TÍNH kích thước widget) nhỏ, còn lúc vẽ Kvantum dùng mảnh góc cỡ
# expansion/2 lấy từ phần tử "expand-<tên>" (rendering.cpp, Kvantum 1.1.6).
# Cần cho font Dongle: hộp dòng của nó cao ~39px ở cỡ 20, cộng thêm khung 8+8
# thì tab cao 64px (đo container 2026-10-03) thay vì ~40 như bản vẽ.
#   tên → cỡ mảnh góc khi vẽ (= expansion/2)
EXPAND = {
    "button": RADIUS["button"],
    "lineedit": RADIUS["lineedit"],
    "tooltip": RADIUS["tooltip"],
    "tab": RADIUS["tab"] + TAB_INSET,
    "itemview": RADIUS["itemview"],
    "menuitem": RADIUS["menuitem"],
    "tbutton": RADIUS["tbutton"],
}
STRETCH = 10            # cạnh/ruột vẽ dài 10px — Kvantum kéo dãn nên bao nhiêu cũng được


# ---------------------------------------------------------------- khung 9 mảnh

def _color(roles, spec):
    """'text@0.08' → (mã màu, độ đục); None → không tô."""
    if spec is None:
        return None
    name, _, alpha = spec.partition("@")
    return roles[name], float(alpha or 1)


def _paint(c):
    if c is None:
        return 'fill="none"'
    color, alpha = c
    return f'fill="{color}"' + (f' fill-opacity="{alpha:g}"' if alpha != 1 else "")


def _bounds(w, h):
    # Hình chữ nhật vô hình định khung của mảnh: QSvgRenderer lấy hộp bao của
    # phần tử để biết vẽ vùng nào — mảnh trong suốt cũng phải có kích thước.
    return f'<rect width="{w}" height="{h}" fill="#000" fill-opacity="0"/>'


def frame_element(roles, eid, f, radius, fill, stroke=None, top_only=False, inset=0):
    """9 mảnh của một khung bo góc, mỗi mảnh là <g id> có toạ độ gốc tại (0,0)
    trong hệ riêng (dời đi bởi transform của ô chứa). Trả về (chiều rộng, cao,
    danh sách (dx, dy, svg của mảnh)). `inset`: dải trong suốt cao bấy nhiêu px
    ở mép trên (chỉ dùng với top_only — tab chừa khe trên dải tab)."""
    fc, sc = _color(roles, fill), _color(roles, stroke)
    E = STRETCH
    pieces = []
    assert radius + inset <= f and (not inset or (top_only and not sc))

    def corner(name, dx, dy, mirror_x, mirror_y, r):
        # Vẽ góc trên-trái trong ô f×f rồi lật cho đủ bốn góc.
        body = []
        t = 0 if mirror_y else inset
        if fc:
            if r:
                body.append(f'<path d="M0,{f} V{t + r} A{r},{r} 0 0 1 {r},{t} H{f} V{f} Z" {_paint(fc)}/>')
            else:
                body.append(f'<rect y="{t}" width="{f}" height="{f - t}" {_paint(fc)}/>')
        if sc:
            # Viền 1px vẽ bằng HÌNH TÔ (vành giữa cung bán kính r và r−1), không
            # dùng stroke: QtSvg tính hộp bao của nét cong lớn hơn mảnh một
            # chút, Kvantum co mảnh lại cho vừa → cột/hàng mép mảnh hơi trong,
            # lộ đường mờ ở góc mọi khung (đo container 2026-10-03: 30 thay vì 23).
            color, alpha = sc
            ring = f'fill="{color}"' + (f' fill-opacity="{alpha:g}"' if alpha != 1 else "")
            if r:
                d = (f"M0,{f} V{r} A{r},{r} 0 0 1 {r},0 H{f} V1 H{r} "
                     f"A{r - 1},{r - 1} 0 0 0 1,{r} V{f} Z")
            else:
                d = f"M0,{f} V0 H{f} V1 H1 V{f} Z"
            body.append(f'<path d="{d}" {ring}/>')
        sx, sy = (-1 if mirror_x else 1), (-1 if mirror_y else 1)
        tx, ty = (f if mirror_x else 0), (f if mirror_y else 0)
        inner = "".join(body)
        svg = (f'<g id="{eid}-{name}">{_bounds(f, f)}'
               f'<g transform="matrix({sx} 0 0 {sy} {tx} {ty})">{inner}</g></g>')
        pieces.append((dx, dy, svg))

    def edge(name, dx, dy, w, h, line, t=0):
        body = [_bounds(w, h)]
        if fc:
            body.append(f'<rect y="{t}" width="{w}" height="{h - t}" {_paint(fc)}/>')
        if sc and line:
            x, y, lw, lh = line
            color, alpha = sc
            body.append(f'<rect x="{x}" y="{y}" width="{lw}" height="{lh}" fill="{color}"'
                        + (f' fill-opacity="{alpha:g}"' if alpha != 1 else "") + "/>")
        pieces.append((dx, dy, f'<g id="{eid}-{name}">{"".join(body)}</g>'))

    rb = 0 if top_only else radius
    corner("topleft", 0, 0, False, False, radius)
    corner("topright", f + E, 0, True, False, radius)
    corner("bottomleft", 0, f + E, False, True, rb)
    corner("bottomright", f + E, f + E, True, True, rb)
    edge("top", f, 0, E, f, (0, 0, E, 1), inset)
    edge("bottom", f, f + E, E, f, (0, f - 1, E, 1))
    edge("left", 0, f, f, E, (0, 0, 1, E))
    edge("right", f + E, f, f, E, (f - 1, 0, 1, E))
    edge("", f, f, E, E, None)
    # Ruột tên là đúng <tên>-<trạng thái>, không có hậu tố.
    dx, dy, svg = pieces[-1]
    pieces[-1] = (dx, dy, svg.replace(f'id="{eid}-"', f'id="{eid}"'))
    return 2 * f + E, 2 * f + E, pieces


# ---------------------------------------------------------------- phần tử

# (tên, {trạng thái: (tô, viền)}, chỉ-bo-trên). Màu ghi bằng vai trò bản tối,
# "@0.08" = độ đục. None = không tô / không viền.
FRAMES = [
    # Nút thường (OK/Cancel, combo): mặt secondary viền sub-secondary.
    # Bật/combo đang mở popup: KHÔNG viền cyan (user 2026-10-13: "thấy chỗ
    # nào có viền xanh thì bỏ") — mặt sub-secondary như hover là đủ.
    ("button", {
        "normal":   ("secondary", "sub-secondary"),
        "focused":  ("sub-secondary", "sub-secondary"),
        "pressed":  ("sub-secondary@0.6", "sub-secondary"),
        "toggled":  ("sub-secondary", "sub-secondary"),
        "disabled": ("secondary@0.5", "sub-secondary@0.5"),
    }, False),
    # HOVER (user 2026-10-05: mọi app một màu, như popup nút nguồn): trên nền
    # card → `hover` (= secondary); trên nền secondary (thanh công cụ, dải
    # tab, thanh menu) → sub-secondary, nâng một bậc. Bấm sáng hơn hover.
    # Nút trên thanh công cụ / nút + của Konsole: không nền, sáng lên khi trỏ.
    ("tbutton", {
        "normal":   (None, None),
        "focused":  ("sub-secondary", None),
        "pressed":  ("text@0.2", None),
        "toggled":  ("text@0.12", None),
        "disabled": (None, None),
    }, False),
    # Ô nhập: ô đường dẫn, ô tìm. Focus KHÔNG đổi viền (user 2026-10-07: hết
    # cyan ở ô tìm hộp thoại phím tắt / Configure Dolphin, như ô tìm Dolphin —
    # style mục 20); con trỏ chữ nhấp nháy đã báo đang gõ ở đâu.
    ("lineedit", {
        "normal":   ("card", "sub-secondary"),
        "focused":  ("card", "sub-secondary"),
        "disabled": ("card@0.5", "sub-secondary@0.5"),
    }, False),
    # Tab: tab đang mở = card liền với nội dung; tab khác trong suốt.
    ("tab", {
        "normal":   (None, None),
        "focused":  ("sub-secondary", None),
        "toggled":  ("card", None),
        "pressed":  ("card", None),
        "disabled": (None, None),
    }, True),
    # Khung quanh nội dung của QTabWidget thường (hộp thoại cài đặt…).
    ("tabframe", {"normal": (None, "sub-secondary")}, False),
    # Khung chung (QFrame, ô cuộn có khung, groupbox).
    ("common", {"normal": (None, "sub-secondary")}, False),
    # Mục trong danh sách/lưới (file trong Dolphin; Places do style "samhie" vẽ).
    # Đang chọn = nền sáng hơn ô trỏ một bậc (`sub-secondary`), KHÔNG viền
    # (user 2026-10-20 bỏ viền cyan #097389; trước nữa: viền accent), KHÔNG tô
    # `selection`: thân thư mục Breeze ăn màu chọn (#006a80) → chọn thư mục
    # là thư mục chìm vào nền (máy ảo 2026-10-04).
    ("itemview", {
        "normal":   (None, None),
        "focused":  ("hover", None),
        "pressed":  ("sub-secondary", None),
        "toggled":  ("sub-secondary", None),
        "disabled": (None, None),
    }, False),
    ("menu", {"normal": ("card", "sub-secondary")}, False),
    # Dòng menu: nền hover, chữ giữ màu thường (trước: nền selection cyan,
    # chữ trắng).
    ("menuitem", {
        "normal":   (None, None),
        "focused":  ("hover", None),
        "pressed":  ("hover", None),
        "toggled":  ("hover", None),
    }, False),
    ("menubaritem", {
        "normal":   (None, None),
        "focused":  ("sub-secondary", None),
        "pressed":  ("text@0.2", None),
        "toggled":  ("text@0.2", None),
    }, False),
    ("tooltip", {"normal": ("card", "sub-secondary")}, False),
    ("header", {
        "normal":   (None, None),
        "focused":  ("hover", None),
        "pressed":  ("sub-secondary", None),
        "toggled":  (None, None),
    }, False),
    ("scrollbargroove", {"normal": (None, None)}, False),
    # Thanh cuộn đục (không thấy nền xuyên qua), sáng lên ít khi trỏ/kéo — user 2026-10-05.
    ("scrollbarslider", {
        "normal":   ("scrollbar", None),
        "focused":  ("scrollbar-hover", None),
        "pressed":  ("scrollbar-pressed", None),
        "disabled": ("scrollbar@0.5", None),
    }, False),
    # Vạch ngăn sidebar | vùng file của Dolphin (bản vẽ: 1px #3b3c3c) — Kvantum
    # vẽ tay nắm dock bằng chính phần tử splitter (PE_IndicatorDockWidgetResizeHandle).
    # Trỏ/kéo KHÔNG đổi màu (user 2026-10-08: vạch cyan khi kéo thanh chia
    # của hộp chọn file) — con trỏ ⟺ đã báo kéo được.
    ("splitter", {"normal": ("sub-secondary", None), "focused": ("sub-secondary", None),
                  "pressed": ("sub-secondary", None)}, False),
    ("progress", {"normal": ("sub-secondary", None)}, False),
    ("progress-pattern", {"normal": ("accent", None), "disabled": ("text-muted", None)}, False),
    ("slider", {"normal": ("sub-secondary", None), "toggled": ("accent", None),
                "disabled": ("secondary", None)}, False),
]



def hugeicon(name):
    """Nét của theme/icons/hugeicons/kvantum-<name>.svg: [(thẻ, {toạ độ})], bỏ màu."""
    root = ET.parse(HUGEICONS / f"kvantum-{name}.svg").getroot()
    return [(el.tag.rpartition("}")[2], {k: v for k, v in el.attrib.items() if k in ("d", "cx", "cy", "r")})
            for el in root]


def _attrs(geo):
    return "".join(f' {k}="{v}"' for k, v in geo.items())


def _icon(eid, size, body):
    return f'<g id="{eid}">{_bounds(size, size)}{body}</g>'


def indicators(roles):
    """Phần tử nhỏ: mũi tên, ô tích, nút tròn, × đóng tab, tay nắm. Hộp 16×16."""
    out = []
    states = {"normal": "text-muted", "focused": "text", "pressed": "text",
              "toggled": "text", "disabled": "text-muted@0.4"}

    def col(spec):
        color, alpha = _color(roles, spec)
        return color, alpha

    def line(spec, d, w=1.5):
        color, alpha = col(spec)
        a = f' stroke-opacity="{alpha:g}"' if alpha != 1 else ""
        return (f'<path d="{d}" fill="none" stroke="{color}" stroke-width="{w}" '
                f'stroke-linecap="round" stroke-linejoin="round"{a}/>')

    chevrons = {"down": "M4,6 8,10 12,6", "up": "M4,10 8,6 12,10",
                "left": "M10,4 6,8 10,12", "right": "M6,4 10,8 6,12"}
    for name, d in chevrons.items():
        for st, spec in states.items():
            out.append(_icon(f"arrow-{name}-{st}", 16, line(spec, d)))
    for st, spec in states.items():
        out.append(_icon(f"arrow-plus-{st}", 16, line(spec, "M8,4 V12 M4,8 H12")))
        out.append(_icon(f"arrow-minus-{st}", 16, line(spec, "M4,8 H12")))
        # × đóng tab: bản vẽ là dấu × mảnh ~8px.
        out.append(_icon(f"tab-close-{st}", 16, line(spec, "M4.5,4.5 11.5,11.5 M11.5,4.5 4.5,11.5", 1.2)))
        out.append(_icon(f"grip-{st}", 16, line(spec, "M6,5 V11 M10,5 V11", 1)))
        out.append(_icon(f"splitter-grip-{st}", 16, ""))
        out.append(_icon(f"resize-grip-{st}", 16, line(spec, "M12,6 6,12 M12,10 10,12", 1)))

    # Ô tích + nút tròn: Hugeicons user chọn (2026-10-20), README cạnh file.
    # Đã tích = nền accent, dấu ✓ / chấm / gạch màu on-accent (tối: đen,
    # sáng: trắng). Hình 24 thu về hộp 16: viền ngoài nằm đúng 1..15 và nét
    # khung 1px trọn điểm ảnh (vuông 2.5..21.5 → tâm nét 1.5..14.5, tròn r 6.5).
    # Dấu ✓ / gạch dày 1,5px (theo tỉ lệ gốc chỉ còn 1px — mảnh, mờ trên nền
    # cyan; so ảnh 2026-10-20); chấm radio giữ nét gốc cho khỏi to.
    # radio-button vòng ngoài r 9 (circle r 10) → phóng theo vòng ngoài của
    # chính nó để tích/bỏ tích không co giãn.
    acc, on_acc, muted = roles["accent"], roles["on-accent"], roles["text-muted"]

    def ticked(name, frame, mark_fill="none"):
        shapes = hugeicon(name)
        tag, geo = shapes[0]
        s = 6.5 / (float(geo["r"]) if tag == "circle" else 9.5)
        body = f'<{tag}{_attrs(geo)} stroke-width="{1 / s:g}" fill="{frame[0]}" stroke="{frame[1]}"/>'
        for tag, geo in shapes[1:]:
            w = 1.5 / s if tag == "path" else 1.5
            body += (f'<{tag}{_attrs(geo)} stroke-width="{w:g}" fill="{mark_fill}" stroke="{on_acc}" '
                     f'stroke-linecap="round" stroke-linejoin="round"/>')
        return f'<g transform="translate(8 8) scale({s:g}) translate(-12 -12)">{body}</g>'

    for st in ("normal", "focused", "pressed", "disabled"):
        edge = muted if st != "focused" else roles["text"]
        op = ' opacity="0.4"' if st == "disabled" else ""
        for kind in ("checkbox", "radio"):
            out.append(_icon(f"{kind}-{st}", 16, f'<g{op}>{ticked(kind, ("none", edge))}</g>'))
        out.append(_icon(f"checkbox-checked-{st}", 16, f'<g{op}>{ticked("checkbox-checked", (acc, acc))}</g>'))
        out.append(_icon(f"checkbox-tristate-{st}", 16, f'<g{op}>{ticked("checkbox-tristate", (acc, acc))}</g>'))
        out.append(_icon(f"radio-checked-{st}", 16,
                         f'<g{op}>{ticked("radio-checked", (acc, acc), mark_fill=on_acc)}</g>'))
    # Núm thanh trượt: vai trò slider-* (bản sáng trắng viền xám, user
    # 2026-10-21). Vô hiệu hoá thì mờ 40% như ô tích.
    sub = roles["sub-secondary"]
    for st in ("normal", "focused", "pressed", "disabled"):
        down = "-pressed" if st in ("focused", "pressed") else ""
        op = ' opacity="0.4"' if st == "disabled" else ""
        out.append(_icon(f"slidercursor-{st}", 16,
                         f'<circle cx="8" cy="8" r="6" fill="{roles["slider-knob" + down]}" '
                         f'stroke="{roles["slider-knob-rim" + down]}"{op}/>'))
    out.append(_icon("header-separator", 16,
                     f'<rect x="7.5" y="3" width="1" height="10" fill="{sub}"/>'))
    out.append(_icon("toolbar-handle", 16, ""))
    # Không tam giác "nút mặc định" ở góc nút (OK của hộp thoại — Qt cho nút
    # đang focus làm mặc định nên tam giác chạy theo Tab; user 2026-10-07).
    # Thiếu phần tử thì Kvantum lấy của theme mặc định → khai phần tử rỗng.
    out.append(_icon("button-default-indicator", 16, ""))
    out.append(_icon("toolbar-separator", 16, f'<rect x="7.5" y="2" width="1" height="12" fill="{sub}"/>'))
    out.append(_icon("menu-separator", 16, f'<rect x="0" y="7.5" width="16" height="1" fill="{sub}"/>'))
    out.append(_icon("tree-plus", 16, line("text-muted", chevrons["right"])))
    out.append(_icon("tree-minus", 16, line("text-muted", chevrons["down"])))
    return out


def svg(pal, mode="dark"):
    roles = pal[mode]
    cells = []
    for name, states, top_only in FRAMES:
        r = RADIUS[name]
        inset = TAB_INSET if name == "tab" else 0
        for st, (fill, stroke) in states.items():
            # Bản thường: mảnh góc = cỡ khung trong kvconfig (= bán kính).
            # Bản expand-: mảnh góc = EXPAND (xem chú thích ở đó).
            cells.append(frame_element(roles, f"{name}-{st}", r + inset, r, fill, stroke,
                                       top_only, inset))
            if name in EXPAND:
                cells.append(frame_element(roles, f"expand-{name}-{st}", EXPAND[name], r,
                                           fill, stroke, top_only, inset))
    icons = indicators(roles)
    # Khung focus bàn phím: Kvantum tra phần tử "focus" KHÔNG có hậu tố trạng
    # thái, ép mép ≤ 2px và bỏ expansion (Kvantum.cpp PE_FrameFocusRect).
    cells.append(frame_element(roles, "focus", 2, 2, None, "accent@0.7"))

    # Xếp các ô thành lưới để mở file bằng Inkscape còn nhìn ra (Kvantum chỉ
    # tra theo id, vị trí không quan trọng).
    parts, x, y, row_h, width = [], 0, 0, 0, 640
    for w, h, pieces in cells:
        if x + w > width:
            x, y, row_h = 0, y + row_h + 8, 0
        for dx, dy, piece in pieces:
            parts.append(f'<g transform="translate({x + dx},{y + dy})">{piece}</g>')
        x, row_h = x + w + 8, max(row_h, h)
    x, y = 0, y + row_h + 16
    for icon in icons:
        if x + 16 > width:
            x, y = 0, y + 24
        parts.append(f'<g transform="translate({x},{y})">{icon}</g>')
        x += 24
    height = y + 24
    body = "\n".join(parts)
    return (f'<?xml version="1.0" encoding="UTF-8"?>\n<!-- {HEADER}\n'
            f'     Theme Kvantum Samhie — phần tử vẽ: xem tools/kvantum.py. -->\n'
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
            f'viewBox="0 0 {width} {height}">\n{body}\n</svg>\n')


# ---------------------------------------------------------------- kvconfig

def kvconfig(pal, mode="dark"):
    d = pal[mode]
    transparency = round((1 - pal["window"]["opacity"]) * 100)

    def frame(name, top=None, side=None):
        """Khoá frame.* của một mục. top/side: cỡ khung dùng để TÍNH kích thước
        (mặc định = bán kính); phần tử có trong EXPAND thì vẽ góc bằng expand-."""
        r = RADIUS[name]
        t = r if top is None else top
        sd = r if side is None else side
        keys = {"frame.element": name, "interior.element": name,
                "frame.top": t, "frame.bottom": t, "frame.left": sd, "frame.right": sd}
        if name in EXPAND:
            keys["frame.expansion"] = 2 * EXPAND[name]
        return keys

    # Chữ ô file đang chọn: bản tối sáng hơn chữ thường một chút (on-selection
    # trên nền sub-secondary). Bản sáng giữ `text`: on-selection là TRẮNG (cho
    # nền cyan `selection`) — trên nền xám #d0d0d0 thì gần như mất chữ.
    selected_text = d["on-selection"] if mode == "dark" else d["text"]

    text = {"text.normal.color": d["text"], "text.focus.color": d["text"],
            "text.press.color": d["text"], "text.toggle.color": d["text"],
            "text.shadow": 0}

    sections = [
        ("%General", {
            "author": "Samhie", "comment": f"Samhie — {HEADER}",
            "x11drag": "menubar_and_primary_toolbar",
            "composite": "true",
            # Nền cửa sổ trong (palette [window] opacity) — Hyprland làm mờ phía sau.
            "translucent_windows": "true",
            "reduce_window_opacity": transparency,
            "reduce_menu_opacity": 0,
            "blurring": "false", "popup_blurring": "false",
            "menu_shadow_depth": 0, "tooltip_shadow_depth": 0,
            "no_inactiveness": "true",
            "animate_states": "true",
            # Tab: dính liền nội dung, xếp từ trái, không vạch ngăn giữa tab.
            "attach_active_tab": "true", "left_tabs": "true",
            "center_doc_tabs": "false", "center_normal_tabs": "false",
            "joined_inactive_tabs": "false", "no_active_tab_separator": "true",
            "active_tab_overlap": 0, "embedded_tabs": "false",
            "spread_menuitems": "false", "spread_header": "true",
            "scroll_arrows": "false", "transient_scrollbar": "true",
            "scrollbar_in_view": "true", "scroll_width": 6, "scroll_min_extent": 36,
            "slider_width": 4, "slider_handle_width": 16, "slider_handle_length": 16,
            "check_size": 16, "progressbar_thickness": 6,
            # Phần đã chạy phủ trọn bề dày thanh. Không bật: Kvantum lấy 6 trừ
            # viền trên+dưới (3+3) = 0 → không vẽ phần đã chạy (CE_ProgressBar-
            # Contents, V1.1.6) — hộp chép file 42% chỉ thấy rãnh xám (đo
            # 2026-10-09). Thanh Free space (style mục 53) vẽ đúng cao 6 nên
            # Kvantum đã tự "spread" — giờ mọi thanh giống nó.
            "spread_progressbar": "true",
            "splitter_width": 1,
            "small_icon_size": 16, "large_icon_size": 32,
            # Icon thanh công cụ 24 như bản vẽ (icon user vẽ khung 16 nét 1 → nét
            # 1,5px ở cỡ 24; hai nút tròn vẽ khung 30 nét 1,875 — cùng tỉ lệ).
            "button_icon_size": 16, "toolbar_icon_size": 24,
            # Đệm quanh thanh công cụ (PM_ToolBarFrameWidth, mọi phía) — máy ảo
            # 2026-10-03 user thấy thanh dính sát mép trên cửa sổ.
            "toolbar_interior_spacing": 8, "toolbar_item_spacing": 4,
            "combo_as_lineedit": "true", "combo_menu": "true", "square_combo_button": "false",
            "inline_spin_indicators": "true", "vertical_spin_indicators": "false",
            "menu_separator_height": 9,
            "dialog_button_layout": 0,
            "submenu_overlap": 0,
        }),
        ("GeneralColors", {
            # Nền cửa sổ secondary (khung, thanh công cụ, sidebar, dải tab — user
            # 2026-10-03). Vùng nội dung card do style "samhie" tô (PanePainter).
            "window.color": d["secondary"], "base.color": d["card"], "alt.base.color": d["card-alt"],
            "button.color": d["secondary"], "light.color": d["sub-secondary"],
            "mid.light.color": d["sub-secondary"], "dark.color": d["bar"],
            "mid.color": d["secondary"], "shadow.color": d["bar"],
            "highlight.color": d["selection"], "inactive.highlight.color": d["selection"],
            "text.color": d["text"], "window.text.color": d["text"],
            "button.text.color": d["text"], "disabled.text.color": d["text-muted"],
            "tooltip.text.color": d["text"], "highlight.text.color": d["on-selection"],
            "link.color": d["link"], "link.visited.color": d["visited"],
            "progress.indicator.text.color": d["text"],
        }),
        ("Hacks", {
            "transparent_dolphin_view": "true",
            "transparent_ktitle_label": "true", "transparent_menutitle": "true",
            "respect_darkness": "true", "no_selection_tint": "true",
            "tint_on_mouseover": 0, "disabled_icon_opacity": 50,
            "normal_default_pushbutton": "true", "iconless_pushbutton": "false",
            "blur_translucent": "false",
            # Menu chỉ có chữ, KHÔNG chừa cột icon (user 2026-10-06: menu hẹp
            # lại, khỏi vẽ icon từng mục). Riêng ShowIconsInMenuItems=false
            # (kdeglobals) thì icon ẩn nhưng Kvantum vẫn chừa chỗ: QMenu vẫn báo
            # maxIconWidth > 0 (Kvantum.cpp 1.1.6, CE_MenuItem iconSpace).
            "iconless_menu": "true",
        }),
        # Cỡ khung trên/dưới ~0 ở mọi nút/ô: hộp dòng Dongle (~39px ở cỡ 20) đã
        # có sẵn khoảng trống trên dưới chữ — nút cao ~41 như ô đường dẫn bản vẽ.
        ("PanelButtonCommand", {
            "frame": "true", "interior": "true", **frame("button", top=1, side=8),
            "indicator.element": "arrow", "indicator.size": 12,
            **text, "text.margin.top": 0, "text.margin.bottom": 0,
            "text.margin.left": 8, "text.margin.right": 8, "text.iconspacing": 6,
        }),
        # indicator: mũi tên ⌄ nhỏ cạnh nút có menu NHẤN-GIỮ (lùi/tiến của Dolphin,
        # + của Konsole) — bản vẽ không có. Trỏ vào "tarrow" không có trong SVG,
        # cỡ 0 (Kvantum.cpp: arrowDelayed lấy indicator của chính mục này).
        # Nút menu tách đôi (MenuButtonPopup) vẫn có mũi tên — lấy ở DropDownButton.
        # Khung trên/dưới 0: nút + của Konsole là widget GÓC của dải tab — Qt cho
        # dải cao bằng widget cao nhất, nút cao hơn tab là hở một khe dưới tab
        # đang mở (đo container: nút 42 > tab 40 → khe 2px).
        ("PanelButtonTool", {"inherits": "PanelButtonCommand", **frame("tbutton", top=0, side=4),
                             "indicator.element": "tarrow", "indicator.size": 0,
                             "text.margin.top": 0, "text.margin.bottom": 0,
                             "text.margin.left": 2, "text.margin.right": 2}),
        ("ToolbarButton", {"inherits": "PanelButtonTool"}),
        ("DropDownButton", {"inherits": "PanelButtonCommand", "indicator.element": "arrow-down"}),
        ("IndicatorArrow", {"indicator.element": "arrow", "indicator.size": 12}),
        ("IndicatorSpinBox", {"inherits": "PanelButtonCommand", "indicator.element": "arrow",
                              "indicator.size": 12, "frame": "false", "interior": "false"}),
        ("ComboBox", {"inherits": "PanelButtonCommand"}),
        ("ToolbarComboBox", {"inherits": "ComboBox"}),
        ("LineEdit", {"inherits": "PanelButtonCommand", **frame("lineedit", top=1, side=10),
                      "text.margin.left": 4, "text.margin.right": 4}),
        ("ToolbarLineEdit", {"inherits": "LineEdit"}),
        ("RadioButton", {"inherits": "PanelButtonCommand", "frame": "false",
                         "interior.element": "radio"}),
        ("CheckBox", {"inherits": "PanelButtonCommand", "frame": "false",
                      "interior.element": "checkbox"}),
        ("Focus", {"frame": "true", "frame.element": "focus", "interior": "false",
                   "frame.top": 2, "frame.bottom": 2, "frame.left": 2, "frame.right": 2}),
        ("GenericFrame", {"frame": "true", "interior": "false", **frame("common"), **text}),
        ("GroupBox", {"inherits": "GenericFrame"}),
        ("TabFrame", {"inherits": "GenericFrame", "frame": "true", **frame("tabframe")}),
        # Dải tab = chính nền cửa sổ (secondary), không vẽ thêm lớp nào — nên
        # trong đúng 90% như cửa sổ, và phủ cả dưới nút + góc trái của Konsole
        # (Kvantum không vẽ nền dải tab dưới widget góc, PE_FrameTabBarBase).
        ("TabBarFrame", {"frame": "false", "interior": "false"}),
        # Tab cao 36, chữ cách mép trái 12 (bản vẽ Konsole: tab 36px trong dải 40).
        # Tab: khung trên/dưới 0 — cao = hộp dòng Dongle (~39) ≈ dải 40 của bản
        # vẽ; chữ cách mép trái 12 (= khung trái). KHÔNG đặt min_width cho tab
        # rộng 180 như bản vẽ: Kvantum 1.1.6 viết cứng chữ tab CĂN GIỮA
        # (CE_TabBarTabLabel, dòng căn trái bị chú thích bỏ) — tab ôm vừa chữ
        # nhìn giống bản vẽ hơn tab rộng có chữ ở giữa. Góc bo + khe 4px phía trên
        # vẽ bằng expand-tab-* (EXPAND).
        ("Tab", {"inherits": "PanelButtonCommand", "frame": "true", "interior": "true",
                 **frame("tab", top=0, side=12), "indicator.element": "tab",
                 "indicator.size": 16, **text,
                 "text.margin.top": 0, "text.margin.bottom": 0,
                 "text.margin.left": 0, "text.margin.right": 0}),
        ("ToolboxTab", {"inherits": "PanelButtonCommand"}),
        ("ItemView", {"inherits": "PanelButtonCommand", **frame("itemview", top=1, side=4),
                      "text.toggle.color": selected_text, "text.press.color": selected_text,
                      "text.margin.top": 0, "text.margin.bottom": 0,
                      "text.margin.left": 0, "text.margin.right": 0}),
        ("HeaderSection", {"inherits": "PanelButtonCommand", **frame("header"),
                           "text.margin.left": 6, "text.margin.right": 6}),
        ("Menu", {"inherits": "PanelButtonCommand", **frame("menu")}),
        ("MenuItem", {"inherits": "PanelButtonCommand", **frame("menuitem", top=1, side=4),
                      "text.focus.color": d["text"], "text.toggle.color": d["text"],
                      "text.press.color": d["text"], "indicator.element": "arrow",
                      "text.margin.top": 0, "text.margin.bottom": 0,
                      "text.margin.left": 4, "text.margin.right": 4}),
        ("MenuBar", {"frame": "false", "interior": "false"}),
        ("MenuBarItem", {"inherits": "PanelButtonCommand", **frame("menubaritem")}),
        ("ToolTip", {"inherits": "GenericFrame", "interior": "true",
                     **frame("tooltip", top=1, side=6),
                     "text.margin.top": 0, "text.margin.bottom": 0,
                     "text.margin.left": 6, "text.margin.right": 6}),
        ("Scrollbar", {"inherits": "PanelButtonCommand", "indicator.size": 0,
                       "frame": "false", "interior": "false"}),
        ("ScrollbarGroove", {"frame": "false", "interior": "false", **frame("scrollbargroove")}),
        ("ScrollbarSlider", {"frame": "true", "interior": "true", **frame("scrollbarslider"),
                             "indicator.element": "grip", "indicator.size": 0}),
        ("ScrollbarTransientSlider", {"inherits": "ScrollbarSlider"}),
        ("Slider", {"frame": "true", "interior": "true", **frame("slider")}),
        ("SliderCursor", {"frame": "false", "interior": "true", "interior.element": "slidercursor"}),
        ("Progressbar", {"inherits": "PanelButtonCommand", **frame("progress"), "text.margin": 0}),
        ("ProgressbarContents", {"frame": "true", "interior": "true", **frame("progress-pattern")}),
        ("TreeExpander", {"frame": "false", "interior": "false",
                          "indicator.element": "tree", "indicator.size": 12}),
        ("Splitter", {"frame": "false", "interior": "true", "interior.element": "splitter",
                      "indicator.element": "splitter-grip", "indicator.size": 0}),
        ("Toolbar", {"frame": "false", "interior": "false", "indicator.element": "toolbar",
                     "indicator.size": 0, "text.margin": 0, **text}),
        ("SizeGrip", {"frame": "false", "interior": "false",
                      "indicator.element": "resize-grip", "indicator.size": 12}),
        ("Dock", {"frame": "false", "interior": "false", **text}),
        ("DockTitle", {"frame": "false", "interior": "false", **text}),
        ("StatusBar", {"frame": "false", "interior": "false"}),
        ("TitleBar", {"frame": "false", "interior": "false", **text}),
        ("Window", {"frame": "false", "interior": "false"}),
    ]
    name = "SamhieDark" if mode == "dark" else "Samhie"
    lines = [f"# {HEADER}",
             f"# Theme Kvantum Samhie (bản {'tối' if mode == 'dark' else 'sáng'}). Phần tử vẽ: {name}.svg cùng thư mục.", ""]
    for name, keys in sections:
        lines.append(f"[{name}]")
        lines += [f"{k}={v}" for k, v in keys.items()]
        lines.append("")
    return "\n".join(lines)
