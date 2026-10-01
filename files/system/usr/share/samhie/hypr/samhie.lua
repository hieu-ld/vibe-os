-- Cấu hình Hyprland MẶC ĐỊNH của Samhie.
--
-- File này nằm trong image (/usr/share/samhie/hypr/samhie.lua), chỉ đọc, và
-- được thay mới mỗi lần image cập nhật. Người dùng KHÔNG sửa file này; họ sửa
-- ~/.config/hypr/hyprland.lua — file đó nạp file này trước rồi mới tới phần
-- riêng của họ, nên cái gì họ đặt lại sẽ đè lên mặc định ở đây.
--
-- Cú pháp: Hyprland 0.56 cấu hình bằng Lua. Mẫu gốc của upstream:
-- https://github.com/hyprwm/Hyprland/blob/v0.56.2/example/hyprland.lua

------------------
---- BẢNG MÀU ----
------------------

-- Sinh từ theme/palette.toml (nguồn màu duy nhất của Samhie) — đổi màu thì sửa
-- file đó rồi chạy tools/gen-theme.py, đừng ghi mã màu thẳng vào đây.
local color = require("/usr/share/samhie/hypr/colors")

-------------------
---- TUỲ CHỌN ----
-------------------

-- Những thứ người dùng đổi bằng một dòng trong ~/.config/hypr/hyprland.lua:
--     local samhie = require("/usr/share/samhie/hypr/samhie")
--     samhie.wallpaper = "~/Storage/Pictures/nui.jpg"   -- có ổ Storage riêng: "/mnt/storage/Pictures/…"
-- Chúng được ĐỌC lúc Hyprland khởi động / nạp lại cấu hình (hl.on bên dưới),
-- tức là sau khi cả file của người dùng đã chạy xong, nên gán ở đâu sau dòng
-- require cũng được.
local samhie = {
    -- Hình nền: file PNG/JPG. Mặc định là tranh núi đêm của Samhie
    -- (tools/build-wallpaper.py). Đường dẫn bắt đầu bằng ~/ được hiểu là $HOME.
    wallpaper = "/usr/share/backgrounds/samhie/samhie.png",
    -- Giao diện Quickshell (thanh trên…): thư mục chứa shell.qml. Muốn tự sửa
    -- thì chép cả thư mục này về, vd ~/.config/quickshell/samhie, rồi trỏ vào
    -- đó. Đặt false là không chạy (không có thanh trên).
    shell = "/usr/share/samhie/quickshell",
}

-------------------
---- CHƯƠNG TRÌNH ----
-------------------

local terminal    = "konsole"
local fileManager = "dolphin"

-------------------
---- MÀN HÌNH ----
-------------------

hl.monitor({
    output   = "",
    mode     = "preferred",
    position = "auto",
    scale    = "auto",
})

--------------------------
---- BIẾN MÔI TRƯỜNG ----
--------------------------

-- Con trỏ Samhie (user vẽ; sinh bằng tools/build-cursors.py). Hyprland vẽ bằng
-- bản hyprcursor, app XWayland và app tự nạp con trỏ dùng bản XCursor — cùng
-- một theme ở /usr/share/icons/Samhie-cursors. 24 = cỡ khung user vẽ.
hl.env("XCURSOR_THEME", "Samhie-cursors")
hl.env("HYPRCURSOR_THEME", "Samhie-cursors")
hl.env("XCURSOR_SIZE", "24")
hl.env("HYPRCURSOR_SIZE", "24")

-- Ứng dụng Qt (Dolphin, Konsole) lấy giao diện từ kdeglobals như trong Plasma:
-- màu, icon, font, và widget style (Kvantum ở giai đoạn 4). Mặc định của Samhie
-- ở /etc/xdg/kdeglobals, người dùng đè trong ~/.config/kdeglobals.
hl.env("QT_QPA_PLATFORMTHEME", "kde")

-- Gõ tiếng Việt. App GTK, Electron, Edge nói chuyện với fcitx5 qua giao thức
-- text-input của Hyprland. App Qt (Konsole, Dolphin) thì vì QT_IM_MODULE=fcitx
-- mà đi qua D-Bus, và ô báo kiểu gõ do fcitx5-qt vẽ phía app (đo 2026-09-30) —
-- xem bước 0b trong /usr/libexec/samhie/user-setup. XMODIFIERS cho app X11.
-- Cố ý KHÔNG đặt GTK_IM_MODULE — theo wiki fcitx5, đặt nó trên Wayland làm
-- app GTK4 bỏ qua text-input và gõ lỗi.
hl.env("XMODIFIERS", "@im=fcitx")
hl.env("QT_IM_MODULE", "fcitx")

