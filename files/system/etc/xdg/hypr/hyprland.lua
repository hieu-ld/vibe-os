-- Cấu hình Hyprland cho người dùng CHƯA có file riêng.
--
-- Hyprland tìm ~/.config/hypr/hyprland.lua trước; không thấy mới đọc file này.
-- Nhờ vậy tài khoản tạo từ trước khi rebase sang Samhie (không nhận /etc/skel)
-- vẫn vào đúng giao diện Samhie, thay vì cấu hình tự sinh của Hyprland.
--
-- Đừng sửa file này. Muốn chỉnh thì tạo ~/.config/hypr/hyprland.lua, dòng đầu
-- là `local samhie = require("/usr/share/samhie/hypr/samhie")`, rồi viết phần
-- của bạn sau nó. Tạo xong thì ĐĂNG XUẤT rồi đăng nhập lại một lần (hoặc
-- `hyprctl reload full-reset`): Hyprland nhớ đường dẫn file chính từ lúc khởi
-- động, `hyprctl reload` thường vẫn đọc file này (đọc mã nguồn 0.56.2,
-- Jeremy.cpp). Từ đó về sau, lưu file là tự nạp lại.

require("/usr/share/samhie/hypr/samhie")
