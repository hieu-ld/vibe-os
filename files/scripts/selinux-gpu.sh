#!/usr/bin/env bash
# Cài quy tắc SELinux cho container dùng card NVIDIA (trợ lý S-bot, xem
# /usr/share/selinux/packages/samhie/samhie-gpu-container.cil để biết lý do).
#
# -n: chỉ dựng lại policy trong kho (/etc/selinux), không nạp vào nhân — lúc
# build không có SELinux đang chạy. Máy đang dùng nhận policy mới ở lần khởi
# động sau update như mọi thay đổi policy khác.
set -oue pipefail
semodule -n -i /usr/share/selinux/packages/samhie/samhie-gpu-container.cil
semodule -n -l | grep -qx samhie-gpu-container