-- App Electron (VS Code) chạy Wayland thật thay vì XWayland.
hl.env("ELECTRON_OZONE_PLATFORM_HINT", "auto")

-------------------
---- TỰ KHỞI ĐỘNG ----
-------------------

-- hypridle đọc ~/.config/hypr/hypridle.conf nếu người dùng có, không thì dùng
-- bản mặc định của Samhie. Không có file nào thì hypridle không chạy, tức là
-- máy KHÔNG tự khoá — nên luôn phải có bản mặc định.
local idleCmd = "test -f ~/.config/hypr/hypridle.conf && exec hypridle || exec hypridle -c /etc/xdg/hypr/hypridle.conf"

-- Hình nền do swaybg vẽ (TẠM, giai đoạn 3 Quickshell làm thay). Lệnh chạy qua
-- shell nên đường dẫn được đặt trong ngoặc kép, thoát các ký tự đặc biệt.
local function wallpaperCmd()
    local path = samhie.wallpaper:gsub('[\\"$`]', "\\%0")
    path = path:gsub("^~/", "$HOME/")
    return 'pkill -x swaybg; exec swaybg -m fill -i "' .. path .. '"'
end

-- -n: đang có một bản chạy rồi thì không mở thêm. SAMHIE_WALLPAPER: màn
-- tổng quan vẽ hình nền trong ô màn ảo (đổi hình nền thì phải chạy lại
-- Quickshell mới thấy trong tổng quan).
local function quoted(path)
    return '"' .. path:gsub('[\\"$`]', "\\%0"):gsub("^~/", "$HOME/") .. '"'
end
local function shellCmd()
    return "SAMHIE_WALLPAPER=" .. quoted(samhie.wallpaper) .. " exec quickshell -n -p " .. quoted(samhie.shell)
end

hl.on("hyprland.start", function()
    hl.exec_cmd(wallpaperCmd())
    if samhie.shell then hl.exec_cmd(shellCmd()) end
    hl.exec_cmd("/usr/libexec/kf6/polkit-kde-authentication-agent-1")
    hl.exec_cmd("fcitx5 -d --replace")
    hl.exec_cmd("mako")
    hl.exec_cmd("wl-paste --watch cliphist store")
    hl.exec_cmd(idleCmd)
end)

-- Đổi samhie.wallpaper rồi lưu file là Hyprland tự nạp lại cấu hình — vẽ lại
-- hình nền để thấy ngay, khỏi phải đăng xuất.
hl.on("config.reloaded", function()
    hl.exec_cmd(wallpaperCmd())
end)

-------------------
---- GIAO DIỆN ----
-------------------

hl.config({
    general = {
        gaps_in     = 4,
        gaps_out    = 8,
        border_size = 1,

        col = {
            active_border   = "rgba(" .. color.accent .. "aa)",
            inactive_border = "rgba(" .. color.subSecondary .. "ff)",
        },

        resize_on_border = true,
        allow_tearing    = false,
        layout           = "dwindle",
    },

    decoration = {
        -- 12px như Windows 11. Trên Plasma phải build Klassy mới có; ở đây một dòng.
        rounding       = 12,
        rounding_power = 2,

        active_opacity   = 1.0,
        inactive_opacity = 1.0,

        shadow = {
            enabled      = true,
            range        = 12,
            render_power = 3,
            color        = 0x66000000,
        },

        -- Blur chỉ hiện ra sau những cửa sổ có nền trong suốt. Giai đoạn 4 theme
        -- Kvantum sẽ cho Dolphin/Konsole nền hơi trong để ăn hiệu ứng này.
        blur = {
            enabled  = true,
            size     = 6,
            passes   = 2,
            vibrancy = 0.17,
        },
    },

    animations = {
        enabled = true,
    },

    dwindle = {
        preserve_split = true,
    },

    misc = {
        -- Tắt hình nền anime và logo mặc định của Hyprland. Hình nền thật do
        -- swaybg vẽ (TỰ KHỞI ĐỘNG ở trên); màu này chỉ hiện khi file ảnh hỏng.
        force_default_wallpaper = 0,
        disable_hyprland_logo   = true,
        background_color        = "rgb(" .. color.bar .. ")",
        -- Màn hình khoá Quickshell chết giữa chừng thì phiên vẫn khoá; cho
        -- /usr/libexec/samhie/lock chạy một bản mới nhận lại, khỏi phải
        -- khởi động lại máy.
        allow_session_lock_restore = true,
    },

    input = {
        kb_layout    = "us",
        follow_mouse = 1,
        sensitivity  = 0,
        touchpad = {
            natural_scroll = true,
        },
    },
})

