# shellcheck shell=bash
# Tên tab Konsole = GỐC + thư mục cuối (user chốt 2026-10-09). Bash gửi tiêu
# đề (OSC 0) mỗi lần hiện dấu nhắc; profile Samhie đặt LocalTabTitleFormat=%w
# ("tiêu đề do shell đặt") để tab hiện nó. Konsole tự có %d / %D nhưng không
# rút gọn kiểu này được.
#
# Gốc là chỗ GẦN NHẤT trong các chỗ sau:
#   - dự án git (thư mục có .git, trừ chính home) → tên dự án, giữ nguyên chữ
#   - ổ Storage (/mnt/storage, hoặc ~/Storage khi máy không có ổ riêng) → Storage
#   - home → Home (như sidebar Dolphin)
#   - còn lại: thư mục đầu tiên tính từ / (/etc, /usr…)
# Sau gốc: không có gì / một tầng / "…" + tầng cuối.
#
#   ~                               → Home
#   ~/Projects                      → Home/Projects
#   ~/Download/a/b                  → Home/…/b
#   ~/Projects/samhie-distro        → samhie-distro
#   ~/Projects/samhie-distro/files  → samhie-distro/files
#   …/samhie-distro/files/…/quickshell → samhie-distro/…/quickshell
#   /mnt/storage/Docs               → Storage/Docs
#   /etc, /usr/share                → /etc, /usr/share
#   /usr/share/icons/Samhie         → /usr/…/Samhie
#
# Fedora (/etc/bashrc) cũng gửi tiêu đề "user@host:…" qua PROMPT_COMMAND;
# hàm này nối vào CUỐI mảng nên tiêu đề của mình đè lên.

if [ -n "${BASH_VERSION-}" ] && [ -n "${PS1-}" ] && [ "${TERM:-dumb}" != dumb ]; then
    __samhie_title() {
        # Ký tự điều khiển trong tên thư mục không được lọt vào chuỗi gửi
        # terminal (tên thư mục có ESC là tự viết được lệnh terminal).
        local p=${PWD//[[:cntrl:]]/?} home=${HOME%/} root= rest= d title

        # Fedora Atomic: /home → /var/home, /mnt → /var/mnt. `cd /home/x`
        # thì PWD giữ /home/x — đưa về một dạng trước khi so.
        case $p in
            /home|/home/*|/mnt|/mnt/*) [ "$p" -ef "/var$p" ] && p=/var$p ;;
        esac

        # Dự án git gần nhất: dò .git (thư mục, hoặc file với worktree /
        # submodule) từ đây lên — chỉ stat, không chạy lệnh git.
        d=$p
        while [ -n "$d" ]; do
            if [ "$d" != "$home" ] && [ -e "$d/.git" ]; then
                root=${d##*/} rest=${p#"$d"}
                break
            fi
            d=${d%/*}
        done

        if [ -z "$root" ]; then
            case $p in
                /var/mnt/storage|/var/mnt/storage/*) root=Storage rest=${p#/var/mnt/storage} ;;
                "$home/Storage"|"$home/Storage"/*)   root=Storage rest=${p#"$home/Storage"} ;;
                "$home"|"$home"/*)                   root=Home rest=${p#"$home"} ;;
                /)                                   root=/ ;;
                *)  rest=${p#/*/}
                    [ "$rest" = "$p" ] && rest=
                    root=${p%"/$rest"} ;;
            esac
        fi

        rest=${rest#/}
        case $rest in
            '')  title=$root ;;
            */*) title=$root/…/${rest##*/} ;;
            *)   title=$root/$rest ;;
        esac
        printf '\e]0;%s\a' "$title"

        # Konsole đổi tab NGAY chỉ khi chuỗi tên tab đang hiện có %w — mà cứ
        # 2 s sau khi gõ phím, SessionController::snapshot (Konsole 26.08) lại
        # ghi đè chuỗi đó bằng tên đã thay xong → tab chậm 1–2 s, lúc mở ghi
        # "Samhie" (tên profile). OSC 30 (riêng Konsole) đặt thẳng chuỗi tên
        # tab VÀ định dạng tab cục bộ → đổi ngay, snapshot sau cũng ra đúng
        # tên đó. Cái giá: Rename Tab của người dùng bị đè ở dấu nhắc kế.
        if [ -n "${KONSOLE_VERSION-}" ]; then
            printf '\e]30;%s\a' "$title"
        fi
    }
    PROMPT_COMMAND+=(__samhie_title)
fi
