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
TERMINAL_SINGLE = ("background", "background-intense", "foreground", "foreground-faint",
                   "foreground-intense", "cursor")


# ---------------------------------------------------------------- đọc & kiểm

def load():
    with PALETTE.open("rb") as f:
        pal = tomllib.load(f)

    errors = []
    for section in ("shared", *MODES):
        for name, value in pal[section].items():
            if not isinstance(value, str) or not HEX.match(value):
                errors.append(f"[{section}] {name} = {value!r}: phải là mã '#rrggbb' chữ thường")
    for kind in ("normal", "bright", "faint"):
        colors = pal["terminal"].get(kind, [])
        if len(colors) != 8 or not all(HEX.match(c) for c in colors):
            errors.append(f"[terminal] {kind}: phải đúng 8 mã '#rrggbb'")
    for key in TERMINAL_SINGLE:
        if not HEX.match(str(pal["terminal"].get(key, ""))):
            errors.append(f"[terminal] {key}: phải là mã '#rrggbb' chữ thường")

    fonts = pal.get("fonts", {})
    if not (isinstance(fonts.get("ui"), str) and isinstance(fonts.get("mono"), str)
            and all(isinstance(fonts.get(k), int) for k in ("size", "mono-size", "ui-weight"))):
        errors.append("[fonts] cần ui, mono (chuỗi) và size, mono-size, ui-weight (số nguyên)")
    for key in ("frame-contrast", "icon-theme"):
        if set(pal.get("kde", {}).get(key, {})) != set(MODES):
            errors.append(f"[kde] {key} cần đủ hai khoá dark và light")

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


def quickshell_js(pal):
    # Thư viện JS cho QML: `import "colors.js" as Colors` rồi Colors.dark.card,
    # Colors.shared.captureFrame. .pragma library: nạp một lần, mọi file dùng chung.
    lines = [f"// {HEADER}", ".pragma library", ""]
    for section in ("shared", *MODES):
        lines.append(f"var {section} = {{")
        for name, value in pal[section].items():
            lines.append(f'    {camel(name)}: "{value}",')
        lines.append("};")
        lines.append("")
    return "\n".join(lines)


# ---------------------------------------------------------------- KDE

def rgb(hex_color):
    return ",".join(str(int(hex_color[i:i + 2], 16)) for i in (1, 3, 5))


# Bề mặt của từng nhóm màu KDE: (BackgroundNormal, BackgroundAlternate).
# Ánh xạ user chốt ở bản 1.0 (2026-09-13, chỉ bằng ảnh chụp Dolphin):
# nội dung = Card, khung (sidebar, thanh công cụ) = Secondary, mặt nút = Sub Secondary.
KDE_SURFACES = {
    "View":          ("card", "card-alt"),          # vùng file, ô path, ô nhập
    "Window":        ("secondary", "sub-secondary"),  # sidebar, thân cửa sổ
    "Header":        ("secondary", "card"),         # thanh công cụ Dolphin
    "Button":        ("sub-secondary", "selection"),
    "Tooltip":       ("card", "secondary"),         # mọi thứ nổi lên trên
    "Complementary": ("bar", "selection"),          # vùng "đối màu" (kiểu panel)
    "Selection":     ("selection", "selection-alt"),
}