-- Chuyển động: giữ đúng bộ đường cong của mẫu upstream, chỉnh sau khi đã thấy tận mắt.
hl.curve("easeOutQuint", { type = "bezier", points = { {0.23, 1},  {0.32, 1} } })
hl.curve("linear",       { type = "bezier", points = { {0, 0},     {1, 1}    } })
hl.curve("almostLinear", { type = "bezier", points = { {0.5, 0.5}, {0.75, 1} } })
hl.curve("quick",        { type = "bezier", points = { {0.15, 0},  {0.1, 1}  } })
hl.curve("easy",         { type = "spring", mass = 1, stiffness = 238.1191, dampening = 24.21279333 })

hl.animation({ leaf = "global",     enabled = true, speed = 10,   bezier = "default" })
hl.animation({ leaf = "border",     enabled = true, speed = 5.39, bezier = "easeOutQuint" })
hl.animation({ leaf = "windows",    enabled = true, speed = 4.79, spring = "easy" })
hl.animation({ leaf = "windowsIn",  enabled = true, speed = 4.1,  spring = "easy",   style = "popin 87%" })
hl.animation({ leaf = "windowsOut", enabled = true, speed = 1.49, bezier = "linear", style = "popin 87%" })
hl.animation({ leaf = "fadeIn",     enabled = true, speed = 1.73, bezier = "almostLinear" })
hl.animation({ leaf = "fadeOut",    enabled = true, speed = 1.46, bezier = "almostLinear" })
hl.animation({ leaf = "fade",       enabled = true, speed = 3.03, bezier = "quick" })
hl.animation({ leaf = "layers",     enabled = true, speed = 3.81, bezier = "easeOutQuint" })
hl.animation({ leaf = "workspaces", enabled = true, speed = 1.94, bezier = "almostLinear", style = "fade" })

-- Lớp chụp màn hình (Quickshell, /usr/share/samhie/quickshell/screenshot-*.qml)
-- hiện khung hình đã đông đè lên chính màn hình đó: có hiệu ứng mở là lộ ngay.
-- Ảnh xem trước tự mờ dần trong QML.
hl.layer_rule({ match = { namespace = "^samhie-screenshot$" }, no_anim = true })
-- Lớp phủ của popup thanh trên (lịch, menu nguồn, menu khay) phủ cả màn
-- hình: hiệu ứng mở layer sẽ trượt/phóng cả lớp — tắt, popup hiện ngay.
hl.layer_rule({ match = { namespace = "^samhie-popup$" }, no_anim = true })
-- Màn tổng quan hiện/ẩn bằng mờ dần (mặc định layer trượt vào từ mép).
hl.layer_rule({ match = { namespace = "^samhie-overview$" }, animation = "fade" })
-- Dock trên desktop tự trượt/mờ trong QML (quickshell/dock/DesktopDock.qml).
hl.layer_rule({ match = { namespace = "^samhie-dock$" }, no_anim = true })

hl.gesture({
    fingers   = 3,
    direction = "horizontal",
    action    = "workspace",
})

------------------
---- PHÍM TẮT ----
------------------

-- Theo thói quen Windows ở những chỗ Windows đã có phím quen tay.
local mod = "SUPER"

hl.bind(mod .. " + E",         hl.dsp.exec_cmd(fileManager))  -- Win+E: quản lý file
-- Màn tổng quan (Quickshell, /usr/share/samhie/quickshell/overview): ô tìm
-- kiếm trong đó tìm app luôn. Nhấn rồi THẢ riêng phím Win là mở/đóng như
-- Ubuntu; Win+phím khác không kích hoạt nó.
local overview = hl.dsp.global("samhie:overview")
hl.bind(mod .. " + SUPER_L",   hl.dsp.global("samhie:overview-key"), { release = true })
hl.bind(mod .. " + R",         overview)                      -- Win+R: chạy app
hl.bind(mod .. " + SPACE",     overview)
hl.bind(mod .. " + L",         hl.dsp.exec_cmd("loginctl lock-session"))  -- Win+L: khoá máy
hl.bind("CTRL + ALT + T",      hl.dsp.exec_cmd(terminal))     -- quen tay từ Ubuntu
hl.bind(mod .. " + T",         hl.dsp.exec_cmd(terminal))

