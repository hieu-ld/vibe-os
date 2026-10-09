#!/usr/bin/env bash
# Menu chuột phải vùng chữ của Konsole gọn lại (user 2026-10-09). Áp cho cả
# Konsole lẫn panel Terminal của Dolphin (F4) — chung file sessionui.rc.
#
# Cùng cách với dolphin-ui.sh (đọc phần đầu file đó): Konsole nhúng
# sessionui.rc vào libkonsoleprivate.so (Qt resource, nén zstd); KXmlGui ưu
# tiên file cùng tên trong /usr/share/kxmlgui5/konsole/ khi version lớn hơn.
# Lấy bản nhúng của đúng Konsole trong image, sửa, ghi ra — Konsole lên bản
# thì file tự theo.
#
# Chỉ XOÁ DÒNG trong <Menu name="session-popup-menu">, không ẩn hành động:
# các mục vẫn còn trong Menu › (thanh menu ẩn) và phím tắt vẫn chạy.
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
# Không tìm thấy bản nhúng hoặc thiếu hành động nào ở trên thì build DỪNG.
set -oue pipefail

python3 - <<'EOF'
import glob
import os
import re
from compression import zstd  # Python 3.14+

libs = glob.glob("/usr/lib64/libkonsoleprivate.so.*")
assert len(libs) == 1, f"expected 1 libkonsoleprivate, found {libs}"
data = open(libs[0], "rb").read()

found = []
i = 0
while (i := data.find(b"\x28\xb5\x2f\xfd", i)) >= 0:
    try:
        text = zstd.ZstdDecompressor().decompress(data[i:i + 1_000_000])
    except zstd.ZstdError:
        text = b""
    if b'<gui name="session"' in text:
        found.append(text.decode())
    i += 4
assert len(found) == 1, f"expected 1 embedded sessionui.rc, found {len(found)}"
ui = found[0]

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
ui = ui[:start] + body + ui[end:]

note = ("<!-- Samhie: Konsole's own sessionui.rc with a shorter right-click menu\n"
        "     (files/scripts/konsole-ui.sh). -->\n")
ui = ui.replace("<gui ", note + "<gui ", 1)

# Lần sửa của Samhie: tăng mỗi khi đổi file này theo cách tài khoản cũ cần
# nhận (xem dolphin-ui.sh). Bản nhúng + 1 + SAMHIE_REV.
SAMHIE_REV = 0
version = int(re.search(r'<gui name="session" version="(\d+)"', ui).group(1)) + 1 + SAMHIE_REV
ui, n = re.subn(r'(<gui name="session" version=")\d+"', rf'\g<1>{version}"', ui, count=1)
assert n == 1, "no version in sessionui.rc"
os.makedirs("/usr/share/kxmlgui5/konsole", exist_ok=True)
with open("/usr/share/kxmlgui5/konsole/sessionui.rc", "w") as f:
    f.write(ui)
print(f"sessionui.rc version {version}: shorter session-popup-menu")
EOF