def kde_group(pal, mode, group):
    r = pal[mode]
    bg, alt = KDE_SURFACES[group]
    if group == "Selection":
        # Mọi chữ trong vùng chọn dùng on-selection (bản 1.0 có thêm các sắc nhạt
        # riêng cho link/lỗi/cảnh báo trong vùng chọn — bỏ cho gọn, chữ vẫn đọc được).
        fg = dict(normal=r["on-selection"], inactive=r["on-selection-muted"],
                  active=r["on-selection"], link=r["on-selection"], visited=r["on-selection"],
                  negative=r["on-selection"], neutral=r["on-selection"], positive=r["on-selection"],
                  decoration=pal["shared"]["cyan"])
    elif group == "Complementary":
        # Nằm trên nền `bar` (luôn tối lúc này) nên chữ + màu trạng thái lấy của bản TỐI.
        d = pal["dark"]
        fg = dict(normal=r["bar-text"], inactive=d["text-muted"], active=r["bar-accent"],
                  link=d["link"], visited=d["visited"], negative=d["danger"],
                  neutral=d["warning"], positive=d["success"], decoration=r["bar-accent"])
    else:
        fg = dict(normal=r["text"], inactive=r["text-muted"], active=r["accent-text"],
                  link=r["link"], visited=r["visited"], negative=r["danger"],
                  neutral=r["warning"], positive=r["success"], decoration=r["accent"])
    return {
        "BackgroundAlternate": r[alt],
        "BackgroundNormal":    r[bg],
        "DecorationFocus":     fg["decoration"],
        "DecorationHover":     fg["decoration"],
        "ForegroundActive":    fg["active"],
        "ForegroundInactive":  fg["inactive"],
        "ForegroundLink":      fg["link"],
        "ForegroundNegative":  fg["negative"],
        "ForegroundNeutral":   fg["neutral"],
        "ForegroundNormal":    fg["normal"],
        "ForegroundPositive":  fg["positive"],
        "ForegroundVisited":   fg["visited"],
    }


def kde_color_sections(pal, mode):
    """Mọi nhóm màu của một chế độ, đúng bộ nhóm của BreezeDark. Thiếu nhóm nào
    thì app lấy màu Breeze cho nhóm đó -> giao diện lem nhem hai tông (bản 1.0)."""
    r = pal[mode]
    out = []
    # Hiệu ứng làm mờ chữ bị vô hiệu hoá / cửa sổ không focus: thông số của bản
    # 1.0 (chép từ BreezeDark), không phải màu của bảng — ColorAmount=0 và
    # Enable=false nên hai giá trị Color ở đây không hiện ra đâu cả.
    out.append(("ColorEffects:Disabled", dict(
        Color="56,56,56", ColorAmount="0", ColorEffect="0", ContrastAmount="0.65",
        ContrastEffect="1", IntensityAmount="0.1", IntensityEffect="2")))
    out.append(("ColorEffects:Inactive", dict(
        ChangeSelectionColor="true", Color="112,111,110", ColorAmount="0.025",
        ColorEffect="2", ContrastAmount="0.1", ContrastEffect="2", Enable="false",
        IntensityAmount="0", IntensityEffect="0")))
    for group in ("Button", "Complementary", "Header", "Selection", "Tooltip", "View", "Window"):
        keys = {k: rgb(v) for k, v in kde_group(pal, mode, group).items()}
        out.append((f"Colors:{group}", keys))
        if group == "Header":
            # Nền thanh công cụ KHÔNG đổi khi cửa sổ mất focus (bản 1.0 chốt).
            out.append(("Colors:Header][Inactive", keys))
    # Thanh tiêu đề: Hyprland không vẽ thanh tiêu đề cho app, nhưng hộp thoại KDE
    # và app XWayland vẫn đọc nhóm này.
    out.append(("WM", dict(
        activeBackground=rgb(r["secondary"]), activeBlend=rgb(r["text"]),
        activeForeground=rgb(r["text"]), inactiveBackground=rgb(r["secondary"]),
        inactiveBlend=rgb(r["text-muted"]), inactiveForeground=rgb(r["text-muted"]))))
    return out


def ini(sections):
    lines = []
    for name, keys in sections:
        lines.append(f"[{name}]")
        lines += [f"{k}={v}" for k, v in keys.items()]
        lines.append("")
    return "\n".join(lines)


SCHEME_NAME = {"dark": ("SamhieDark", "Samhie Dark"), "light": ("SamhieLight", "Samhie Light")}


def qt_font(family, size, weight=400):
    # Định dạng QFont::toString() của Qt 6 — đúng như KDE tự ghi vào kdeglobals.
    # Trường thứ 5 là độ đậm (300 = Light, 400 = Regular).
    return f"{family},{size},-1,5,{weight},0,0,0,0,0,0,0,0,0,0,1"


