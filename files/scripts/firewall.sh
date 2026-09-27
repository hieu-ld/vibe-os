#!/usr/bin/env bash
# Đặt tường lửa về vùng `public` và đóng cổng SSH.
#
# Vì sao: Fedora cho máy desktop dùng vùng FedoraWorkstation, vùng này MỞ SẴN
# mọi cổng từ 1025 đến 65535 cho kết nối từ ngoài vào. Samhie là máy cá nhân,
# không chạy dịch vụ nào cho máy khác gọi vào, nên chặn hết là hợp lý.
# Vùng `public` chỉ cho vào ssh và dhcpv6-client; bỏ luôn ssh.
#
# Cần mở cổng cho app nào (ví dụ KDE Connect) thì mở đích danh app đó.
set -oue pipefail

# firewalld báo LỖI khi đặt lại đúng giá trị đang có (ZONE_ALREADY_SET,
# NOT_ENABLED), nên hỏi trước rồi mới đổi. base-main vốn đã để `public`
# (đo 2026-09-27) — vẫn giữ bước này phòng khi image nền đổi mặc định.
if [[ "$(firewall-offline-cmd --get-default-zone)" != public ]]; then
    firewall-offline-cmd --set-default-zone=public
fi
if firewall-offline-cmd --zone=public --query-service=ssh; then
    firewall-offline-cmd --zone=public --remove-service-from-zone=ssh
fi

# Kiểm lại: sai thì dừng build, không ra image có tường lửa hở
if [[ "$(firewall-offline-cmd --get-default-zone)" != public ]] ||
   firewall-offline-cmd --zone=public --query-service=ssh; then
    echo "firewall.sh: tường lửa chưa đúng cấu hình" >&2
    exit 1
fi
