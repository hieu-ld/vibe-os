#!/usr/bin/env python3
"""
Sửa font Dongle: (1) dời số đo dọc cho chữ nằm giữa dòng; (2) thêm các ký tự
vô hình rộng 0 và (3) các khoảng trắng hẹp mà Dongle thiếu.

    python3 tools/fix-dongle-metrics.py          # sửa 3 file trong files/system/usr/share/fonts/dongle/
    python3 tools/fix-dongle-metrics.py --check  # chỉ kiểm (cần fontTools)

VÌ SAO: Dongle gốc khai ascent 850 / descent 598 (hhea và usWin). Qt (và mọi
thứ dùng FreeType) căn giữa CẢ hộp dòng ascent+descent vào ô — tâm hộp nằm
(850−598)/2 = 126 đơn vị trên đường chân chữ, trong khi tâm chữ hoa (H cao
−10…405) nằm ở 197,5. Chữ vì thế lệch LÊN ~71 đơn vị so với icon cạnh nó. Đo
trong máy ảo 2026-10-01 (sidebar Dolphin, Dongle Light 20): icon đúng tâm dòng,
chữ hoa lệch lên 2,5px.

CÁCH SỬA: giữ nguyên tổng ascent+descent = 1448 (dòng cao y như cũ, không ô
nào đổi cỡ), chỉ chia lại sao cho ascent − descent = 2 × 197,5 → tâm hộp dòng
trùng tâm chữ hoa: ascent 922, descent 526. Dấu cao nhất (Ặ, 565) và chân thấp
nhất (g, −145) vẫn nằm gọn trong hộp. sTypo* không đụng: cờ USE_TYPO_METRICS
tắt nên không ai đọc nó.

KÝ TỰ VÔ HÌNH (2026-10-04): Dongle không có U+200B (zero-width space) và
họ hàng. KDE chèn U+200B sau `+ _ . -`… trong tên file để tên dài xuống dòng
được (KStringHandler::preProcessWrap, Dolphin gọi cho mọi tên). Thiếu glyph
→ Qt mượn font khác cho riêng ký tự đó, dòng chữ lấy hộp dòng của font mượn
→ "lost+found", "a_b" trong Dolphin tụt xuống 12px so với tên khác (đo máy
ảo + container). Thêm glyph rỗng, rộng 0 cho từng ký tự trong ZERO_WIDTH.

KHOẢNG TRẮNG HẸP (2026-10-05): cùng bệnh với ký tự vô hình. Giờ kiểu Mỹ của
Qt/ICU có U+202F (narrow no-break space) trước AM/PM: "Today at 10:58 AM" ở
cột Modified của Dolphin tụt xuống ~13px, "59 minutes ago" thì không (máy ảo
2026-10-05). Thêm glyph rỗng cho U+202F và họ hàng (SPACES), rộng theo dấu
cách của Dongle — dấu cách Dongle đã chỉ 200/1000 (= "thin space" của font
thường) nên hẹp = nửa dấu cách.

Giấy phép: OFL 1.1 cho sửa, bản sửa vẫn OFL; Dongle không có Reserved Font
Name nên giữ tên. README cạnh file font ghi sha256 bản gốc và bản sửa.
"""

import sys
from pathlib import Path

from fontTools.ttLib import TTFont
from fontTools.ttLib.tables._g_l_y_f import Glyph

