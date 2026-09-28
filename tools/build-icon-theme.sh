#!/usr/bin/env bash
#
# Dựng bộ icon Samhie từ các file SVG bỏ trong files/system/usr/share/icons/Samhie/<nhóm>/scalable/.
#
#     ./tools/build-icon-theme.sh
#
# Việc của script: sinh lại index.theme cho khớp những nhóm ĐANG CÓ file, và
# kiểm tra từng SVG. Không đụng tới file SVG.
#
# Cách bộ icon này hoạt động: nó khai Inherits=breeze-dark, nên chỉ cần chứa
# những icon muốn thay. Mọi tên icon không có ở đây, KDE tự lấy của Breeze —
# tức là thay được bao nhiêu dùng bấy nhiêu, không cần phủ hết 19.848 file.
# Tên file PHẢI là tên chuẩn freedesktop (vd system-search.svg), vì KDE tìm
# icon theo tên chứ không theo đường dẫn.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
THEME_DIR="$REPO/files/system/usr/share/icons/Samhie"

# Tên nhóm thư mục -> giá trị Context trong index.theme, theo chuẩn freedesktop.
declare -A CONTEXT_MAP=(
    [actions]=Actions
    [apps]=Applications
    [devices]=Devices
    [places]=Places
    [status]=Status
    [mimetypes]=MimeTypes
)

dir_list=()
total=0

