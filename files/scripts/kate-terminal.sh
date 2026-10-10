#!/usr/bin/env bash
# Build plugin Kate "samhieterminal" (kate/samhie-terminal.cpp): terminal làm
# tab trong vùng editor như VS Code. Mã nguồn được containerfile module chép
# vào /tmp/samhie-kate (recipes/common.yml). Plugin chỉ được BẬT qua file phiên
# của từng người dùng — user-setup bước 0h.
#
# Như samhie-style.sh: đồ build (kho Fedora) cài rồi GỠ bằng dnf history undo,
# image chỉ giữ file .so. Build lỗi thì build image dừng.
set -oue pipefail

SRC=/tmp/samhie-kate
DEST=/usr/lib64/qt6/plugins/kf6/ktexteditor/samhieterminalplugin.so

dnf5 -y install --setopt=install_weak_deps=False cmake gcc-c++ extra-cmake-modules qt6-qtbase-devel \
    kf6-ktexteditor-devel kf6-kparts-devel kf6-kxmlgui-devel kf6-kcoreaddons-devel kf6-kwidgetsaddons-devel
cmake -S "$SRC" -B "$SRC/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$SRC/build"
install -Dm755 "$SRC/build/samhieterminalplugin.so" "$DEST"

# Kate tìm plugin qua metadata JSON nhúng trong mục ELF .note.qt.metadata —
# thiếu tên plugin ở đó là Kate không thấy (kiểm TRƯỚC khi gỡ binutils).
objcopy -O binary --only-section=.note.qt.metadata "$DEST" "$SRC/meta.bin"
grep -q 'Terminal Tabs' "$SRC/meta.bin" || { echo "samhieterminalplugin.so: no plugin metadata" >&2; exit 1; }

dnf5 -y history undo last
rm -rf "$SRC"
echo "samhie Kate terminal plugin: $DEST ($(rpm -q kate))"
