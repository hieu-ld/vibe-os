#!/usr/bin/env bash
#
# Nhận một file SVG tải từ tabler.io và biến nó thành icon dùng được cho KDE.
#
#     ./tools/import-tabler-icon.sh <file.svg> <nhóm> <tên-freedesktop>
#
# Ví dụ:
#     ./tools/import-tabler-icon.sh ~/Downloads/search.svg actions system-search
#
# VÌ SAO PHẢI CHUYỂN ĐỔI, không chép thẳng được:
#
# Tabler viết `stroke="currentColor"` và KHÔNG định nghĩa `color` ở đâu cả. Trên
# trang web thì `currentColor` lấy màu chữ của thẻ cha, nên nhìn vẫn đúng. Trong
# KDE không có "thẻ cha" nào — `currentColor` rơi về giá trị khởi tạo, tức ĐEN.
# Icon đen trên dock đen `#0A0A0A` là không nhìn thấy gì, mà cũng không có lỗi
# nào được in ra. Đây đúng kiểu hỏng im lặng.
#
# Cách chữa chép theo ĐÚNG cách Breeze làm (đã đọc file thật trong chroot,
# breeze-dark/actions/22/system-search.svg):
#
#   <style id="current-color-scheme">.ColorScheme-Text { color: #fcfcfc; }</style>
#   <path style="fill:currentColor" class="ColorScheme-Text" .../>
#
# Khối <style> mang id `current-color-scheme` là cái móc của KDE: khi vẽ icon,
# KDE thay giá trị màu trong đó bằng màu chữ của bảng màu đang dùng. Nhờ vậy
# icon tự đổi màu theo theme thay vì cứng một màu. Đặt sẵn `#fcfcfc` (trùng
# breeze-dark) để nếu KDE vì lý do gì không thay thì icon vẫn ra sáng trên nền
# tối, chứ không đen thui.
#
# Đặt class lên TỪNG phần tử vẽ chứ không lên thẻ <svg> gốc: Breeze làm vậy, và
# QtSvg hỗ trợ CSS rất hạn chế nên cứ bám sát bản đã chứng minh là chạy.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="${1-}"; GROUP="${2-}"; NAME="${3-}"

if [[ -z $SRC || -z $GROUP || -z $NAME ]]; then
    sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
    exit 1
fi
[[ -f $SRC ]] || { echo "Không thấy file: $SRC" >&2; exit 1; }

case "$GROUP" in
    actions|apps|devices|places|status|mimetypes) ;;
    *) echo "Nhóm '$GROUP' không có trong chuẩn freedesktop." >&2
       echo "Chọn một trong: actions apps devices places status mimetypes" >&2
       exit 1 ;;
esac

# Breeze vẽ icon này bằng mấy màu? Đó là cách rẻ nhất để biết upstream coi nó là
# KÝ HIỆU hay là LOGO:
#   1 màu  -> ký hiệu đơn sắc (mũi tên, kính lúp, loa...) — nét Tabler thay vào
#             là like-for-like, hợp.
#   nhiều  -> hình vẽ có màu, thường là logo ứng dụng. Thay bằng nét đơn sắc thì
#             app đó mất danh tính, mà không chỉ trên dock — thanh tiêu đề,
#             Alt-Tab, trình đơn đều đổi theo.
# Đo thật 2026-09-08: utilities-terminal (logo Konsole) 11 màu,
# preferences-system (logo System Settings) 6 màu, còn system-search 1 màu.
#
# ĐẾM MÀU PHẢI ĐẾM ĐÚNG FILE — tìm trong ĐÚNG NHÓM trước, cả cây sau.
# Một tên icon có thể tồn tại ở nhiều nhóm với ý nghĩa khác hẳn nhau, và
# `find` trả về theo thứ tự thư mục chứ không theo ý mình. Đo thật 2026-09-11:
# `system-lock-screen` có bản KÝ HIỆU đơn sắc ở actions/22 (ổ khoá) VÀ bản
# 13 màu ở preferences/32 (icon trang cài đặt "Screen Locking"). Bản cũ vớ
# phải preferences/32 rồi la làng "đây là logo, cân nhắc bỏ qua" — báo động
# nhầm, đúng thứ chốt chặn này sinh ra để tránh.
# Breeze để đối chiếu: lấy từ image Samhie (máy build là Ubuntu, không có sẵn):
#     id=$(docker create localhost/samhie:latest) && mkdir -p .cache \
#       && docker cp "$id:/usr/share/icons/breeze" .cache/breeze && docker rm "$id"
BREEZE_DIR="${BREEZE_DIR:-$REPO/.cache/breeze}"
if [[ -d $BREEZE_DIR ]]; then
    breeze_svg=$(find "$BREEZE_DIR/$GROUP" -name "$NAME.svg" 2>/dev/null | head -1)
    [[ -n $breeze_svg ]] || breeze_svg=$(find "$BREEZE_DIR" -name "$NAME.svg" 2>/dev/null | head -1)
    if [[ -n $breeze_svg ]]; then
        echo "    đối chiếu Breeze: ${breeze_svg#"$BREEZE_DIR"/}"
        n_colors=$(grep -oE '#[0-9a-fA-F]{6}' "$breeze_svg" | sort -u | wc -l)
        if (( n_colors > 1 )); then
            echo "    CẢNH BÁO: Breeze vẽ '$NAME' bằng $n_colors màu — nhiều khả năng đây" >&2
            echo "    là LOGO chứ không phải ký hiệu. Thay bằng nét đơn sắc sẽ làm app đó" >&2
            echo "    mất danh tính ở mọi chỗ, không riêng dock. Cân nhắc bỏ qua icon này." >&2
        fi
    fi