DIR = Path(__file__).resolve().parent.parent / "files/system/usr/share/fonts/dongle"
FILES = ("Dongle-Light.ttf", "Dongle-Regular.ttf", "Dongle-Bold.ttf")
LINE = 850 + 598          # tổng chiều cao dòng của bản gốc — giữ nguyên
CAP_CENTER = 197.5        # tâm chữ H của bản Light/Regular: (−10 + 405) / 2
ASCENT = round((LINE + 2 * CAP_CENTER) / 2)   # 922
DESCENT = LINE - ASCENT                        # 526
# Ký tự vô hình → tên glyph (theo quy ước uniXXXX của Dongle).
ZERO_WIDTH = {cp: f"uni{cp:04X}" for cp in (
    0x200B,  # zero-width space — preProcessWrap chèn
    0x200C,  # zero-width non-joiner
    0x200D,  # zero-width joiner
    0x2060,  # word joiner
    0xFEFF,  # zero-width no-break space (BOM)
)}
# Khoảng trắng có bề ngang → độ rộng tính từ font (dấu cách / dấu chấm).
SPACES = {
    0x202F: lambda space, period: space // 2,  # narrow no-break space — giờ "10:58 AM"
    0x2009: lambda space, period: space // 2,  # thin space
    0x200A: lambda space, period: space // 4,  # hair space
    0x2008: lambda space, period: period,      # punctuation space = bề ngang dấu chấm
}
BLANK = {**ZERO_WIDTH, **{cp: f"uni{cp:04X}" for cp in SPACES}}


def add_blank(font):
    """Thêm glyph rỗng (không nét vẽ) cho ký tự trong BLANK còn thiếu: rộng 0
    với ZERO_WIDTH, rộng theo SPACES với khoảng trắng. Trả về ký tự đã thêm."""
    cmap = font.getBestCmap()
    missing = [cp for cp in BLANK if cp not in cmap]
    if not missing:
        return []
    glyf, hmtx, vmtx = font["glyf"], font["hmtx"], font["vmtx"]
    v_space = vmtx[cmap[0x20]]          # chiều dọc: mượn của dấu cách
    space, period = hmtx[cmap[0x20]][0], hmtx[cmap[0x2E]][0]
    for cp in missing:
        name = BLANK[cp]
        if name in glyf:
            sys.exit(f"glyph {name} đã có mà không nằm trong cmap — bản Dongle lạ?")
        glyf[name] = Glyph()            # không nét vẽ; tự thêm vào thứ tự glyph
        hmtx[name] = (SPACES[cp](space, period) if cp in SPACES else 0, 0)
        vmtx[name] = v_space
        for table in font["cmap"].tables:
            if table.isUnicode():
                table.cmap[cp] = name
    font.setGlyphOrder(glyf.glyphOrder)
    return missing


def main():
    check = "--check" in sys.argv[1:]
    bad = 0
    for name in FILES:
        path = DIR / name
        font = TTFont(path)
        hhea, os2 = font["hhea"], font["OS/2"]
        now = (hhea.ascent, -hhea.descent, os2.usWinAscent, os2.usWinDescent)
        want = (ASCENT, DESCENT, ASCENT, DESCENT)
        cmap = font.getBestCmap()
        lacking = [f"U+{cp:04X}" for cp in BLANK if cp not in cmap]
        if now == want and not lacking:
            print(f"  {name}: đã đúng ({ASCENT}/{DESCENT}, đủ ký tự vô hình + khoảng trắng hẹp)")
            continue
        if check:
            print(f"  {name}: CHƯA SỬA số đo {now}, thiếu {lacking}", file=sys.stderr)
            bad += 1
            continue
        if now != want:
            if hhea.ascent + -hhea.descent != LINE:
                sys.exit(f"{name}: tổng ascent+descent không phải {LINE} — không phải bản Dongle 2.000 gốc?")
            hhea.ascent, hhea.descent = ASCENT, -DESCENT
            os2.usWinAscent, os2.usWinDescent = ASCENT, DESCENT
            print(f"  {name}: {now[0]}/{now[1]} → {ASCENT}/{DESCENT}")
        added = add_blank(font)
        if added:
            print(f"  {name}: thêm " + " ".join(f"U+{cp:04X}" for cp in added))
        font.save(path)
    if bad:
        sys.exit("Chạy python3 tools/fix-dongle-metrics.py rồi commit font.")


if __name__ == "__main__":
    main()