for group_dir in "$THEME_DIR"/*/scalable; do
    [[ -d $group_dir ]] || continue
    group=$(basename "$(dirname "$group_dir")")
    count=$(find "$group_dir" -maxdepth 1 -name '*.svg' | wc -l)
    [[ $count -gt 0 ]] || continue

    if [[ -z ${CONTEXT_MAP[$group]:-} ]]; then
        echo "BỎ QUA nhóm lạ '$group' — không có trong chuẩn freedesktop." >&2
        continue
    fi
    dir_list+=( "$group/scalable" )
    total=$(( total + count ))
    printf '  %-12s %2d icon\n' "$group" "$count"
done

# Giấy phép của bộ icon gốc PHẢI đi kèm. Tabler là MIT — cho dùng và sửa thoải
# mái, nhưng bắt buộc giữ lại thông báo bản quyền khi phát hành lại. Mà image công khai thì
# đúng là phát hành lại. Thiếu file này là vi phạm giấy phép một cách hoàn toàn
# vô ý, và không có gì nhắc.
#
# Kiểm ở đây chứ không ở check-chroot.sh, vì đây là lúc người ta vừa bỏ icon vào
# — nhắc ngay thì đi lấy file mất mười giây, còn nhắc lúc sắp build image thì phải
# quay ngược cả quy trình.
license_file="$THEME_DIR/LICENSE"
if [[ ! -f $license_file ]]; then
    echo "THIẾU $THEME_DIR/LICENSE" >&2
    echo "Chép file LICENSE của bộ icon gốc vào đó (Tabler: MIT)." >&2
    echo "Xem ~/Projects/Distro/docs/icons-wanted.md (bản 1.0), mục Giấy phép." >&2
    exit 1
fi

# Có file thôi CHƯA ĐỦ. Bản trong repo từng ghi "Copyright (c) 2018-2026 The
# Tabler Authors" — nghe rất hợp lý, và sai: upstream ghi "Paweł Kuna". Nó được
# viết lại từ trí nhớ chứ không chép từ gói gốc. MIT đòi đúng MỘT thứ là giữ
# nguyên thông báo bản quyền, nên sai tên tác giả là hỏng ngay chỗ duy nhất
# phải giữ. Phép kiểm cũ chỉ hỏi "file có tồn tại không" nên nó không thể bắt.
if ! grep -qF 'Paweł Kuna' "$license_file"; then
    echo "$THEME_DIR/LICENSE không phải bản gốc của Tabler." >&2
    echo "Thiếu dòng bản quyền 'Copyright (c) ... Paweł Kuna'." >&2
    echo "Chép lại từ gói gốc, đừng gõ tay:" >&2
    echo "  curl -sSL https://registry.npmjs.org/@tabler/icons/-/icons-3.46.0.tgz \\" >&2
    echo "    | tar xz -O package/LICENSE > $license_file" >&2
    exit 1
fi

if [[ ${#dir_list[@]} -eq 0 ]]; then
    echo "Chưa có icon nào. Bỏ file SVG vào files/system/usr/share/icons/Samhie/<nhóm>/scalable/," >&2
    echo "đặt tên theo tên chuẩn freedesktop — xem ~/Projects/Distro/docs/icons-wanted.md (bản 1.0)." >&2
    exit 1
fi

# Kiểm tra từng SVG: phải là XML đọc được và phải có viewBox, không có viewBox
# thì icon không co giãn và sẽ vỡ ở cỡ khác.
bad=0
while IFS= read -r svg; do
    if ! python3 -c "import xml.dom.minidom,sys; xml.dom.minidom.parse(sys.argv[1])" "$svg" 2>/dev/null; then
        echo "  LỖI: không đọc được XML — ${svg#"$REPO"/}" >&2
        bad=$(( bad + 1 ))
        continue
    fi
    if ! grep -qi 'viewBox' "$svg"; then
        echo "  CẢNH BÁO: thiếu viewBox, icon sẽ không co giãn đúng — ${svg#"$REPO"/}" >&2
    fi
    # Bẫy màu đen. File tải thẳng từ tabler.io khai stroke="currentColor" mà
    # không định nghĩa `color`, nên KDE vẽ ra màu ĐEN — đã đo thật: bản gốc cho
    # điểm sáng nhất 0/255, bản qua script cho 253-255. Icon đen trên dock đen
    # là tàng hình mà không có lỗi nào được in ra, nên phải chặn ở đây.
    #
    # ĐIỀU KIỆN ĐÃ SIẾT LẠI CHO ĐÚNG CHỖ (2026-09-10). Trước đó phép kiểm là
    # "thiếu current-color-scheme thì hỏng", mà như vậy là bắt nhầm: logo NHIỀU
    # MÀU vẽ tay khai màu thẳng (fill="#00C0E8"...) thì không cần và không nên
    # có khối đó — nó vẫn hiện đúng màu, chẳng liên quan gì tới bẫy màu đen.
    # Phép kiểm cũ chặn thẳng, tức là chặn đúng thứ user sắp đưa vào.
    #
    # Cái thật sự nguy hiểm là `currentColor` mà KHÔNG có khối định nghĩa màu.
    # Nên hỏi đúng câu đó. Mọi file Tabler chép tay vẫn bị bắt như cũ, vì bản
    # gốc nào cũng có `currentColor`.
    if grep -q 'currentColor' "$svg" && ! grep -q 'current-color-scheme' "$svg"; then
        echo "  LỖI: dùng currentColor mà không có khối current-color-scheme — ${svg#"$REPO"/}" >&2
        echo "       icon sẽ ra màu đen và tàng hình trên dock." >&2
        echo "       Đừng chép tay: ./tools/import-tabler-icon.sh <file> <nhóm> <tên>" >&2
        bad=$(( bad + 1 ))
    fi
    # SVG bọc ảnh raster: cảnh báo, không chặn. Nó CHẠY, chỉ là mất hết cái lợi
    # của vector — không co giãn sắc nét, không đổi màu theo bảng màu, và nặng
    # gấp mấy trăm lần (đo thật: một file 1.0MB bọc PNG 1254x1254, so với 3KB
    # cho cùng hình vẽ bằng path).
    if grep -q '<image' "$svg"; then
        px=$(( $(stat -c%s "$svg") / 1024 ))
        echo "  CẢNH BÁO: là ảnh raster bọc trong SVG (${px}KB) — ${svg#"$REPO"/}" >&2
        echo "            chạy được, nhưng không co giãn sắc nét và không đổi màu." >&2
    fi
    # Logo nhiều màu đi qua được nhánh trên, nhưng vẫn phải có MÀU ở đâu đó.
    # File không có `fill`, `stroke` lẫn `currentColor` thì trắng trơn — chỉ có
    # thể là xuất hỏng.
    #
    # Trừ file bọc ảnh raster ở trên: màu của nó nằm trong PNG nhúng chứ không
    # phải trong thuộc tính SVG, nên hỏi câu này là bắt nhầm. Đã dính thật:
    # search_icon_logo.svg của user không có fill/stroke nào mà vẫn hiện đúng.
    if ! grep -qE 'fill|stroke|currentColor|<image' "$svg"; then
        echo "  LỖI: không thấy fill/stroke nào — ${svg#"$REPO"/}" >&2
        echo "       file này sẽ vẽ ra một ô trống." >&2
        bad=$(( bad + 1 ))
    fi
done < <(find "$THEME_DIR" -name '*.svg')

{
    echo "[Icon Theme]"
    echo "Name=Samhie"
    echo "Comment=Icon overrides for Samhie, everything else falls back to Breeze Dark"
    echo "Comment[vi]=Icon riêng của Samhie, phần còn lại dùng Breeze Dark"
    # Thứ tự kế thừa quan trọng: breeze-dark trước để hợp tông nền tối, rồi
    # breeze, cuối cùng hicolor cho icon do app tự cài (Dolphin, Edge...).
    echo "Inherits=breeze-dark,breeze,hicolor"
    echo "Directories=$(IFS=,; echo "${dir_list[*]}")"
    for d in "${dir_list[@]}"; do
        group=${d%%/*}
        echo
        echo "[$d]"
        # Size chỉ là gợi ý cho bộ chọn; Type=Scalable + Min/Max mới là thứ cho
        # phép dùng một file SVG ở mọi cỡ.
        echo "Size=22"
        echo "Type=Scalable"
        echo "MinSize=8"
        echo "MaxSize=512"
        echo "Context=${CONTEXT_MAP[$group]}"
    done
} > "$THEME_DIR/index.theme"

echo
echo "index.theme: ${#dir_list[@]} nhóm, $total icon."
echo "giấy phép: $(head -1 "$license_file")"
if [[ $bad -gt 0 ]]; then
    echo "$bad file SVG hỏng — sửa rồi chạy lại." >&2
    exit 1
fi
