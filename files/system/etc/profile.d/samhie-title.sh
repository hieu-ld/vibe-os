# shellcheck shell=bash
# Tên tab Konsole = đường dẫn RÚT GỌN của thư mục hiện tại (user chốt
# 2026-10-09). Bash gửi tiêu đề (OSC 0) mỗi lần hiện dấu nhắc; profile Samhie
# đặt LocalTabTitleFormat=%w ("tiêu đề do shell đặt") để tab hiện nó.
# Konsole tự có %d / %D nhưng không đổi được ~ thành tên tài khoản.
#
#   ~                          → Duchieu          (tên tài khoản, hoa chữ đầu)
#   ~/Download                 → Duchieu/Download
#   /mnt/storage (ổ Storage)   → Storage
#   /mnt/storage/Docs          → Storage/Docs
#   ~/Projects/samhie-distro   → …/Projects/samhie-distro  (tối đa 2 tầng cuối)
#   /etc, /usr/share           → /etc, /usr/share
#   /usr/share/icons/Samhie    → …/icons/Samhie
#
# Chỉ hoa chữ đầu của gốc (tài khoản, Storage); tên thư mục giữ nguyên để
# gõ `cd` theo được (Linux phân biệt hoa/thường).
#
# Fedora (/etc/bashrc) cũng gửi tiêu đề "user@host:đường dẫn" qua
# PROMPT_COMMAND; hàm này nối vào CUỐI mảng nên tiêu đề của mình đè lên.

if [ -n "${BASH_VERSION-}" ] && [ -n "${PS1-}" ] && [ "${TERM:-dumb}" != dumb ]; then
    __samhie_title() {
        # Ký tự điều khiển trong tên thư mục không được lọt vào chuỗi gửi
        # terminal (tên thư mục có ESC là tự viết được lệnh terminal).
        local p=${PWD//[[:cntrl:]]/?} home=${HOME%/} root= rest title
        local -a parts

        # Fedora Atomic: /home → /var/home, /mnt → /var/mnt. `cd /home/x`
        # thì PWD giữ /home/x — đưa về một dạng trước khi so.
        case $p in
            /home|/home/*|/mnt|/mnt/*) [ "$p" -ef "/var$p" ] && p=/var$p ;;
        esac

        case $p in
            /var/mnt/storage|/var/mnt/storage/*) root=Storage rest=${p#/var/mnt/storage} ;;
            "$home/Storage"|"$home/Storage"/*)   root=Storage rest=${p#"$home/Storage"} ;;
            "$home"|"$home"/*)                   root=${USER:-$(id -un)} rest=${p#"$home"} ;;
        esac

        if [ -n "$root" ]; then
            root=${root^}
            rest=${rest#/}
            IFS=/ read -ra parts <<< "$rest"
            case ${#parts[@]} in
                0) title=$root ;;
                1) title=$root/$rest ;;
                *) title=…/${parts[-2]}/${parts[-1]} ;;
            esac
        else
            IFS=/ read -ra parts <<< "${p#/}"
            if (( ${#parts[@]} <= 2 )); then
                title=$p
            else
                title=…/${parts[-2]}/${parts[-1]}
            fi
        fi
        printf '\e]0;%s\a' "$title"
    }
    PROMPT_COMMAND+=(__samhie_title)
fi
