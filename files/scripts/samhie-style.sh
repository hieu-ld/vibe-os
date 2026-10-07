#!/usr/bin/env bash
# Build widget style "samhie" (style/samhie-style.cpp: QProxyStyle bọc Kvantum,
# vẽ những chỗ theme Kvantum không vẽ được — docs/PLAN.md giai đoạn 4).
# kdeglobals chọn nó (widgetStyle=samhie). Mã nguồn được containerfile module
# chép vào /tmp/samhie-style (recipes/common.yml).
#
# Đồ build (cmake, gcc, header Qt + KF6WidgetsAddons — kho Fedora) cài rồi GỠ bằng dnf history
# undo: image chỉ giữ lại đúng file .so. Build lỗi thì build image dừng.
set -oue pipefail

SRC=/tmp/samhie-style
DEST=/usr/lib64/qt6/plugins/styles/libsamhie.so

dnf5 -y install --setopt=install_weak_deps=False cmake gcc-c++ qt6-qtbase-devel kf6-kwidgetsaddons-devel
cmake -S "$SRC" -B "$SRC/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$SRC/build"
install -Dm755 "$SRC/build/libsamhie.so" "$DEST"

# Plugin phải khai khoá "samhie" (Q_PLUGIN_METADATA). Qt 6 cất metadata dạng
# CBOR trong mục ELF .note.qt.metadata — tách đúng mục đó ra mà tìm (objcopy
# có nhờ binutils, đi kèm gcc — kiểm TRƯỚC khi gỡ đồ build).
objcopy -O binary --only-section=.note.qt.metadata "$DEST" "$SRC/meta.bin"
grep -q samhie "$SRC/meta.bin" || { echo "libsamhie.so: no samhie key in plugin metadata" >&2; exit 1; }

dnf5 -y history undo last
rm -rf "$SRC"
echo "samhie style plugin: $DEST ($(rpm -q qt6-qtbase))"