-- Win+V: lịch sử clipboard.
hl.bind(mod .. " + V",         hl.dsp.exec_cmd("cliphist list | fuzzel --dmenu | cliphist decode | wl-copy"))
-- Chụp màn hình kiểu Windows: tự lưu vào Storage/Pictures/Screenshots + copy
-- vào clipboard; bấm vào thông báo để vẽ lên ảnh (swappy).
hl.bind(mod .. " + SHIFT + S", hl.dsp.exec_cmd("/usr/libexec/samhie/screenshot region"))  -- chọn vùng
hl.bind("Print",               hl.dsp.exec_cmd("/usr/libexec/samhie/screenshot"))         -- cả màn hình

hl.bind("ALT + F4",            hl.dsp.window.close())
hl.bind(mod .. " + Q",         hl.dsp.window.close())
hl.bind(mod .. " + F",         hl.dsp.window.float({ action = "toggle" }))
hl.bind(mod .. " + J",         hl.dsp.layout("togglesplit"))
hl.bind(mod .. " + SHIFT + M", hl.dsp.exec_cmd("hyprctl dispatch 'hl.dsp.exit()'"))  -- đăng xuất

hl.bind(mod .. " + left",  hl.dsp.focus({ direction = "left" }))
hl.bind(mod .. " + right", hl.dsp.focus({ direction = "right" }))
hl.bind(mod .. " + up",    hl.dsp.focus({ direction = "up" }))
hl.bind(mod .. " + down",  hl.dsp.focus({ direction = "down" }))

for i = 1, 10 do
    local key = i % 10 -- phím 0 là workspace 10
    hl.bind(mod .. " + " .. key,         hl.dsp.focus({ workspace = i }))
    hl.bind(mod .. " + SHIFT + " .. key, hl.dsp.window.move({ workspace = i }))
end

hl.bind(mod .. " + mouse_down", hl.dsp.focus({ workspace = "e+1" }))
hl.bind(mod .. " + mouse_up",   hl.dsp.focus({ workspace = "e-1" }))
hl.bind(mod .. " + mouse:272",  hl.dsp.window.drag(),   { mouse = true })
hl.bind(mod .. " + mouse:273",  hl.dsp.window.resize(), { mouse = true })

-- Phím âm lượng, độ sáng, nhạc — `locked` để vẫn bấm được lúc đang khoá màn hình.
hl.bind("XF86AudioRaiseVolume",  hl.dsp.exec_cmd("wpctl set-volume -l 1 @DEFAULT_AUDIO_SINK@ 5%+"), { locked = true, repeating = true })
hl.bind("XF86AudioLowerVolume",  hl.dsp.exec_cmd("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-"),      { locked = true, repeating = true })
hl.bind("XF86AudioMute",         hl.dsp.exec_cmd("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle"),     { locked = true })
hl.bind("XF86AudioMicMute",      hl.dsp.exec_cmd("wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle"),   { locked = true })
hl.bind("XF86MonBrightnessUp",   hl.dsp.exec_cmd("brightnessctl -e4 -n2 set 5%+"),                  { locked = true, repeating = true })
hl.bind("XF86MonBrightnessDown", hl.dsp.exec_cmd("brightnessctl -e4 -n2 set 5%-"),                  { locked = true, repeating = true })
hl.bind("XF86AudioNext",         hl.dsp.exec_cmd("playerctl next"),       { locked = true })
hl.bind("XF86AudioPause",        hl.dsp.exec_cmd("playerctl play-pause"), { locked = true })
hl.bind("XF86AudioPlay",         hl.dsp.exec_cmd("playerctl play-pause"), { locked = true })
hl.bind("XF86AudioPrev",         hl.dsp.exec_cmd("playerctl previous"),   { locked = true })

------------------------
---- LUẬT CỬA SỔ ----
------------------------

-- Mẫu upstream khuyên dùng: app tự ý phóng to thì bỏ qua, để Hyprland xếp.
hl.window_rule({
    name  = "suppress-maximize-events",
    match = { class = ".*" },
    suppress_event = "maximize",
})

-- Sửa lỗi kéo thả của app XWayland (cũng lấy từ mẫu upstream).
hl.window_rule({
    name  = "fix-xwayland-drags",
    match = {
        class      = "^$",
        title      = "^$",
        xwayland   = true,
        float      = true,
        fullscreen = false,
        pin        = false,
    },
    no_focus = true,
})

return samhie