def kde_colors_file(mode):
    def render(pal):
        ident, name = SCHEME_NAME[mode]
        head = [f"# {HEADER}",
                f"# Bảng màu KDE bản {'tối' if mode == 'dark' else 'sáng'} — để đổi lúc chạy",
                "# (Dolphin/Konsole: Settings → Window Color Scheme). Mặc định toàn máy nằm",
                "# ở /etc/xdg/kdeglobals, cũng sinh từ palette.toml.", ""]
        sections = kde_color_sections(pal, mode) + [
            ("General", dict(ColorScheme=ident, Name=name, shadeSortColumn="true")),
            ("KDE", dict(contrast="4", frameContrast=str(pal["kde"]["frame-contrast"][mode]))),
        ]
        return "\n".join(head) + ini(sections)
    return render


def kdeglobals(pal):
    # Mặc định TOÀN MÁY cho app KDE (plasma-integration đọc cả /etc/xdg). Người
    # dùng đè từng khoá trong ~/.config/kdeglobals. Phải chép hẳn các nhóm
    # [Colors:*] vào đây: KDE đọc màu thật từ chúng, còn ColorScheme= chỉ là
    # cái nhãn ghi tên (bản 1.0 đã vấp).
    f = pal["fonts"]
    ident, _ = SCHEME_NAME["dark"]
    head = [f"# {HEADER}",
            "# Mặc định toàn máy cho app KDE (Dolphin, Konsole…). Đè riêng trong",
            "# ~/.config/kdeglobals — file này không bao giờ ghi vào đó.", ""]
    sections = [
        ("General", dict(
            ColorScheme=ident,
            font=qt_font(f["ui"], f["size"], f["ui-weight"]),
            menuFont=qt_font(f["ui"], f["size"], f["ui-weight"]),
            toolBarFont=qt_font(f["ui"], f["size"], f["ui-weight"]),
            smallestReadableFont=qt_font(f["ui"], f["size"] - 2, f["ui-weight"]),
            fixed=qt_font(f["mono"], f["mono-size"]),
            shadeSortColumn="true",
        )),
        ("Icons", dict(Theme=pal["kde"]["icon-theme"]["dark"])),
        ("KDE", dict(contrast="4", frameContrast=str(pal["kde"]["frame-contrast"]["dark"]),
                     SingleClick="false")),
    ] + kde_color_sections(pal, "dark")
    return "\n".join(head) + ini(sections)


# ---------------------------------------------------------------- Konsole

def konsole_colorscheme(pal):
    # Konsole KHÔNG đọc màu KDE — nó có bảng riêng. Thanh tab, menu, viền là
    # khung Qt, vẫn lấy màu từ kdeglobals; file này chỉ tô vùng chữ.
    t = pal["terminal"]
    head = [f"# {HEADER}",
            "# Bảng màu vùng chữ của Konsole. Profile Samhie.profile chọn nó.", ""]
    sections = [
        ("Background", dict(Color=rgb(t["background"]))),
        ("BackgroundFaint", dict(Color=rgb(t["background"]))),
        ("BackgroundIntense", dict(Color=rgb(t["background-intense"]))),
    ]
    for i in range(8):
        sections += [
            (f"Color{i}", dict(Color=rgb(t["normal"][i]))),
            (f"Color{i}Faint", dict(Color=rgb(t["faint"][i]))),
            (f"Color{i}Intense", dict(Color=rgb(t["bright"][i]))),
        ]
    sections += [
        ("Foreground", dict(Color=rgb(t["foreground"]))),
        ("ForegroundFaint", dict(Color=rgb(t["foreground-faint"]))),
        ("ForegroundIntense", dict(Color=rgb(t["foreground-intense"]))),
        # Nền đục: nền hơi trong để ăn blur của Hyprland là việc giai đoạn 4.
        ("General", dict(Blur="false", ColorRandomization="false", Description="Samhie",
                         Opacity="1", Wallpaper="")),
    ]
    return "\n".join(head) + ini(sections)


