#!/usr/bin/env bash
# Giao diện Dolphin của Samhie: bỏ hẳn chế độ Compact, Details = Ctrl+2, bấm
# nút chế độ xem là mở menu. Giống bản 1.0 (user thử đạt 2026-09-22).
#
# Dolphin nhúng dolphinui.rc vào chính file chạy (Qt resource, nén zstd), và
# KXmlGui ưu tiên file cùng tên trong /usr/share/kxmlgui5/dolphin/ khi số
# version BẰNG bản nhúng — file hệ thống đứng trước trong danh sách, mà chỉ
# version LỚN HƠN mới giành được chỗ (KXmlGuiVersionHandler, kxmlgui 6.30).
# Nên ở đây LẤY RA bản nhúng của đúng Dolphin trong image, sửa ba thuộc tính
# trong <ActionProperties>, ghi vào /usr/share. Không chép tay file của Dolphin
# vào repo: Dolphin lên bản thì file tự theo, menu mới của Dolphin không mất.
#
# KXmlGui áp MỌI thuộc tính trong <ActionProperties> qua QObject::setProperty,
# không riêng phím tắt:
#   compact        visible="false" shortcut=""   ẩn khỏi mọi menu, Ctrl+2 cũ chết theo
#   details        shortcut="Ctrl+2"             Icons = Ctrl+1, Details = Ctrl+2
#   view_settings  popupMode="InstantPopup"      mặc định nút đôi: bấm icon thì
#                  XOAY VÒNG icons → compact → details (viết cứng trong
#                  DolphinViewActionHandler), ẩn Compact thôi là chưa đủ.
#   toggle_selection_mode  visible="false" shortcut=""   (thêm mới vào
#                  ActionProperties) bỏ Selection Mode khỏi menu ☰ và phím
#                  Space: hai thanh xanh lá, chọn bằng bấm từng ô — trùng với
#                  Ctrl/Shift+bấm, Ctrl+A (user 2026-10-04).
#   Bớt menu (user 2026-10-05) — cũng visible="false" (hành động ẩn thì phím
#   tắt của nó cũng chết; bốn cái này không có phím tắt):
#   view_properties  "Adjust View Display Style…" (menu ▦): Samhie dùng chung
#                    một kiểu xem cho mọi thư mục (GlobalViewProps mặc định
#                    true) → hộp thoại chỉ lặp lại các mục có sẵn trong ▦.
#   group_by         "Group By" (▦).
#   additional_info  "Show Additional Information" (▦): dòng chữ dưới tên file
#                    ở Icons; Details chọn cột bằng chuột phải tiêu đề cột.
#   basic_actions    "Actions for Current View" (☰): Cut/Copy/Paste/Select
#                    All… trùng menu chuột phải; phím tắt vẫn chạy (ẩn MENU
#                    con, không ẩn từng hành động).
#
# Thanh công cụ theo bản vẽ của user (2026-10-03):
#   ← → | ▦  [đường dẫn ✎]  🔍  ☰
# = go_back go_forward | view_settings url_navigators toggle_search
#   hamburger_menu. Bỏ nút ⧉ copy_location (máy ảo 2026-10-04): nó chỉ chép
#   đường dẫn MỤC ĐANG CHỌN, chưa chọn gì thì bật Selection Mode — không phải
#   "chép đường dẫn thư mục đang mở" như bản vẽ, mà Dolphin không có hành động
#   đó; vẫn còn ở menu chuột phải + Ctrl+Alt+C. Không có nút lọc ⧩ riêng: thanh tìm của
#   Dolphin đã có nút Filter (user bỏ 2026-10-03). Nút ✎ (sửa đường dẫn) là nút
# CÓ SẴN ở cuối thanh đường dẫn (KUrlNavigator, icon edit-entry) — thêm hành
# động editable_location nữa là hai cây bút (máy ảo 2026-10-03).
# Lùi/tiến dùng icon tròn riêng samhie-go-back/forward (theme/icons).
# Bỏ split_view/split_stash khỏi thanh (vẫn còn F3 và menu ☰). Thanh gốc khác
# với danh sách DOLPHIN_TOOLBAR bên dưới thì build dừng — Dolphin đổi thanh
# công cụ thì xem lại bản vẽ trước khi chép đè.
#
# Không tìm thấy bản nhúng hoặc thiếu hành động nào ở trên thì build DỪNG —
# thà biết ngay còn hơn Dolphin lặng lẽ quay về mặc định.
set -oue pipefail

python3 - <<'EOF'
import os
import re
from compression import zstd  # Python 3.14+

data = open("/usr/bin/dolphin", "rb").read()

# Qt rcc lưu mỗi file nén thành một khung zstd (mở đầu 28 B5 2F FD). Dolphin
# nhúng hai file <gui name="dolphin">: dolphinui.rc và dolphinuiforphones.rc
# (giao diện điện thoại, thanh công cụ ở đáy) — phân biệt bằng thanh địa chỉ
# url_navigators, chỉ thanh công cụ bản máy tính có.
found = []
i = 0
while (i := data.find(b"\x28\xb5\x2f\xfd", i)) >= 0:
    try:
        text = zstd.ZstdDecompressor().decompress(data[i:i + 1_000_000])
    except zstd.ZstdError:
        text = b""
    if b'<gui name="dolphin"' in text and b'name="url_navigators"' in text:
        found.append(text.decode())
    i += 4
