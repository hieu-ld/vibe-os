#!/usr/bin/env bash
# Menu của Konsole gọn lại (user 2026-10-09).
#
# Cùng cách với dolphin-ui.sh (đọc phần đầu file đó): Konsole nhúng
# sessionui.rc và konsoleui.rc vào libkonsoleprivate.so (Qt resource, nén
# zstd); KXmlGui ưu tiên file cùng tên trong /usr/share/kxmlgui5/konsole/ khi
# version lớn hơn. Lấy bản nhúng của đúng Konsole trong image, sửa, ghi ra —
# Konsole lên bản thì file tự theo.
#
# sessionui.rc — menu chuột phải vùng chữ (cả panel Terminal của Dolphin, F4,
# dùng chung). Chỉ XOÁ DÒNG trong <Menu name="session-popup-menu">, không ẩn
# hành động: phím tắt vẫn chạy.
#   open-browser          "Open File Manager" — trùng Open Folder With › Dolphin
#   set-encoding          "Set Encoding" — Samhie luôn UTF-8
#   adjust-history        "Adjust Scrollback…"
#   view-readonly         "Read-Only"
#   allow-mouse-tracking  "Allow Mouse Tracking" — tắt nhầm là chuột trong
#                         vim/htop thôi chạy
#   switch-profile        "Switch Profile" — Samhie một profile; "Built-in" là
#                         giao diện gốc Konsole, bấm nhầm là mất theme
#   edit-current-profile  "Create New Profile…" / "Edit Current Profile…"
# "Open Folder With" và "Menu" do mã Konsole tự chèn (không nằm trong file).
# Vạch ngăn thừa (đầu/cuối menu, hai vạch liền) bỏ theo.
#
# konsoleui.rc — cửa sổ chính: xoá phím tắt của thứ Samhie bỏ khỏi menu ☰
# (style mục 55) — chỉ phím tắt, thêm <ActionProperties>:
#   options_show_menubar  Ctrl+Shift+M  thanh menu KDE cũ, phá bố cục — và
#                         Konsole ghi nhớ nó vào ~/.config/konsolerc
#   add_bookmark          Ctrl+Shift+B  menu Bookmarks đã bỏ
#
# Không tìm thấy bản nhúng hoặc thiếu hành động nào ở trên thì build DỪNG.
set -oue pipefail

python3 - <<'PY'
import glob
import os
import re
from compression import zstd  # Python 3.14+

libs = glob.glob("/usr/lib64/libkonsoleprivate.so.*")
assert len(libs) == 1, f"expected 1 libkonsoleprivate, found {libs}"
data = open(libs[0], "rb").read()


def embedded(gui):
    found = []
    i = 0
    while (i := data.find(b"\x28\xb5\x2f\xfd", i)) >= 0:
        try:
            text = zstd.ZstdDecompressor().decompress(data[i:i + 1_000_000])
        except zstd.ZstdError:
            text = b""
        if f'<gui name="{gui}"'.encode() in text:
            found.append(text.decode())
        i += 4
    assert len(found) == 1, f'expected 1 embedded <gui name="{gui}">, found {len(found)}'
    return found[0]


def write(gui, ui, rev):
    """Ghi bản sửa. Version = bản nhúng + 1 + rev — tăng rev mỗi khi đổi file
    theo cách tài khoản đã có bản riêng cần nhận (xem dolphin-ui.sh)."""
    note = "<!-- Samhie: Konsole's own file, patched by files/scripts/konsole-ui.sh -->\n"
    ui = ui.replace("<gui ", note + "<gui ", 1)
    version = int(re.search(rf'<gui name="{gui}" version="(\d+)"', ui).group(1)) + 1 + rev
    ui, n = re.subn(rf'(<gui name="{gui}" version=")\d+"', rf'\g<1>{version}"', ui, count=1)
    assert n == 1, f"no version in {gui}"
    name = f"{gui}ui.rc"
    os.makedirs("/usr/share/kxmlgui5/konsole", exist_ok=True)
    with open(f"/usr/share/kxmlgui5/konsole/{name}", "w") as f:
        f.write(ui)
    print(f"{name} version {version}")


# ── sessionui.rc ────────────────────────────────────────────────────────────
ui = embedded("session")
# Thân menu: tới thẻ </Menu> KHỚP (bên trong còn menu con Split View).
start = ui.find('<Menu name="session-popup-menu">')
assert start >= 0, "no session-popup-menu in sessionui.rc"
start = ui.index(">", start) + 1
depth, pos = 1, start
while depth:
    tag = re.compile(r"<Menu\b|</Menu>").search(ui, pos)
    depth += 1 if tag.group(0) == "<Menu" else -1
    pos = tag.end()
end = tag.start()
body = ui[start:end]
for a in ("open-browser", "set-encoding", "adjust-history", "view-readonly",
          "allow-mouse-tracking", "switch-profile", "edit-current-profile"):
    body, n = re.subn(rf'\n\s*<Action name="{a}"\s*/>', "", body)
    assert n == 1, f'action "{a}": {n} matches in session-popup-menu'
# Vạch ngăn: không ở đầu/cuối, không hai cái liền nhau.
sep = r'\n\s*<Separator/>'
body = re.sub(rf'(?:{sep})+(?=\n\s*<Separator/>)', "", body)
body = re.sub(rf'^(?:{sep})+', "", body)
body = re.sub(rf'(?:{sep})+(?=\s*$)', "", body)
write("session", ui[:start] + body + ui[end:], rev=0)

# ── konsoleui.rc ────────────────────────────────────────────────────────────
# Chưa có <ActionProperties> — thêm trước </gui>. Tên phải là hành động có
# thật: chuỗi UTF-16 trong thư viện KF6 (KStandardAction / KBookmarkMenu).
ui = embedded("konsole")
assert "<ActionProperties" not in ui, "konsoleui.rc now has ActionProperties — merge instead"
kf = b"".join(open(f"/usr/lib64/lib{l}.so.6", "rb").read()
              for l in ("KF6ConfigGui", "KF6BookmarksWidgets"))
props = ""
for a in ("options_show_menubar", "add_bookmark"):
    assert a.encode("utf-16-le") + b"\0\0" in kf, f'unknown action "{a}"'
    props += f'\n        <Action name="{a}" shortcut=""/>'
ui, n = re.subn(r"\n</gui>", f"\n    <ActionProperties>{props}\n    </ActionProperties>\n</gui>", ui)
assert n == 1, "no </gui> in konsoleui.rc"
write("konsole", ui, rev=0)
PY
