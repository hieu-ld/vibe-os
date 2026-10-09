#!/usr/bin/env python3
"""
Phủ icon LOẠI FILE của Breeze bằng icon Samhie: mỗi tên trong
breeze-dark/mimetypes thành một liên kết tới icon của nhóm nó thuộc về.

    # chạy TRONG image (cần /usr/share/mime và Breeze của image), ghi vào repo:
    docker run --rm -v "$PWD":/repo localhost/samhie:latest \\
        python3 /repo/tools/build-mime-icons.py /repo
    python3 tools/build-mime-icons.py --check   # CI: mọi liên kết trỏ tới file có thật

VÌ SAO PHẢI PHỦ TỪNG TÊN: KDE tìm icon theo tên mime (text-x-python) dọc
chuỗi Inherits (Samhie → breeze-dark) và chỉ rơi về icon chung
(text-x-script) khi KHÔNG bộ nào có tên đó. Breeze có sẵn ~500 tên, nên thiếu
tên nào ở Samhie là tên đó hiện khối màu Breeze.

Icon của từng nhóm (tên chính, file thật) do tools/build-icons.py sinh từ
theme/icons/hugeicons/ — bảng NHÓM dưới đây chỉ nói nhóm nào dùng tên chính
nào. Breeze thêm tên mới khi lên bản thì chạy lại script này; tên không xếp
được nhóm là báo lỗi, để không có tên nào lặng lẽ rơi về Breeze.

Cách xếp một tên, dừng ở luật đầu tiên khớp:
  1. TÊN: bảng tên riêng (tên Breeze không có kiểu mime tương ứng).
  2. Kiểu mime có icon tên đó (theo /usr/share/mime/packages), xét kiểu đó
     rồi lần lượt các kiểu cha (sub-class-of): KIỂU (đúng kiểu) → CHỮ (văn
     bản thường, không phải mã) → generic-icon của kiểu → nhóm theo tiền tố
     (image/, audio/, video/, font/ xét TRƯỚC kiểu cha; text/ là mã nguồn).
  3. Tên Breeze là liên kết tới tên khác → xếp theo tên đích.
  4. MAU_TEN: một phần của tên (excel, powerpoint, audio-…).
"""

import os
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 and sys.argv[1] != "--check"
            else Path(__file__).resolve().parent.parent)
OUT = ROOT / "files/system/usr/share/icons/Samhie/mimetypes/scalable"
BREEZE = Path("/usr/share/icons/breeze-dark/mimetypes/64")
MIME = Path("/usr/share/mime/packages")
NS = "{http://www.freedesktop.org/standards/shared-mime-info}"

# Nhóm → tên chính (file thật, tools/build-icons.py sinh).
NHOM = {
    "txt": "text-plain", "pdf": "application-pdf", "doc": "x-office-document",
    "xls": "x-office-spreadsheet", "ppt": "x-office-presentation",
    "csv": "text-csv", "zip": "application-zip", "rar": "application-vnd.rar",
    "jpg": "image-jpeg", "png": "image-png", "svg": "image-svg+xml",
    "gif": "image-gif", "html": "text-html", "css": "text-css",
    "xml": "application-xml", "wav": "audio-x-wav",
    "text": "text-x-generic", "code": "text-x-script",
    "shell": "application-x-shellscript", "data": "application-json",
    "config": "application-x-desktop", "image": "image-x-generic",
    "video": "video-x-generic", "audio": "audio-x-generic",
    "archive": "package-x-generic", "package": "application-x-rpm",
    "database": "application-vnd.sqlite3", "font": "font-x-generic",
    "key": "application-pgp-keys", "binary": "application-octet-stream",
    "unknown": "unknown",
}

CHINH = {v: k for k, v in NHOM.items()}

