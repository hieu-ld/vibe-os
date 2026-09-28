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
---- CHƯƠNG TRÌNH ----
-------------------

local terminal    = "konsole"
local fileManager = "dolphin"
local launcher    = "fuzzel"   -- TẠM, giai đoạn 3 thay bằng launcher Quickshell

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

hl.env("XCURSOR_SIZE", "24")
hl.env("HYPRCURSOR_SIZE", "24")

-- Ứng dụng Qt (Dolphin, Konsole) lấy giao diện từ qt6ct, qt6ct trỏ tới Kvantum.
hl.env("QT_QPA_PLATFORMTHEME", "qt6ct")

-- Gõ tiếng Việt. Trên Wayland, app GTK và Qt đời mới nói chuyện với fcitx5 qua
-- giao thức text-input của Hyprland; hai biến này dành cho app Qt/XWayland đời
-- cũ. Cố ý KHÔNG đặt GTK_IM_MODULE — theo wiki fcitx5, đặt nó trên Wayland làm
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

hl.on("hyprland.start", function()
    hl.exec_cmd("/usr/libexec/kf6/polkit-kde-authentication-agent-1")
    hl.exec_cmd("fcitx5 -d --replace")
    hl.exec_cmd("mako")
    hl.exec_cmd("wl-paste --watch cliphist store")
    hl.exec_cmd(idleCmd)
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
        -- Tắt hình nền anime và logo mặc định của Hyprland; nền là màu trơn cho
        -- tới khi giai đoạn 2 mang wallpaper Samhie sang.
        force_default_wallpaper = 0,
        disable_hyprland_logo   = true,
        background_color        = "rgb(" .. color.bar .. ")",
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
hl.bind(mod .. " + R",         hl.dsp.exec_cmd(launcher))     -- Win+R: chạy app
hl.bind(mod .. " + SPACE",     hl.dsp.exec_cmd(launcher))
hl.bind(mod .. " + L",         hl.dsp.exec_cmd("loginctl lock-session"))  -- Win+L: khoá máy
hl.bind("CTRL + ALT + T",      hl.dsp.exec_cmd(terminal))     -- quen tay từ Ubuntu
hl.bind(mod .. " + T",         hl.dsp.exec_cmd(terminal))

-- Win+V: lịch sử clipboard. Win+Shift+S: chụp một vùng màn hình vào clipboard.
hl.bind(mod .. " + V",         hl.dsp.exec_cmd("cliphist list | fuzzel --dmenu | cliphist decode | wl-copy"))
hl.bind(mod .. " + SHIFT + S", hl.dsp.exec_cmd("grim -g \"$(slurp)\" - | wl-copy"))
hl.bind("Print",               hl.dsp.exec_cmd("grim - | swappy -f -"))

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