def konsole_profile(pal):
    # Profile mặc định (chọn ở /etc/xdg/konsolerc). Người dùng sửa profile trong
    # Konsole thì Konsole lưu bản riêng vào ~/.local/share/konsole — file này
    # không bị đụng tới, bản vá sau của Samhie vẫn tới được người chưa sửa.
    f, t = pal["fonts"], pal["terminal"]
    head = [f"# {HEADER}",
            "# Profile Konsole mặc định của Samhie (bản 1.0: theme/konsole/Samhie.profile).", ""]
    sections = [
        # BoldIntense = "vẽ chữ đậm bằng FONT đậm" (bản 1.0 hiểu ngược). false
        # thì chữ đậm vẫn mảnh — dấu nhắc đậm ra đúng số pixel như nét thường
        # (đo máy ảo 2026-09-30). Việc đổi màu 8-ô đậm sang Intense thì Konsole
        # làm bất kể khoá này; dấu nhắc tránh bằng màu RGB (samhie-prompt.sh).
        ("Appearance", dict(BoldIntense="true", ColorScheme="Samhie",
                            Font=qt_font(f["mono"], f["mono-size"]), LineSpacing="2",
                            UseFontLineChararacters="true")),
        ("Cursor Options", dict(CursorShape="0", CustomCursorColor=rgb(t["cursor"]),
                                UseCustomCursorColor="true")),
        # %d = tên thư mục hiện tại: bù cho phần đường dẫn đã bỏ khỏi dấu nhắc
        # (user chốt 2026-09-28).
        ("General", dict(LocalTabTitleFormat="%d", Name="Samhie", Parent="FALLBACK/",
                         TerminalMargin="10")),
        ("Scrolling", dict(HighlightScrolledLines="false", HistorySize="10000",
                           ScrollBarPosition="2")),
        ("Terminal Features", dict(BlinkingCursorEnabled="true")),
    ]
    return "\n".join(head) + ini(sections)


def bash_prompt(pal):
    r, g, b = rgb(pal["terminal"]["normal"][6]).split(",")
    return f"""# shellcheck shell=bash
# {HEADER}
# Dấu nhắc lệnh của Samhie: chỉ còn "$ " màu cyan (user chốt 2026-09-28: bỏ cả
# user@host lẫn đường dẫn — tên tab Konsole hiện thư mục hiện tại thay, xem
# LocalTabTitleFormat trong /usr/share/konsole/Samhie.profile).
#
# \\$ chứ không phải chữ $: bash in "$" cho người thường và "#" cho root. Đăng
# nhập root mà vẫn thấy "$" là một cách hỏng nguy hiểm (bản 1.0).
#
# Màu = Color6 của [terminal] trong theme/palette.toml, IN ĐẬM (user thấy nét
# thường mảnh quá, 2026-09-30). Đậm + màu 8-ô (01;36) thì Konsole 26.08 đổi sang
# Color6Intense (đo pixel ra #52d6fa). Nên terminal nào báo có màu 24-bit
# (COLORTERM=truecolor, Konsole có) thì ghi thẳng mã RGB: màu RGB không bị
# đổi. Nét đậm cần BoldIntense=true trong Samhie.profile. Terminal khác
# (tty…) dùng 36 thường, không đậm.
#
# Thứ tự: /etc/profile.d chạy theo tên file, nên file này chạy SAU
# bash-color-prompt.sh (Fedora) và đè PS1 của nó; nhưng giữ tiền tố/hậu tố mà
# 70-systemd-shell-extra.sh gắn vào (vd run0 báo đang là root). Người dùng muốn
# dấu nhắc khác thì đặt PS1 trong ~/.bashrc — file đó chạy sau cùng.

if [ -n "${{BASH_VERSION-}}" ] && [ -n "${{PS1-}}" ]; then
    case ${{COLORTERM-}}:${{TERM:-dumb}} in
        *:dumb)               PS1='\\$ ' ;;
        truecolor:*|24bit:*)  PS1='\\[\\e[1;38;2;{r};{g};{b}m\\]\\$\\[\\e[0m\\] ' ;;
        *)                    PS1='\\[\\e[36m\\]\\$\\[\\e[0m\\] ' ;;
    esac
    PS1="${{SHELL_PROMPT_PREFIX-}}$PS1${{SHELL_PROMPT_SUFFIX-}}"

    # Một dòng trống giữa các lệnh (user 2026-09-30: nhìn thoáng; terminal là
    # lưới ký tự nên "khoảng" nhỏ nhất là nguyên một dòng — lệnh và phần trả
    # về của nó vẫn sát nhau). Chỉ khi VỪA chạy lệnh: PS0 hiện ra lúc bash bắt
    # đầu chạy một lệnh (Enter trên dòng trống thì không); ${{ …; }} của bash 5.3
    # chạy ngay trong shell mà không in gì, nên đặt cờ được. (\\# thì không
    # dùng được: bash không tăng nó sau lệnh ĐẦU TIÊN — đo 2026-09-30.)
    # clear/reset xoá cờ, khỏi mở màn hình mới bằng một dòng trống.
    # Nối vào PS0 / PROMPT_COMMAND có sẵn: systemd (80-systemd-osc-context.sh)
    # đã gắn vào hai chỗ này trước và đọc $? trước mình.
    if (( BASH_VERSINFO[0] > 5 || (BASH_VERSINFO[0] == 5 && BASH_VERSINFO[1] >= 3) )); then
        PS0="${{PS0-}}"'${{ __samhie_ran=1; }}'
        __samhie_gap() {{
            if [ -n "${{__samhie_ran-}}" ]; then
                printf '\\n'
                unset __samhie_ran
            fi
        }}
        PROMPT_COMMAND+=(__samhie_gap)
        clear() {{ command clear "$@"; unset __samhie_ran; }}
        reset() {{ command reset "$@"; unset __samhie_ran; }}
    fi
fi
"""