# Đúng kiểu mime → nhóm.
KIEU = {
    "text/plain": "txt",
    "application/pdf": "pdf",
    "text/csv": "csv",
    "application/zip": "zip",
    "application/vnd.rar": "rar", "application/x-rar": "rar",
    "image/jpeg": "jpg", "image/png": "png", "image/gif": "gif",
    "image/svg+xml": "svg", "image/svg+xml-compressed": "svg",
    "text/html": "html", "application/xhtml+xml": "html",
    "text/css": "css",
    "application/xml": "xml", "text/xml": "xml",
    "audio/x-wav": "wav", "audio/vnd.wave": "wav", "audio/wav": "wav",
    # Mã chạy bằng shell (script khác — python, perl… — là mã nguồn).
    "application/x-shellscript": "shell", "application/x-csh": "shell",
    "application/x-zsh": "shell", "application/x-fishscript": "shell",
    "text/x-dsrc": "code", "text/x-sass": "code", "text/x-scss": "code",
    "application/xml-dtd": "code",
    "application/xslt+xml": "xml", "application/x-xliff": "xml",
    # Dữ liệu có cấu trúc / cấu hình.
    "application/json": "data", "application/json5": "data",
    "application/x-yaml": "data", "application/yaml": "data",
    "application/toml": "data", "application/x-toml": "data",
    "text/tab-separated-values": "data", "text/spreadsheet": "data",
    "application/x-desktop": "config",
    "application/x-cisco-vpn-settings": "config",
    "application/x-openvpn-profile": "config",
    # Cơ sở dữ liệu.
    "application/vnd.sqlite3": "database", "application/x-sqlite2": "database",
    "application/sql": "database",
    "application/vnd.oasis.opendocument.database": "database",
    "application/vnd.ms-access": "database",
    # Khoá, chữ ký, chứng chỉ.
    "application/pgp-keys": "key", "application/pgp-encrypted": "key",
    "application/pgp-signature": "key", "application/x-pkcs12": "key",
    "application/pkix-cert": "key", "application/x-x509-ca-cert": "key",
    "application/x-pem-key": "key", "application/pkcs7-mime": "key",
    "application/pkcs7-signature": "key", "application/x-pkcs7-certificates": "key",
    # Gói cài đặt + ảnh đĩa (một "hộp" chứa cả hệ thống / chương trình).
    "application/x-rpm": "package", "application/x-source-rpm": "package",
    "application/vnd.debian.binary-package": "package",
    "application/vnd.flatpak": "package", "application/vnd.flatpak.ref": "package",
    "application/vnd.flatpak.repo": "package",
    "application/vnd.android.package-archive": "package",
    "application/x-iso9660-appimage": "package", "application/vnd.appimage": "package",
    "application/x-msi": "package", "application/vnd.snap": "package",
    "application/vnd.efi.iso": "package", "application/vnd.efi.img": "package",
    "application/x-raw-disk-image": "package", "application/x-cd-image": "package",
    "application/x-apple-diskimage": "package", "application/x-qemu-disk": "package",
    "application/x-virtualbox-vdi": "package", "application/x-virtualbox-vmdk": "package",
    "application/x-virtualbox-vhd": "package", "application/x-virtualbox-vhdx": "package",
    # Nhị phân.
    "application/octet-stream": "binary", "application/x-executable": "binary",
    "application/x-sharedlib": "binary", "application/x-object": "binary",
    "application/x-core": "binary", "application/x-ms-dos-executable": "binary",
    "application/x-java": "binary", "application/x-gettext-translation": "binary",
}

# Văn bản để ĐỌC (không phải mã nguồn) — mọi kiểu text/ khác là mã.
CHU = {
    "text/markdown", "text/x-log", "text/x-readme", "text/x-authors",
    "text/x-changelog", "text/x-copying", "text/x-credits", "text/x-install",
    "text/x-nfo", "text/x-txt2tags", "text/vnd.kde.kcrash-report",
    "text/x-apport", "text/enriched", "text/troff", "text/x-tex",
    "text/x-bibtex", "text/x-texinfo", "text/vnd.typst", "text/x-lilypond",
    "text/vnd.abc", "text/calendar", "text/vcard", "text/x-ldif",
    "text/vtt", "text/x-ssa", "text/x-subviewer", "text/x-microdvd",
    "text/x-mpsub", "application/x-subrip", "text/xmcd",
    "message/rfc822", "message/news", "message/partial", "message/x-gnu-rmail",
    "text/x-rst", "text/org", "text/x-todo-txt", "text/x-setext", "text/richtext",
    "text/x-troff-me", "text/x-troff-ms", "text/x-typst", "text/rfc822-headers",
}

# generic-icon của kiểu mime → nhóm.
CHUNG = {
    "x-office-document": "doc", "x-office-spreadsheet": "xls",
    "x-office-presentation": "ppt", "x-office-drawing": "image",
    "package-x-generic": "archive", "font-x-generic": "font",
    "image-x-generic": "image", "audio-x-generic": "audio",
    "video-x-generic": "video", "text-x-script": "code",
    "text-x-generic": "text", "text-html": "xml",
    "application-x-executable": "binary", "media-optical": "package",
}

TIEN_TO = {"text/": "code", "image/": "image", "audio/": "audio",
           "video/": "video", "font/": "font", "message/": "text"}

