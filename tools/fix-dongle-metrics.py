#!/usr/bin/env python3
"""
Dời số đo dọc của font Dongle cho chữ nằm giữa dòng.

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

Giấy phép: OFL 1.1 cho sửa, bản sửa vẫn OFL; Dongle không có Reserved Font
Name nên giữ tên. README cạnh file font ghi sha256 bản gốc và bản sửa.
"""

import sys
from pathlib import Path

from fontTools.ttLib import TTFont

DIR = Path(__file__).resolve().parent.parent / "files/system/usr/share/fonts/dongle"
FILES = ("Dongle-Light.ttf", "Dongle-Regular.ttf", "Dongle-Bold.ttf")
LINE = 850 + 598          # tổng chiều cao dòng của bản gốc — giữ nguyên
CAP_CENTER = 197.5        # tâm chữ H của bản Light/Regular: (−10 + 405) / 2
ASCENT = round((LINE + 2 * CAP_CENTER) / 2)   # 922
DESCENT = LINE - ASCENT                        # 526


def main():
    check = "--check" in sys.argv[1:]
    bad = 0
    for name in FILES:
        path = DIR / name
        font = TTFont(path)
        hhea, os2 = font["hhea"], font["OS/2"]
        now = (hhea.ascent, -hhea.descent, os2.usWinAscent, os2.usWinDescent)
        want = (ASCENT, DESCENT, ASCENT, DESCENT)
        if now == want:
            print(f"  {name}: đã đúng ({ASCENT}/{DESCENT})")
            continue
        if check:
            print(f"  {name}: CHƯA SỬA {now}", file=sys.stderr)
            bad += 1
            continue
        if hhea.ascent + -hhea.descent != LINE:
            sys.exit(f"{name}: tổng ascent+descent không phải {LINE} — không phải bản Dongle 2.000 gốc?")
        hhea.ascent, hhea.descent = ASCENT, -DESCENT
        os2.usWinAscent, os2.usWinDescent = ASCENT, DESCENT
        font.save(path)
        print(f"  {name}: {now[0]}/{now[1]} → {ASCENT}/{DESCENT}")
    if bad:
        sys.exit("Chạy python3 tools/fix-dongle-metrics.py rồi commit font.")


if __name__ == "__main__":
    main()