# ---------------------------------------------------------------- Plymouth

def plymouth_theme(pal):
    # Màn hình khởi động luôn TỐI, kể cả khi người dùng chọn bản sáng: nó hiện
    # trước khi có tài khoản nào, và nối tiếp màn hình đen của firmware.
    # Plymouth ghi màu dạng 0xrrggbb.
    def c(hex_color):
        return "0x" + hex_color[1:]
    bg = c(pal["shared"]["black"])
    d = pal["dark"]
    modes = [("updates", "Installing Updates..."),
             ("system-upgrade", "Upgrading System..."),
             ("firmware-upgrade", "Upgrading Firmware..."),
             ("system-reset", "Resetting System...")]
    lines = [
        f"# {HEADER}",
        "#",
        "# Màn hình khởi động của Samhie. Như bản 1.0: module \"two-step\" có sẵn",
        "# của Plymouth (lo sẵn ô nhập mật khẩu ổ mã hoá, đèn Caps Lock, thông báo",
        "# lỗi) — cố ý KHÔNG tự viết theme kiểu script, viết sai là máy mã hoá ổ",
        "# không nhập được mật khẩu, tức không boot nổi.",
        "#",
        "# Ảnh trong thư mục này:",
        f"#   throbber-*.png  5 chấm màu {d['text']} (dark.text) — sinh bằng",
        "#                   tools/build-plymouth-throbber.py (cần Pillow), đổi",
        "#                   dark.text thì chạy lại script đó",
        "#   watermark.png   chữ Samhie, cao 64px — bản 1.0 (branding/watermark.png)",
        "#   entry, bullet, lock, capslock, keyboard, keymap-render: ô mật khẩu,",
        "#                   chép từ theme spinner của Fedora lúc build",
        "#                   (files/scripts/plymouth.sh)",
        "#",
        "# Font: trong initramfs chỉ có plugin chữ label-freetype, nó dùng font",
        "# MẶC ĐỊNH của hệ thống (fc-match không tham số = Noto Sans) và bỏ qua tên",
        "# font ghi ở đây, chỉ lấy cỡ chữ. Ghi đúng Noto Sans để lúc tắt máy (có",
        "# label-pango, đọc tên font) ra cùng một kiểu chữ.",
        "",
        "[Plymouth Theme]",
        "Name=Samhie",
        "Description=Samhie boot screen",
        "ModuleName=two-step",
        "",
        "[two-step]",
        "Font=Noto Sans 12",
        "TitleFont=Noto Sans 30",
        "ImageDir=/usr/share/plymouth/themes/samhie",
        "DialogHorizontalAlignment=.5",
        "DialogVerticalAlignment=.382",
        "TitleHorizontalAlignment=.5",
        "TitleVerticalAlignment=.382",
        "HorizontalAlignment=.5",
        "VerticalAlignment=.7",
        "WatermarkHorizontalAlignment=.5",
        "WatermarkVerticalAlignment=.96",
        "Transition=none",
        "TransitionDuration=0.0",
        "# Nền phẳng Black (hai đầu gradient cùng một màu), như bản 1.0.",
        f"BackgroundStartColor={bg}",
        f"BackgroundEndColor={bg}",
        "# Thanh tiến trình (chỉ hiện lúc cài cập nhật firmware…): rãnh",
        "# sub-secondary, phần chạy accent.",
        f"ProgressBarBackgroundColor={c(d['sub-secondary'])}",
        f"ProgressBarForegroundColor={c(d['accent'])}",
        "MessageBelowAnimation=true",
        "",
        "# Tắt end animation ở mọi chế độ nên không cần ảnh animation-*. Đừng bật:",
        "# thiếu ảnh đó thì two-step lấy throbber-* làm end animation và BỎ vòng",
        "# xoay (đọc plugin.c, Plymouth 24.004.60).",
        "[boot-up]",
        "UseEndAnimation=false",
        "",
        "[shutdown]",
        "UseEndAnimation=false",
        "",
        "[reboot]",
        "UseEndAnimation=false",
    ]
    for mode, title in modes:
        lines += ["", f"[{mode}]", "SuppressMessages=true", "ProgressBarShowPercentComplete=true",
                  "UseProgressBar=true", f"Title={title}", "SubTitle=Do not turn off your computer"]
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------- fcitx5

