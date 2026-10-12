#!/usr/bin/env bash
# Giao diện Kate kiểu VS Code (user 2026-10-10) — phần nằm trong file menu.
#
# Cùng cách với dolphin-ui.sh / konsole-ui.sh (đọc phần đầu dolphin-ui.sh):
# file menu nhúng trong thư viện (Qt resource, nén zstd — có khi HAI bản giống
# hệt nhau); KXmlGui ưu tiên file cùng tên trong /usr/share/kxmlgui5/<component>/
# khi version LỚN HƠN. Lấy bản nhúng của đúng bản trong image, sửa, ghi ra —
# Kate/KTextEditor lên bản thì file tự theo.
#
# kateui.rc (libkateprivate.so, component "kate"):
#   mainToolBar hidden="true"  bỏ thanh New/Open/Save/Save As/Undo/Redo (VS
#       Code không có). Thanh này gom hành động của Kate + KTextEditor; ẩn cả
#       thanh thay vì xoá từng hành động. Thuộc tính đọc ở KToolBar::loadState
#       (kxmlgui 6.30) — chỉ áp khi tài khoản CHƯA có trạng thái cửa sổ riêng
#       (~/.local/state/katestaterc); bật lại: chuột phải menubar.
#
# katepart5ui.rc (libKF6TextEditor.so, component "katepart" — vùng code của
# Kate; KWrite cũng dùng): menu chuột phải trong vùng code (<Menu
# name="ktexteditor_popup">) bỏ Copy as HTML, Paste Selection (dán chữ đang bôi
# đen ở chỗ khác — kiểu chuột giữa X11), Swap with Clipboard Contents, Text to
# Speech + Say current selection (rà Kate đợt 1, user 2026-10-12). Chỉ bỏ khỏi
# menu chuột phải: hành động vẫn còn trong menu ☰ › Edit, phím tắt vẫn chạy.
#
# Không tìm thấy bản nhúng hoặc chỗ cần sửa thì build DỪNG.
set -oue pipefail

python3 - <<'PY'
import glob
import os
import re
from compression import zstd  # Python 3.14+

def embedded(lib_glob, gui):
    libs = glob.glob(lib_glob)
    assert len(libs) == 1, f"expected 1 {lib_glob}, found {libs}"
    data = open(libs[0], "rb").read()
    found = set()
    i = 0
    while (i := data.find(b"\x28\xb5\x2f\xfd", i)) >= 0:
        try:
            text = zstd.ZstdDecompressor().decompress(data[i:i + 1_000_000])
        except zstd.ZstdError:
            text = b""
        if f'<gui name="{gui}"'.encode() in text:
            found.add(text.decode())
        i += 4
    assert len(found) == 1, f'expected 1 distinct embedded <gui name="{gui}">, found {len(found)}'
    return found.pop()


def write(component, gui, name, ui, rev):
    """Version = bản nhúng + 1 + rev — tăng rev mỗi khi đổi file theo cách tài
    khoản đã có bản riêng cần nhận (xem dolphin-ui.sh)."""
    note = "<!-- Samhie: Kate's own file, patched by files/scripts/kate-ui.sh -->\n"
    ui = ui.replace("<gui ", note + "<gui ", 1)
    version = int(re.search(rf'<gui name="{gui}"[^>]* version="(\d+)"', ui).group(1)) + 1 + rev
    ui, n = re.subn(rf'(<gui name="{gui}"[^>]* version=")\d+"', rf'\g<1>{version}"', ui, count=1)
    assert n == 1, f"no version in {name}"
    os.makedirs(f"/usr/share/kxmlgui5/{component}", exist_ok=True)
    with open(f"/usr/share/kxmlgui5/{component}/{name}", "w") as f:
        f.write(ui)
    print(f"{component}/{name} version {version}")


ui = embedded("/usr/lib64/libkateprivate.so.*", "kate")
ui, n = re.subn(r'<ToolBar name="mainToolBar"(?![^>]*hidden=)', '<ToolBar name="mainToolBar" hidden="true"', ui)
assert n == 1, f"mainToolBar: {n} matches in kateui.rc"
write("kate", "kate", "kateui.rc", ui, rev=0)

ui = embedded("/usr/lib64/libKF6TextEditor.so.6.*", "KatePartView")
popup = re.search(r'<Menu name="ktexteditor_popup".*?\n</Menu>', ui, re.S)
assert popup, "no ktexteditor_popup in katepart5ui.rc"
menu = popup.group(0)
for action in ("edit_copy_html", "edit_paste_selection", "edit_swap_with_clipboard", "tools_speech_say"):
    menu, n = re.subn(rf'\n\s*<Action name="{action}"[^>]*/>', "", menu)
    assert n == 1, f"{action}: {n} matches in ktexteditor_popup"
menu, n = re.subn(r'\n\s*<Menu name="speech".*?</Menu>', "", menu, flags=re.S)
assert n == 1, f"speech: {n} matches in ktexteditor_popup"
# vạch ngăn trước mục Text to Speech giờ đứng cuối menu
menu = re.sub(r'\n\s*<Separator[^>]*/>(\n</Menu>)$', r"\1", menu)
ui = ui.replace(popup.group(0), menu)
write("katepart", "KatePartView", "katepart5ui.rc", ui, rev=0)
PY
