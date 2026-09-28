#!/usr/bin/env bash
# Đặt theme Plymouth "samhie" làm màn hình khởi động mặc định.
#
# Theme (files/system/usr/share/plymouth/themes/samhie) chỉ chứa thứ của
# Samhie: samhie.plymouth, 5 chấm, chữ Samhie. Ảnh của ô nhập mật khẩu (ổ mã
# hoá) lấy từ theme spinner của Fedora — chép lúc build chứ không commit, để
# chúng đi theo bản vá của gói plymouth-theme-spinner.
#
# Chỉ ghi /etc/plymouth/plymouthd.conf. Theme nằm trong INITRAMFS mới hiện được
# lúc khởi động — module `initramfs` cuối recipe dựng lại nó.
set -oue pipefail

theme=/usr/share/plymouth/themes/samhie
spinner=/usr/share/plymouth/themes/spinner

for img in entry bullet lock capslock keyboard keymap-render; do
    cp "$spinner/$img.png" "$theme/$img.png"
done

# Không -R: dựng lại initramfs là việc của module initramfs, làm một lần.
plymouth-set-default-theme samhie
[[ "$(plymouth-set-default-theme)" == samhie ]]
