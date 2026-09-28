-- Cấu hình Hyprland CỦA BẠN.
--
-- Dòng dưới nạp cấu hình mặc định của Samhie (nằm trong hệ thống, tự cập nhật
-- theo image). Mọi thứ bạn viết SAU dòng đó sẽ đè lên mặc định — muốn đổi gì
-- thì viết lại đúng mục đó ở đây, không cần chép cả file mặc định về.
--
-- Xem mặc định đang có gì: /usr/share/samhie/hypr/samhie.lua
-- Tài liệu: https://wiki.hypr.land/Configuring/

local samhie = require("/usr/share/samhie/hypr/samhie")

-- Ví dụ: hình nền riêng (PNG/JPG)
-- samhie.wallpaper = "~/Pictures/wallpaper.jpg"

-- Ví dụ: đổi bo góc cửa sổ
-- hl.config({ decoration = { rounding = 16 } })

-- Ví dụ: màn hình và độ phân giải cụ thể (xem tên màn hình bằng `hyprctl monitors`)
-- hl.monitor({ output = "HDMI-A-1", mode = "1920x1200@60", position = "0x0", scale = 1 })
