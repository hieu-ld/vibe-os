# shellcheck shell=bash
# SINH TỰ ĐỘNG từ theme/palette.toml bởi tools/gen-theme.py — đừng sửa tay.
# Dấu nhắc lệnh của Samhie: chỉ còn "$ " màu cyan (user chốt 2026-09-28: bỏ cả
# user@host lẫn đường dẫn — tên tab Konsole hiện thư mục hiện tại thay, xem
# LocalTabTitleFormat trong /usr/share/konsole/Samhie.profile).
#
# \$ chứ không phải chữ $: bash in "$" cho người thường và "#" cho root. Đăng
# nhập root mà vẫn thấy "$" là một cách hỏng nguy hiểm (bản 1.0).
#
# Màu = Color6 của [terminal] trong theme/palette.toml, IN ĐẬM (user thấy nét
# thường mảnh quá, 2026-09-30). Đậm + màu 8-ô (01;36) thì Konsole 26.08 đổi sang
# Color6Intense (đo pixel ra #52d6fa). Nên terminal nào báo có màu 24-bit
# (COLORTERM=truecolor, Konsole có) thì ghi thẳng mã RGB: màu RGB không bị
# đổi. Nét đậm cần BoldIntense=true trong Samhie.profile. Terminal khác
# (tty…) dùng 36 thường, không đậm.
#
# Thứ tự: /etc/profile.d chạy theo tên file, nên file này chạy SAU
# bash-color-prompt.sh (Fedora) và đè PS1 của nó; nhưng giữ tiền tố/hậu tố mà
# 70-systemd-shell-extra.sh gắn vào (vd run0 báo đang là root). Người dùng muốn
# dấu nhắc khác thì đặt PS1 trong ~/.bashrc — file đó chạy sau cùng.

if [ -n "${BASH_VERSION-}" ] && [ -n "${PS1-}" ]; then
    case ${COLORTERM-}:${TERM:-dumb} in
        *:dumb)               PS1='\$ ' ;;
        truecolor:*|24bit:*)  PS1='\[\e[1;38;2;0;192;232m\]\$\[\e[0m\] ' ;;
        *)                    PS1='\[\e[36m\]\$\[\e[0m\] ' ;;
    esac
    PS1="${SHELL_PROMPT_PREFIX-}$PS1${SHELL_PROMPT_SUFFIX-}"

    # Một dòng trống giữa các lệnh (user 2026-09-30: nhìn thoáng; terminal là
    # lưới ký tự nên "khoảng" nhỏ nhất là nguyên một dòng — lệnh và phần trả
    # về của nó vẫn sát nhau). Chỉ khi VỪA chạy lệnh: PS0 hiện ra lúc bash bắt
    # đầu chạy một lệnh (Enter trên dòng trống thì không); ${ …; } của bash 5.3
    # chạy ngay trong shell mà không in gì, nên đặt cờ được. (\# thì không
    # dùng được: bash không tăng nó sau lệnh ĐẦU TIÊN — đo 2026-09-30.)
    # clear/reset xoá cờ, khỏi mở màn hình mới bằng một dòng trống.
    # Nối vào PS0 / PROMPT_COMMAND có sẵn: systemd (80-systemd-osc-context.sh)
    # đã gắn vào hai chỗ này trước và đọc $? trước mình.
    if (( BASH_VERSINFO[0] > 5 || (BASH_VERSINFO[0] == 5 && BASH_VERSINFO[1] >= 3) )); then
        PS0="${PS0-}"'${ __samhie_ran=1; }'
        __samhie_gap() {
            if [ -n "${__samhie_ran-}" ]; then
                printf '\n'
                unset __samhie_ran
            fi
        }
        PROMPT_COMMAND+=(__samhie_gap)
        clear() { command clear "$@"; unset __samhie_ran; }
        reset() { command reset "$@"; unset __samhie_ran; }
    fi
fi