# Theme của classicui — thứ vẽ ô "Unikey"/"English" hiện lên mỗi lần Ctrl+Space
# (ShowInputMethodInformation, mặc định bật) và bảng chọn chữ của emoji,
# quickphrase, clipboard (Ctrl+;). Khoá lấy từ src/ui/classic/theme.h của
# fcitx5 5.1.22 (bản trong image); bố cục chép theo theme default-dark có sẵn.
# fcitx5 KHÔNG có khoá bo góc — muốn bo thì nền phải là ảnh SVG (việc sau).
FCITX_THEME_DIR = "files/system/usr/share/fcitx5/themes/Samhie"


def fcitx_theme(pal):
    d = pal["dark"]
    margin = lambda n: dict(Left=n, Right=n, Top=n, Bottom=n)
    # Màu viền sau cùng; BorderWidth phải nhỏ hơn Margin (theme.h ghi rõ).
    panel = lambda: dict(Color=d["card"], BorderColor=d["sub-secondary"], BorderWidth="1")
    head = [f"# {HEADER}",
            "# Theme fcitx5 của Samhie (bản tối). /etc/xdg/fcitx5/conf/classicui.conf chọn nó.", ""]
    sections = [
        ("Metadata", dict(Name="Samhie", Version="1", Author="Samhie",
                          Description="Samhie dark theme", ScaleWithDPI="True")),
        ("InputPanel", dict(NormalColor=d["text"], HighlightCandidateColor=d["on-selection"],
                            HighlightColor=d["on-selection"],
                            HighlightBackgroundColor=d["selection"],
                            FullWidthHighlight="True", PageButtonAlignment="Last Candidate")),
        ("InputPanel/TextMargin", margin(5)),
        ("InputPanel/ContentMargin", margin(2)),
        ("InputPanel/Background", panel()),
        ("InputPanel/Background/Margin", margin(2)),
        ("InputPanel/Highlight", dict(Color=d["selection"])),
        ("InputPanel/Highlight/Margin", margin(5)),
        ("InputPanel/PrevPage", dict(Image="prev.svg")),
        ("InputPanel/PrevPage/ClickMargin", dict(Left=5, Right=5, Top=4, Bottom=4)),
        ("InputPanel/NextPage", dict(Image="next.svg")),
        ("InputPanel/NextPage/ClickMargin", dict(Left=5, Right=5, Top=4, Bottom=4)),
        # Menu chỉ hiện với khay XEmbed (X11); trên Hyprland menu khay do thanh
        # trên vẽ. Vẫn khai cho đủ, khỏi lọt màu của theme default.
        ("Menu", dict(NormalColor=d["text"], HighlightCandidateColor=d["on-selection"])),
        ("Menu/Background", panel()),
        ("Menu/Background/Margin", margin(2)),
        ("Menu/ContentMargin", margin(2)),
        ("Menu/CheckBox", dict(Image="radio.svg")),
        ("Menu/SubMenu", dict(Image="arrow.svg")),
        ("Menu/Highlight", dict(Color=d["selection"])),
        ("Menu/Highlight/Margin", margin(5)),
        ("Menu/Separator", dict(Color=d["sub-secondary"])),
        ("Menu/TextMargin", margin(5)),
    ]
    return "\n".join(head) + ini(sections)


