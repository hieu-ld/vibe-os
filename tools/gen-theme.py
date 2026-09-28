#!/usr/bin/env python3
"""Sinh cấu hình màu của mọi app từ theme/palette.toml (nguồn màu duy nhất).

    python3 tools/gen-theme.py            sinh lại mọi file
    python3 tools/gen-theme.py --check    chỉ kiểm: file sinh ra có khớp bảng
                                          màu không, tương phản có đạt không
    python3 tools/gen-theme.py --report   in bảng tương phản đầy đủ
    python3 tools/gen-theme.py --json F   ghi bảng màu + số đo ra F (để xem trước)

Chạy lúc làm việc trên máy; file sinh ra được COMMIT vào repo để diff dễ duyệt.
Build image không chạy script này — CI chạy --check để bắt file quên sinh lại.
Chỉ dùng thư viện chuẩn (Python 3.11+ vì cần tomllib).
"""

import argparse
import json
import re
import sys
import tomllib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PALETTE = ROOT / "theme" / "palette.toml"
MODES = ("dark", "light")
HEX = re.compile(r"^#[0-9a-f]{6}$")

HEADER = "SINH TỰ ĐỘNG từ theme/palette.toml bởi tools/gen-theme.py — đừng sửa tay."


# ---------------------------------------------------------------- đọc & kiểm

def load():
    with PALETTE.open("rb") as f:
        pal = tomllib.load(f)

    errors = []
    for section in ("shared", *MODES):
        for name, value in pal[section].items():
            if not isinstance(value, str) or not HEX.match(value):
                errors.append(f"[{section}] {name} = {value!r}: phải là mã '#rrggbb' chữ thường")
    for kind in ("normal", "bright"):
        colors = pal["terminal"][kind]
        if len(colors) != 8 or not all(HEX.match(c) for c in colors):
            errors.append(f"[terminal] {kind}: phải đúng 8 mã '#rrggbb'")

    # Bản tối và bản sáng phải có CÙNG bộ vai trò. Bản 1.0 học được: thiếu một
    # nhóm là app lấy màu mặc định cho nhóm đó, giao diện lem nhem hai tông.
    dark, light = set(pal["dark"]), set(pal["light"])
    if dark != light:
        errors.append(f"[dark] và [light] lệch vai trò: chỉ tối có {sorted(dark - light)}, "
                      f"chỉ sáng có {sorted(light - dark)}")
    return pal, errors


def luminance(hex_color):
    def channel(c):
        c /= 255
        return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
    r, g, b = (int(hex_color[i:i + 2], 16) for i in (1, 3, 5))
    return 0.2126 * channel(r) + 0.7152 * channel(g) + 0.0722 * channel(b)


def contrast(a, b):
    la, lb = sorted((luminance(a), luminance(b)), reverse=True)
    return (la + 0.05) / (lb + 0.05)


def measure(pal):
    """Đo mọi cặp trong [[contrast]] ở cả hai chế độ -> (kết quả, lỗi)."""
    results, errors = [], []
    for rule in pal["contrast"]:
        for mode in MODES:
            roles = pal[mode]
            for fg in rule["fg"]:
                for bg in rule["bg"]:
                    for role in (fg, bg):
                        if role not in roles:
                            errors.append(f"[[contrast]] nhắc vai trò không có: {role!r}")
                    if fg not in roles or bg not in roles:
                        continue
                    ratio = contrast(roles[fg], roles[bg])
                    ok = ratio >= rule["min"]
                    results.append(dict(mode=mode, fg=fg, bg=bg, ratio=round(ratio, 2),
                                        min=rule["min"], ok=ok))
                    if not ok:
                        errors.append(f"{mode}: {fg} {roles[fg]} trên {bg} {roles[bg]} = "
                                      f"{ratio:.2f}:1, cần {rule['min']}:1")
    return results, errors


# ---------------------------------------------------------------- sinh file

def camel(name):
    head, *rest = name.split("-")
    return head + "".join(p.capitalize() for p in rest)


def hyprland_lua(pal):
    # Hyprland ghép mã thành "rgb(...)"/"rgba(...)" nên để hex KHÔNG có '#'.
    # Hyprland chưa có chế độ sáng -> chỉ sinh bản tối.
    lines = [f"-- {HEADER}",
             "-- Màu bản tối, mã hex không có '#': dùng như \"rgba(\" .. color.accent .. \"aa)\".",
             "return {"]
    for name, value in pal["dark"].items():
        lines.append(f'    {camel(name):<13} = "{value[1:]}",')
    lines.append("}")
    return "\n".join(lines) + "\n"


def hyprlock_conf(pal):
    # hyprlang thay biến DÀI trước ($bar_text trước $bar) nên tên lồng nhau vẫn an toàn.
    lines = [f"# {HEADER}",
             "# Màu bản tối, hex không có '#': dùng rgb($card), hoặc ##$text_muted trong markup.",
             ""]
    for name, value in pal["dark"].items():
        lines.append(f"${name.replace('-', '_')} = {value[1:]}")
    return "\n".join(lines) + "\n"


OUTPUTS = {
    "files/system/usr/share/samhie/hypr/colors.lua": hyprland_lua,
    "files/system/usr/share/samhie/hypr/hyprlock-colors.conf": hyprlock_conf,
}


# ---------------------------------------------------------------- chạy

def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--check", action="store_true", help="chỉ kiểm, không ghi file")
    ap.add_argument("--report", action="store_true", help="in bảng tương phản")
    ap.add_argument("--json", metavar="FILE", help="ghi bảng màu + số đo ra FILE")
    args = ap.parse_args()

    pal, errors = load()
    results = []
    if not errors:
        results, errors = measure(pal)

    if args.report:
        for r in results:
            mark = "  " if r["ok"] else "✗ "
            print(f"{mark}{r['mode']:<5} {r['fg']:>12} / {r['bg']:<13} {r['ratio']:>5.2f}:1  (cần {r['min']})")

    if errors:
        print("Bảng màu có lỗi:", *errors, sep="\n  ", file=sys.stderr)
        return 1

    if args.json:
        data = {k: pal[k] for k in ("shared", *MODES, "terminal")}
        data["contrast"] = results
        Path(args.json).write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n")

    stale = []
    for rel, render in OUTPUTS.items():
        path = ROOT / rel
        content = render(pal)
        if path.exists() and path.read_text() == content:
            continue
        if args.check:
            stale.append(rel)
        else:
            path.write_text(content)
            print(f"đã sinh {rel}")

    if stale:
        print("File sinh ra đã cũ so với theme/palette.toml — chạy python3 tools/gen-theme.py:",
              *stale, sep="\n  ", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
