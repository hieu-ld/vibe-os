#!/usr/bin/env bash
# Máy chủ ngôn ngữ QML (qmlls) cho Kate: lỗi, gợi ý, nhảy tới định nghĩa trong
# file QML — kể cả kiểu của Quickshell (Quickshell cài .qmltypes vào
# /usr/lib64/qt6/qml, nơi qmlls tự tìm).
#
# Fedora 44 chỉ có qmlls trong qt6-qtdeclarative-devel; cài nguyên gói kéo theo
# gcc, cmake, qt6-qtbase-devel… (~450 MB, đo 2026-10-12). Bản thân qmlls 5,4 MB
# và mọi thư viện nó cần (qt6-qtdeclarative, qt6-qtbase) đã có trong image —
# nên chỉ TÁCH đúng một file đó ra. Gói vẫn tải từ kho Fedora và kiểm chữ ký
# Fedora trước khi mở. Khi nào Fedora tách qmlls thành gói riêng thì bỏ script
# này, cài gói đó trong recipes/common.yml.
#
# qmlls dùng API riêng của Qt nên phải ĐÚNG phiên bản qt6-qtdeclarative trong
# image — tải theo version-release của gói đã cài, không lấy bản mới nhất.
set -oue pipefail

TMP=$(mktemp -d)
NVR=qt6-qtdeclarative-devel-$(rpm -q --qf '%{VERSION}-%{RELEASE}.%{ARCH}' qt6-qtdeclarative)

dnf5 -y download --destdir "$TMP" "$NVR"
# Chữ ký số của Fedora (khoá có sẵn trong rpmdb của image nền). Sai/thiếu khoá
# là rpmkeys trả lỗi → build dừng.
rpmkeys --checksig "$TMP/$NVR.rpm"

(cd "$TMP" && rpm2cpio "$NVR.rpm" | cpio -idm --quiet ./usr/bin/qmlls)
install -Dm755 "$TMP/usr/bin/qmlls" /usr/bin/qmlls

# Thiếu thư viện thì dừng build ngay, đừng để Kate báo lỗi lúc chạy.
if ldd /usr/bin/qmlls | grep -q "not found"; then
    ldd /usr/bin/qmlls >&2
    exit 1
fi
/usr/bin/qmlls --version

rm -rf "$TMP"
echo "qmlls: /usr/bin/qmlls (from $NVR)"