def fcitx_svg(width, height, shape):
    # Mũi tên/chấm của theme: cùng kích thước và hình như theme default-dark,
    # chỉ đổi màu. Toạ độ tính bằng px.
    def render(pal):
        return (f'<!-- {HEADER} -->\n'
                f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
                f'viewBox="0 0 {width} {height}">\n'
                f'  {shape.format(fill=pal["dark"]["text-muted"])}\n</svg>\n')
    return render


def fcitx_classicui(pal):
    # Cấu hình classicui toàn máy. fcitx5 tìm conf/classicui.conf ở ~/.config/fcitx5
    # rồi mới tới $XDG_CONFIG_DIRS/fcitx5 (như `profile` bên cạnh) — người dùng
    # đổi trong fcitx5-configtool thì bản ~/.config của họ thắng file này.
    # fcitx5-qt (app Qt) thì CHỈ đọc bản ~/.config → user-setup chép file này
    # sang đó ở lần đăng nhập đầu (xem bước 0b trong user-setup).
    # Định dạng: khoá nằm thẳng ở gốc, không có nhóm (classicui.h).
    f = pal["fonts"]
    # Chuỗi font kiểu Pango: "Họ Kiểu Cỡ". Dongle chỉ có tên kiểu Light/Regular/Bold.
    weight = {300: "Light", 400: "", 700: "Bold"}[f["ui-weight"]]
    font = " ".join(p for p in (f["ui"], weight, str(f["size"])) if p)
    lines = [f"# {HEADER}",
             "# Giao diện fcitx5 (addon classicui): theme Samhie + font giao diện.",
             "# Bản trong ~/.config/fcitx5/conf là bản CHÉP của riêng bạn — sửa thoải mái.",
             f'Font="{font}"', f'MenuFont="{font}"',
             # Cùng một theme cho cả hai chế độ: Samhie chưa có bản sáng cho fcitx5.
             "Theme=Samhie", "DarkTheme=Samhie"]
    return "\n".join(lines) + "\n"


OUTPUTS = {
    "files/system/usr/share/samhie/hypr/colors.lua": hyprland_lua,
    "files/system/usr/share/samhie/hypr/hyprlock-colors.conf": hyprlock_conf,
    "files/system/usr/share/samhie/quickshell/colors.js": quickshell_js,
    "files/system/usr/share/konsole/Samhie.colorscheme": konsole_colorscheme,
    "files/system/usr/share/konsole/Samhie.profile": konsole_profile,
    "files/system/etc/profile.d/samhie-prompt.sh": bash_prompt,
    "files/system/etc/xdg/kdeglobals": kdeglobals,
    "files/system/usr/share/color-schemes/SamhieDark.colors": kde_colors_file("dark"),
    "files/system/usr/share/color-schemes/SamhieLight.colors": kde_colors_file("light"),
    "files/system/usr/share/plymouth/themes/samhie/samhie.plymouth": plymouth_theme,
    "files/system/etc/xdg/fcitx5/conf/classicui.conf": fcitx_classicui,
    f"{FCITX_THEME_DIR}/theme.conf": fcitx_theme,
    f"{FCITX_THEME_DIR}/prev.svg": fcitx_svg(16, 20, '<path fill="{fill}" d="M11 16 5 10l6-6z"/>'),
    f"{FCITX_THEME_DIR}/next.svg": fcitx_svg(16, 20, '<path fill="{fill}" d="m5 4 6 6-6 6z"/>'),
    f"{FCITX_THEME_DIR}/arrow.svg": fcitx_svg(6, 12, '<path fill="{fill}" d="m0 0 6 6-6 6z"/>'),
    f"{FCITX_THEME_DIR}/radio.svg": fcitx_svg(24, 24, '<circle fill="{fill}" cx="12" cy="12" r="4"/>'),
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
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content)
            print(f"đã sinh {rel}")

    if stale:
        print("File sinh ra đã cũ so với theme/palette.toml — chạy python3 tools/gen-theme.py:",
              *stale, sep="\n  ", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