fi

DEST_DIR="$REPO/files/system/usr/share/icons/Samhie/$GROUP/scalable"
DEST="$DEST_DIR/$NAME.svg"
mkdir -p "$DEST_DIR"

python3 - "$SRC" "$DEST" <<'PY'
import sys, xml.etree.ElementTree as ET

src, dest = sys.argv[1], sys.argv[2]
NS = "http://www.w3.org/2000/svg"
ET.register_namespace("", NS)
tree = ET.parse(src)
root = tree.getroot()

if "viewBox" not in root.attrib:
    sys.exit("SVG không có viewBox — icon sẽ không co giãn được.")

# Tabler khai màu ở thẻ <svg> gốc; gỡ ra để không còn currentColor nào lơ lửng
# không phân giải được, rồi khai lại trên từng phần tử vẽ.
root_stroke = root.attrib.pop("stroke", None)
root_fill   = root.attrib.pop("fill",   None)
# Mấy thuộc tính này KHÔNG dính tới màu, để nguyên cho nét vẽ giữ đúng dáng.
# width/height cố định thì bỏ — viewBox mới là thứ quyết định, giữ lại chỉ tổ
# ghim icon vào đúng 24px.
root.attrib.pop("width", None)
root.attrib.pop("height", None)
root.attrib.pop("class", None)

style = ET.Element(f"{{{NS}}}style")
style.set("id", "current-color-scheme")
style.set("type", "text/css")
style.text = ".ColorScheme-Text { color: #fcfcfc; }"
defs = ET.Element(f"{{{NS}}}defs")
defs.append(style)
root.insert(0, defs)

painted = 0
for el in root.iter():
    tag = el.tag.split("}")[-1]
    if tag in ("svg", "defs", "style", "title", "desc"):
        continue
    s, f = el.get("stroke", root_stroke), el.get("fill", root_fill)
    # Phần tử vẽ trong suốt (Tabler chèn một path khung 24x24 rỗng) thì bỏ qua.
    if (s in (None, "none")) and (f in (None, "none")):
        continue

    # PHẢI ghi cả giá trị "none" ra, đừng chỉ ghi cái nào có màu. Mặc định của
    # SVG là `fill: black` — thẻ <svg> gốc của Tabler khai `fill="none"` để chặn
    # cái mặc định đó, nên gỡ nó đi mà không khai lại thì mọi nét hở biến thành
    # mảng đen đặc. Đã dính đúng vậy: icon terminal ra ô vuông đen phủ mất dấu
    # `>` và gạch dưới, mà phép kiểm độ sáng vẫn báo "đạt" vì viền ngoài vẫn
    # sáng. Hình sai chứ không phải màu sai.
    if f is None and s is not None:
        f = "none"          # nét hở: chặn fill đen mặc định
    decls = []
    if s is not None:
        decls.append("stroke:currentColor" if s == "currentColor" else f"stroke:{s}")
    if f is not None:
        decls.append("fill:currentColor" if f == "currentColor" else f"fill:{f}")
    if not any(d.endswith("currentColor") for d in decls):
        continue
    el.set("style", ";".join(decls))
    el.set("class", "ColorScheme-Text")
    for a in ("stroke", "fill"):
        if a in el.attrib:
            del el.attrib[a]
    painted += 1

if painted == 0:
    sys.exit("Không tìm thấy phần tử vẽ nào — file này có đúng là icon Tabler không?")

tree.write(dest, encoding="unicode", xml_declaration=True)
print(f"    {painted} phần tử vẽ")
PY

# Đối chiếu HÌNH, không chỉ màu. Vẽ bản gốc (thay currentColor bằng màu sáng)
# và bản đã chuyển đổi ra cùng một cỡ, rồi so kênh alpha: chỗ nào có nét thì cả
# hai phải giống nhau. Bắt được đúng loại lỗi mà phép kiểm độ sáng bỏ sót —
# hình bị tô đặc thêm, hoặc mất nét, mà màu vẫn "sáng".
if command -v rsvg-convert >/dev/null && python3 -c "import PIL" 2>/dev/null; then
    tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
    sed 's/currentColor/#fcfcfc/g' "$SRC" > "$tmp/goc.svg"
    rsvg-convert -w 128 -h 128 -o "$tmp/goc.png"  "$tmp/goc.svg"
    rsvg-convert -w 128 -h 128 -o "$tmp/moi.png"  "$DEST"
    python3 - "$tmp/goc.png" "$tmp/moi.png" <<'PY2'
import sys
from PIL import Image
a = Image.open(sys.argv[1]).convert("RGBA").getchannel("A")
b = Image.open(sys.argv[2]).convert("RGBA").getchannel("A")
pa, pb = list(a.getdata()), list(b.getdata())
diff = sum(1 for x, y in zip(pa, pb) if abs(x - y) > 60)
inked = sum(1 for x in pa if x > 60)
pct = 100.0 * diff / max(inked, 1)
print(f"    đối chiếu hình với bản gốc: lệch {pct:.1f}%")
if pct > 12:
    sys.exit("    HÌNH LỆCH so với bản gốc — xem lại, đừng dùng file này.")
PY2
else
    echo "    (bỏ qua đối chiếu hình: thiếu rsvg-convert hoặc python3-pil)"
fi

echo "    $SRC"
echo " -> files/system/usr/share/icons/Samhie/$GROUP/scalable/$NAME.svg"
