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

firewall-offline-cmd --set-default-zone=public
firewall-offline-cmd --zone=public --remove-service=ssh
