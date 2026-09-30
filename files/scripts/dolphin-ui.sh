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

note = ("<!-- Samhie: Dolphin's own dolphinui.rc with three properties changed\n"
        "     in <ActionProperties> (files/scripts/dolphin-ui.sh). -->\n")
ui = ui.replace("<gui ", note + "<gui ", 1)

version = re.search(r'<gui name="dolphin" version="(\d+)"', ui).group(1)
os.makedirs("/usr/share/kxmlgui5/dolphin", exist_ok=True)
with open("/usr/share/kxmlgui5/dolphin/dolphinui.rc", "w") as f:
    f.write(ui)
print(f"dolphinui.rc version {version}: patched compact, details, view_settings")
EOF
