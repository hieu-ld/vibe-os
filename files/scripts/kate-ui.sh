#!/usr/bin/env bash
# Giao diện Kate kiểu VS Code (user 2026-10-10) — phần nằm trong file menu.
#
# Cùng cách với dolphin-ui.sh / konsole-ui.sh (đọc phần đầu dolphin-ui.sh):
# Kate nhúng kateui.rc vào libkateprivate.so (Qt resource, nén zstd — có HAI
# bản giống hệt nhau); KXmlGui ưu tiên file cùng tên trong
# /usr/share/kxmlgui5/kate/ khi version LỚN HƠN. Lấy bản nhúng của đúng Kate
# trong image, sửa, ghi ra — Kate lên bản thì file tự theo.
#
# kateui.rc:
#   mainToolBar hidden="true"  bỏ thanh New/Open/Save/Save As/Undo/Redo (VS
#       Code không có). Thanh này gom hành động của Kate + KTextEditor; ẩn cả
#       thanh thay vì xoá từng hành động. Thuộc tính đọc ở KToolBar::loadState
#       (kxmlgui 6.30) — chỉ áp khi tài khoản CHƯA có trạng thái cửa sổ riêng
#       (~/.local/state/katestaterc); bật lại: chuột phải menubar.
#
# Không tìm thấy bản nhúng hoặc chỗ cần sửa thì build DỪNG.
set -oue pipefail

python3 - <<'PY'
import glob
import os
import re
from compression import zstd  # Python 3.14+

libs = glob.glob("/usr/lib64/libkateprivate.so.*")
assert len(libs) == 1, f"expected 1 libkateprivate, found {libs}"
data = open(libs[0], "rb").read()


def embedded(gui):
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


def write(gui, name, ui, rev):
    """Version = bản nhúng + 1 + rev — tăng rev mỗi khi đổi file theo cách tài
    khoản đã có bản riêng cần nhận (xem dolphin-ui.sh)."""
    note = "<!-- Samhie: Kate's own file, patched by files/scripts/kate-ui.sh -->\n"
    ui = ui.replace("<gui ", note + "<gui ", 1)
    version = int(re.search(rf'<gui name="{gui}"[^>]* version="(\d+)"', ui).group(1)) + 1 + rev
    ui, n = re.subn(rf'(<gui name="{gui}"[^>]* version=")\d+"', rf'\g<1>{version}"', ui, count=1)
    assert n == 1, f"no version in {name}"
    os.makedirs("/usr/share/kxmlgui5/kate", exist_ok=True)
    with open(f"/usr/share/kxmlgui5/kate/{name}", "w") as f:
        f.write(ui)
    print(f"{name} version {version}")


ui = embedded("kate")
ui, n = re.subn(r'<ToolBar name="mainToolBar"(?![^>]*hidden=)', '<ToolBar name="mainToolBar" hidden="true"', ui)
assert n == 1, f"mainToolBar: {n} matches in kateui.rc"
write("kate", "kateui.rc", ui, rev=0)
PY
