-- Cấu hình Hyprland cho người dùng CHƯA có file riêng.
--
-- Hyprland tìm ~/.config/hypr/hyprland.lua trước; không thấy mới đọc file này.
-- Nhờ vậy tài khoản tạo từ trước khi rebase sang Samhie (không nhận /etc/skel)
-- vẫn vào đúng giao diện Samhie, thay vì cấu hình tự sinh của Hyprland.
--
-- Đừng sửa file này. Muốn chỉnh thì tạo ~/.config/hypr/hyprland.lua, dòng đầu
-- là dòng require bên dưới, rồi viết phần của bạn sau nó.

require("/usr/share/samhie/hypr/samhie")