# Tên Breeze không có kiểu mime nào mang (tên cũ, tên của app, tên chung).
TEN = {
    "android-package-archive": "package", "application-certificate": "key",
    "application-msonenote": "doc", "application-msoutlook": "text",
    "application-x-kdenlive": "video", "application-x-kdenlivetitle": "video",
    "application-x-kmymoney": "database", "application-x-kplato": "doc",
    "fonts-package": "font", "message": "text", "podcast": "audio",
    "x-media-podcast": "audio", "text-csharp": "code", "text-dockerfile": "code",
    "text-wiki": "text", "text-x-apport": "text", "text-x-r": "code",
    "text-vnd.typst": "text", "virtualbox-ova": "package",
    "virtualbox-ovf": "package", "virtualbox-vbox": "config",
    "virtualbox-vbox-extpack": "package", "virtualbox-hdd": "package",
    "virtualbox-vdi": "package", "virtualbox-vhd": "package",
    "virtualbox-vmdk": "package", "x-kde-nsplugin-generated": "binary",
    "x-office-address-book": "text", "x-office-calendar": "text",
    "x-office-contact": "text", "x-mail-distribution-list": "text",
    "application-x-executable-script": "shell", "text-x-shellscript": "shell",
    "text-x-generic": "text", "text-x-plain": "txt",
    "uri-mms": "video", "uri-mmst": "video", "uri-pnm": "video",
    "uri-rtspt": "video", "uri-rtspu": "video",
    "application-vnd.android.app-bundle": "package", "application-x-xz-pkg": "package",
    "application-vnd.apple.pkpasses": "archive", "application-x-ar": "archive",
    "application-vnd.iccprofile": "binary", "application-x-ms-shortcut": "binary",
    "application-x-python-bytecode": "binary", "application-x-bittorrent": "binary",
    "application-vnd.nokia.xml.qt.resource": "xml",
    "application-x-audacity-project": "audio", "application-x-cda": "audio",
    "application-x-gdscript": "code", "application-x-godot-shader": "code",
    "application-x-godot-project": "config", "application-x-godot-resource": "data",
    "application-x-godot-scene": "data", "application-x-kcsrc": "config",
    "application-x-kexi-connectiondata": "database",
    "application-x-kexiproject-shortcut": "database",
    "application-x-skg": "database", "application-x-skgc": "database",
    "application-x-labplot2": "data", "application-x-marble": "data",
    "application-x-rdata": "data", "application-x-root": "data",
    "application-xmind": "doc", "application-vnd.ms-infopath": "doc",
    "application-sxw": "doc", "odf": "doc",
    "application-x-zerosize": "unknown", "image-x-svg+xml": "svg",
    "text-x-java-source": "code", "audiobook": "audio",
    "application-x-partial-download": "binary",
    "application-x-qet-element": "xml", "application-x-qet-project": "xml",
}

# Phần tên → nhóm, cho tên vẫn chưa xếp được (Office có macro, WPS, LibreOffice…).
MAU_TEN = [("excel", "xls"), (".xls", "xls"), ("spreadsheet", "xls"),
           ("powerpoint", "ppt"), (".ppt", "ppt"), ("presentation", "ppt"),
           ("drawing", "image"), ("master-document", "doc"), ("web-template", "doc"),
           ("audio-", "audio"), ("video-", "video"), ("image-", "image")]

# Không phải file: nhóm/máy mạng SMB, thùng rác (KIO vẽ như thư mục), tên hỏng của
# Breeze ("text-x-generic.svapplication-x-awk"), "none".
BO_QUA = {"application-x-smb-workgroup", "application-x-smb-server",
          "application-x-trash", "none",
          "text-x-generic.svapplication-x-awk"}


def doc_mime():
    """{kiểu: (cha, generic-icon)}, {tên icon: [kiểu]}"""
    info, theo_icon = {}, {}
    for f in sorted(MIME.glob("*.xml")):
        for mt in ET.parse(f).getroot().iter(NS + "mime-type"):
            t = mt.get("type")
            cha, chung, icon, ten_khac = [], None, None, []
            for c in mt:
                tag = c.tag[len(NS):]
                if tag == "sub-class-of":
                    cha.append(c.get("type"))
                elif tag == "generic-icon":
                    chung = c.get("name")
                elif tag == "icon":
                    icon = c.get("name")
                elif tag == "alias":
                    ten_khac.append(c.get("type"))
            old = info.get(t, ([], None))
            info[t] = (old[0] + cha, chung or old[1])
            for n in [icon or t.replace("/", "-")] + [a.replace("/", "-") for a in ten_khac]:
                theo_icon.setdefault(n, []).append(t)
    return info, theo_icon