assert len(found) == 1, f"expected 1 embedded dolphinui.rc, found {len(found)}"
ui = found[0]

props = {
    "compact": ' shortcut="" visible="false"',
    "details": ' shortcut="Ctrl+2"',
    "view_settings": ' popupMode="InstantPopup"',
}
head, sep, tail = ui.partition("<ActionProperties")
assert sep, "no <ActionProperties> in dolphinui.rc"
for name, attrs in props.items():
    tail, n = re.subn(rf'(<Action\b[^>]*\bname="{name}")', rf"\1{attrs}", tail)
    assert n == 1, f'action "{name}": {n} matches in <ActionProperties>'
ui = head + sep + tail

DOLPHIN_TOOLBAR = ["go_back", "go_forward", "view_settings", "url_navigators",
                   "split_view", "split_stash", "toggle_search", "hamburger_menu"]
SAMHIE_TOOLBAR = ["go_back", "go_forward", None, "view_settings", "url_navigators",
                  "toggle_search", "hamburger_menu"]   # None = vạch ngăn
bar = re.search(r'(<ToolBar\b[^>]*name="mainToolBar"[^>]*>\s*<text[^<]*</text>)(.*?)(\s*</ToolBar>)',
                ui, re.S)
assert bar, "no mainToolBar in dolphinui.rc"
found = re.findall(r'<Action name="([^"]+)"', bar.group(2))
assert found == DOLPHIN_TOOLBAR, f"Dolphin's toolbar changed: {found}"
# Tên mới thêm phải là hành động có thật của Dolphin: có trong menu của file.
rest = ui[:bar.start()] + ui[bar.end():]
for a in set(filter(None, SAMHIE_TOOLBAR)) - set(DOLPHIN_TOOLBAR):
    assert f'name="{a}"' in rest, f'unknown action "{a}"'
items = "".join("\n        <Separator/>" if a is None else f'\n        <Action name="{a}" />'
                for a in SAMHIE_TOOLBAR)
ui = ui[:bar.start(2)] + items + ui[bar.end(2):]
# Nút thêm mới (nếu có) phải hiện chỉ icon: <Action priority="0" name=…> trong
# ActionProperties như go_back… — thanh để "chữ cạnh icon". Hiện không có nút mới.
head, sep, tail = ui.partition("<ActionProperties")
# Ẩn hành động. Phải là hành động có thật: có trong menu của file, hoặc tên
# có trong mã Dolphin (basic_actions chỉ nằm ở menu ☰, không có trong file;
# tên trong mã là QStringLiteral = UTF-16).
# Đã có mục trong ActionProperties thì thêm thuộc tính, chưa có thì thêm mục.
libs = data + open("/usr/lib64/libdolphinprivate.so.6", "rb").read()
hidden = ["toggle_selection_mode", "view_properties", "group_by", "additional_info", "basic_actions"]
for a in hidden:
    attrs = ' shortcut="" visible="false"' if a == "toggle_selection_mode" else ' visible="false"'
    assert f'name="{a}"' in head or a.encode("utf-16-le") + b"\0\0" in libs, f'unknown action "{a}"'
    props_part = tail.split("</ActionProperties>")[0]
    if re.search(rf'<Action\b[^>]*\bname="{a}"', props_part):
        tail, n = re.subn(rf'(<Action\b[^>]*\bname="{a}")', rf"\1{attrs}", tail, count=1)
    else:
        tail = tail.replace(">", f'>\n        <Action name="{a}"{attrs}/>', 1)
# Icon tròn cho lùi/tiến: thuộc tính icon= trong ActionProperties (KXmlGui gán
# QIcon::fromTheme — đo container: nút hiện đúng icon).
for a, icon in (("go_back", "samhie-go-back"), ("go_forward", "samhie-go-forward")):
    tail, n = re.subn(rf'(<Action priority="0" name="{a}")', rf'\1 icon="{icon}"', tail, count=1)
    assert n == 1, f'no ActionProperties entry for "{a}"'

ui = head + sep + tail

note = ("<!-- Samhie: Dolphin's own dolphinui.rc with three properties changed\n"
        "     in <ActionProperties> and a new mainToolBar (files/scripts/dolphin-ui.sh). -->\n")
ui = ui.replace("<gui ", note + "<gui ", 1)

version = re.search(r'<gui name="dolphin" version="(\d+)"', ui).group(1)
os.makedirs("/usr/share/kxmlgui5/dolphin", exist_ok=True)
with open("/usr/share/kxmlgui5/dolphin/dolphinui.rc", "w") as f:
    f.write(ui)
print(f"dolphinui.rc version {version}: patched compact, details, view_settings, toolbar")
EOF
