#!/usr/bin/bash
# Bật Num Lock trong initramfs, trước ô mật khẩu ổ mã hoá (Plymouth). Nhân
# Linux khởi động với Num Lock TẮT dù BIOS đã bật — gõ mật khẩu có số ở bàn
# phím số thì không ra gì (user 2026-10-23, máy thật). Giống hook "numlock"
# của mkinitcpio bên Arch: `setleds -D +num` cho các tty.
# Gọi bởi dracut (dracut.conf.d/90-samhie-numlock.conf thêm module này).

check() {
    require_binaries setleds || return 1
    return 0
}

depends() {
    echo systemd
}

install() {
    inst_multiple setleds
    inst_simple "$moddir/samhie-numlock.service" "$systemdsystemunitdir/samhie-numlock.service"
    $SYSTEMCTL -q --root "$initdir" add-wants sysinit.target samhie-numlock.service
}