def xep_kieu(t, info, da_xet=()):
    if t in da_xet:
        return None
    if t in KIEU:
        return KIEU[t]
    if t in CHU:
        return "text"
    cha, chung = info.get(t, ([], None))
    if chung in CHUNG:
        return CHUNG[chung]
    # Ảnh/âm thanh/video/font quyết theo tiền tố TRƯỚC kiểu cha: audio/x-vorbis+ogg
    # là con của application/ogg (generic-icon video) nhưng vẫn là âm thanh.
    for tien_to in ("image/", "audio/", "video/", "font/"):
        if t.startswith(tien_to):
            return TIEN_TO[tien_to]
    for p in cha:
        if p in ("text/plain", "application/x-executable"):
            continue   # cha chung của mọi văn bản / script — để tiền tố quyết
        if (r := xep_kieu(p, info, da_xet + (t,))):
            return r
    for tien_to, nhom in TIEN_TO.items():
        if t.startswith(tien_to):
            return nhom
    return None


def xep(ten, info, theo_icon, lien_ket):
    if ten in TEN:
        return TEN[ten]
    for t in theo_icon.get(ten, []):
        if (r := xep_kieu(t, info)):
            return r
    if ten in lien_ket:
        dich = lien_ket[ten]
        if dich in CHINH:
            return CHINH[dich]
        if (r := xep(dich, info, theo_icon, lien_ket)):
            return r
    for mau, nhom in MAU_TEN:
        if mau in ten:
            return nhom
    return None


def check():
    bad = [p for p in OUT.iterdir() if p.is_symlink() and not p.resolve().is_file()]
    for p in bad:
        print(f"HỎNG: {p.relative_to(ROOT)} → {os.readlink(p)}", file=sys.stderr)
    if bad:
        sys.exit("Liên kết icon loại file trỏ vào file không có — chạy tools/build-icons.py.")
    n = sum(1 for p in OUT.iterdir() if p.is_symlink())
    print(f"icon loại file: {n} liên kết đều trỏ tới icon có thật")


def main():
    if "--check" in sys.argv:
        return check()
    info, theo_icon = doc_mime()
    ten_breeze, lien_ket = [], {}
    for p in sorted(BREEZE.glob("*.svg")):
        if p.stem.startswith("inode-") or p.stem in BO_QUA:
            continue   # thư mục: places/ của Samhie lo
        ten_breeze.append(p.stem)
        if p.is_symlink():
            lien_ket[p.stem] = Path(os.readlink(p)).stem

    chinh = set(CHINH)
    ket_qua, thieu = {}, []
    for ten in ten_breeze:
        if ten in chinh:
            continue
        nhom = xep(ten, info, theo_icon, lien_ket)
        if nhom is None:
            thieu.append(ten)
        else:
            ket_qua[ten] = NHOM[nhom]
    if thieu:
        sys.exit("Không xếp được nhóm (thêm vào TEN hoặc KIEU): " + ", ".join(thieu))

    # Kiểu mime Breeze KHÔNG có icon: KDE rơi về generic-icon (không khai thì
    # "<loại>-x-generic", không có nữa thì "unknown"). Chỉ thêm tên khi nhóm
    # của mình khác chỗ KDE sẽ rơi về — vd .yml: text-x-generic → dữ liệu {}.
    def roi_ve(t):
        chung = info.get(t, ([], None))[1] or t.split("/")[0] + "-x-generic"
        dich = ket_qua.get(chung, chung)
        return dich if dich in chinh else "unknown"
    for ten, kieu in sorted(theo_icon.items()):
        if ten in ket_qua or ten in chinh or ten in BO_QUA or ten.startswith("inode-"):
            continue
        nhom = xep_kieu(kieu[0], info)
        if nhom and NHOM[nhom] != roi_ve(kieu[0]):
            ket_qua[ten] = NHOM[nhom]
    for ten in chinh:
        if not (OUT / f"{ten}.svg").is_file():
            sys.exit(f"Thiếu icon chính {ten}.svg — chạy tools/build-icons.py trước.")

    for p in OUT.iterdir():
        if p.is_symlink():
            p.unlink()
    for ten, dich in sorted(ket_qua.items()):
        (OUT / f"{ten}.svg").symlink_to(f"{dich}.svg")

    theo_nhom = {}
    for ten, dich in ket_qua.items():
        theo_nhom.setdefault(dich, []).append(ten)
    for nhom, chinh_ten in NHOM.items():
        ds = sorted(theo_nhom.get(chinh_ten, []))
        print(f"{nhom:9} {chinh_ten:26} +{len(ds):3}  {' '.join(ds)}")
    print(f"\n{len(ket_qua)} liên kết ({len(ten_breeze)} tên Breeze), {len(chinh)} icon chính.")


if __name__ == "__main__":
    main()
