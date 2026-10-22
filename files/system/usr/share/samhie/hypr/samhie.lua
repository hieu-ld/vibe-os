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
-- Chia đôi màn hình kiểu Windows (Super + mũi tên) + cỡ cửa sổ nổi mặc định.
local snap = require("/usr/share/samhie/hypr/snap")
-- Toàn cục để Quickshell gọi được qua hl.dispatch (nút ⤢ thanh trên,
-- Windows.qml: `function() samhie_snap.toggle_maximize("…") end`).
samhie_snap = snap

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
    -- Hình nền: file PNG/JPG. Mặc định là logo Samhie trên nền sao
    -- (theme/wallpaper/, tools/build-wallpaper.py). Đường dẫn bắt đầu bằng ~/ được hiểu là $HOME.
    wallpaper = "/usr/share/backgrounds/samhie/samhie.png",
    -- Hình nền lúc bật chế độ sáng (ô Dark style của quick settings), vd
    --     samhie.wallpaper_light = "~/Storage/Pictures/bien.jpg"
    -- Không đặt: hình nền mặc định thì dùng bản sáng của logo
    -- (samhie-light.png); đã tự đổi `wallpaper` thì dùng luôn hình đó.
    wallpaper_light = nil,
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

-- Phần RIÊNG app Settings ghi (~/.config/hypr/displays.lua, input.lua): chỉ
-- là một bảng `return { … }`. Nạp bằng load() với môi trường RỖNG: file chỉ
-- trả về dữ liệu được, không gọi được hàm nào. Không có / hỏng → nil (về
-- mặc định), không làm hỏng cả cấu hình. Đọc bằng io.open (không require)
-- để Hyprland không theo dõi file: app ghi file là Hyprland KHÔNG nạp lại cấu
-- hình — app đã tự áp dụng rồi.
local function userTable(name)
    local config = os.getenv("XDG_CONFIG_HOME") or (os.getenv("HOME") .. "/.config")
    local f = io.open(config .. "/hypr/" .. name)
    if not f then
        return nil
    end
    local chunk = load(f:read("a"), name, "t", {})
    f:close()
    local ok, t = pcall(chunk or error)
    return ok and type(t) == "table" and t or nil
end

-------------------
---- MÀN HÌNH ----
-------------------

hl.monitor({
    output   = "",
    mode     = "preferred",
    position = "auto",
    scale    = "auto",
})

-- Màn hình người dùng chỉnh trong app Settings (trang Display) — phần RIÊNG,
-- ~/.config/hypr/displays.lua: `return { {output=…}, … }` (userTable ở trên).
-- hl.monitor viết trong hyprland.lua chạy sau nên vẫn đè lên.
for _, spec in ipairs(userTable("displays.lua") or {}) do
    if type(spec) == "table" and type(spec.output) == "string" then
        pcall(hl.monitor, spec)
    end
end

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

-- Ngày giờ kiểu Anh: 29/09/2026, giờ 24h (user 2026-10-05: Dolphin cột
-- Modified ra 9/29/26 kiểu Mỹ). Chữ vẫn tiếng Anh ("Today at 09:32"); không
-- locale nào có năm 2 chữ số + ngày trước tháng mà chữ tiếng Anh. Đồng hồ và
-- màn khoá Quickshell tự ghép chuỗi tiếng Việt, không đọc biến này.
hl.env("LC_TIME", "en_GB.UTF-8")

-- App Electron (người dùng tự cài, vd VS Code) chạy Wayland thật thay vì XWayland.
hl.env("ELECTRON_OZONE_PLATFORM_HINT", "auto")

-------------------
---- TỰ KHỞI ĐỘNG ----
-------------------

-- Tự khoá: /usr/libexec/samhie/idle chạy hypridle — ~/.config/hypr/hypridle.conf
-- của người dùng nếu có, không thì bản mặc định với thời gian khoá app
-- Settings lưu (trang Appearance). Không có file nào thì hypridle không chạy,
-- tức là máy KHÔNG tự khoá — nên luôn phải có bản mặc định.
local idleCmd = "exec /usr/libexec/samhie/idle"

-- Lệnh chạy qua shell nên đường dẫn được đặt trong ngoặc kép, thoát các ký
-- tự đặc biệt; ~/ thành $HOME/.
local function quoted(path)
    return '"' .. path:gsub('[\\"$`]', "\\%0"):gsub("^~/", "$HOME/") .. '"'
end

local DEFAULT_WALLPAPER = "/usr/share/backgrounds/samhie/samhie.png"
local DEFAULT_WALLPAPER_LIGHT = "/usr/share/backgrounds/samhie/samhie-light.png"
-- Hình app Settings chép về khi bấm Choose… (trang Appearance): symlink tới
-- bản chép. Xét lúc CHẠY lệnh (shell), không lúc nạp cấu hình — chọn hình
-- mới không cần nạp lại Hyprland.
local CUSTOM_WALLPAPER = '"${XDG_DATA_HOME:-$HOME/.local/share}/samhie/wallpaper"'

local function lightWallpaper()
    if samhie.wallpaper_light then return samhie.wallpaper_light end
    if samhie.wallpaper == DEFAULT_WALLPAPER then return DEFAULT_WALLPAPER_LIGHT end
    return samhie.wallpaper
end

-- Lệnh shell đặt $img = hình nền của một chế độ. Thứ tự: samhie.wallpaper(_light)
-- người dùng ghi trong hyprland.lua → hình chọn trong app Settings (dùng cho
-- cả hai chế độ) → mặc định của Samhie.
local function imgCmd(light)
    local img = light and lightWallpaper() or samhie.wallpaper
    local cmd = "img=" .. quoted(img) .. "; "
    local fromLua = samhie.wallpaper ~= DEFAULT_WALLPAPER or (light and samhie.wallpaper_light)
    if not fromLua then
        cmd = cmd .. "[ -e " .. CUSTOM_WALLPAPER .. " ] && img=" .. CUSTOM_WALLPAPER .. "; "
    end
    return cmd
end

-- Hình nền do swaybg vẽ (TẠM, giai đoạn 3 Quickshell làm thay), theo chế độ
-- sáng/tối. `light` nil (khởi động, nạp lại cấu hình, app Settings đổi
-- hình): đọc chế độ đã lưu ở settings.json (quicksettings/Settings.qml ghi).
-- Đổi hình: bật swaybg mới TRƯỚC, vẽ xong mới tắt cái cũ — tắt trước thì
-- chớp màu nền Hyprland.
local function wallpaperCmd(light)
    local pick
    if light == nil then
        pick = "if grep -qs '\"lightMode\": *true' "
            .. '"${XDG_STATE_HOME:-$HOME/.local/state}/samhie/settings.json"; then '
            .. imgCmd(true) .. "else " .. imgCmd(false) .. "fi; "
    else
        pick = imgCmd(light)
    end
    return pick .. 'old=$(pgrep -x swaybg); swaybg -m fill -i "$img" & sleep 1; [ -n "$old" ] && kill $old'
end
-- Chế độ sáng/tối đã lưu (quicksettings/Settings.qml ghi settings.json) — đọc
-- mỗi lần nạp cấu hình, nên nạp lại không làm viền về màu tối.
local function savedLight()
    local state = os.getenv("XDG_STATE_HOME") or (os.getenv("HOME") .. "/.local/state")
    local f = io.open(state .. "/samhie/settings.json")
    if not f then return false end
    local text = f:read("a")
    f:close()
    return text:find('"lightMode"%s*:%s*true') ~= nil
end

-- Viền cửa sổ theo chế độ: bản tối viền #393939 trên cửa sổ sáng thành nét
-- đen quanh app (container 2026-10-20).
local function borderColors(light)
    local c = "rgb(" .. (light and color.light or color).windowBorder .. ")"
    return { active_border = c, inactive_border = c }
end

-- Toàn cục để Quickshell gọi khi bấm đổi sáng/tối (quicksettings/Settings.qml):
-- `function() samhie_color_mode(true) end` — hình nền + viền cửa sổ.
function samhie_color_mode(light)
    hl.exec_cmd(wallpaperCmd(light))
    hl.config({ general = { col = borderColors(light) } })
end

-- Toàn cục cho app Settings (trang Appearance) gọi sau khi chọn hình nền /
-- về mặc định: vẽ lại hình nền theo chế độ đang lưu.
function samhie_wallpaper_refresh()
    hl.exec_cmd(wallpaperCmd())
end

-- -n: đang có một bản chạy rồi thì không mở thêm. SAMHIE_WALLPAPER(_LIGHT):
-- màn tổng quan vẽ hình nền trong ô màn ảo — đường dẫn THẬT (readlink -f:
-- hình chọn trong Settings là symlink; mỗi lần chọn là một bản chép mới tên
-- khác, nên Image của Qt không giữ ảnh cũ trong bộ nhớ đệm). Đổi hình lúc
-- shell đang chạy: app Settings báo qua IPC `wallpaper set` (shell.qml).
-- SAMHIE_SHELL: app mở từ shell (dock → Settings) biết gọi IPC vào đâu.
local function shellCmd()
    return imgCmd(false) .. 'dark=$(readlink -f "$img"); '
        .. imgCmd(true) .. 'light=$(readlink -f "$img"); '
        .. "SAMHIE_SHELL=" .. quoted(samhie.shell)
        .. ' SAMHIE_WALLPAPER="$dark" SAMHIE_WALLPAPER_LIGHT="$light"'
        .. " exec quickshell -n -p " .. quoted(samhie.shell)
end

hl.on("hyprland.start", function()
    hl.exec_cmd(wallpaperCmd())
    if samhie.shell then hl.exec_cmd(shellCmd()) end
    hl.exec_cmd("/usr/libexec/kf6/polkit-kde-authentication-agent-1")
    hl.exec_cmd("fcitx5 -d --replace")
    -- Lịch sử clipboard: xoá lịch sử cũ rồi ghi mọi lần copy (tối đa 50).
    hl.exec_cmd("/usr/libexec/samhie/clipboard watch")
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
        -- Viền tối như bản 1.0, CÙNG màu cho cửa sổ đang dùng và không dùng
        -- (user 2026-10-04): cửa sổ đang dùng phân biệt bằng cách làm tối cửa
        -- sổ KHÔNG dùng (decoration.dim_inactive bên dưới), thay viền cyan.
        border_size = 1,

        col = borderColors(savedLight()),

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

        -- Cửa sổ không dùng tối đi một chút: phủ đen với độ đục dim_strength.
        -- Ubuntu/Samhie 1.0 phân biệt bằng thanh tiêu đề + bóng (KDE/GNOME
        -- không làm tối nội dung) — Samhie 2 không có thanh tiêu đề.
        dim_inactive = true,
        dim_strength = 0.15,

        -- Bóng đổ xuống kiểu Windows 11 (user 2026-10-20): bản cũ toả đều bốn
        -- phía, đen 40%, 12px — trên hình nền sáng thành "quầng" quanh cửa sổ.
        -- Giờ lệch xuống 4px, toả 20px, nhạt 25% (0x40).
        shadow = {
            enabled      = true,
            range        = 20,
            render_power = 3,
            offset       = { 0, 4 },  -- dạng { x = , y = } stub cho phép nhưng Hyprland báo lỗi (đo)
            color        = "rgba(" .. color.shadow .. "40)",
        },

        -- Blur chỉ hiện ra sau những cửa sổ có nền trong suốt: Dolphin/Konsole
        -- nền đục 90% (palette.toml [window] opacity, theme Kvantum). User chọn
        -- 2026-10-03 "trong 90%, blur nhẹ" (bản vẽ không blur) → size 6 → 4.
        blur = {
            enabled  = true,
            size     = 4,
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
    },
})

-- Chuột + bàn phím: mặc định ở đây, người dùng chỉnh trong app Settings
-- (trang Mouse & Keyboard) → ~/.config/hypr/input.lua, chỉ ghi khoá khác
-- mặc định: `return { sensitivity = 0.3, touchpad = { tap_to_click = false } }`.
-- Chỉ nhận đúng các khoá dưới, đúng kiểu. Toàn cục để app gọi sau mỗi lần
-- ghi/xoá file: `function() samhie_input_apply() end` — áp lại TỪ mặc định,
-- nên xoá file là về mặc định ngay. input trong hyprland.lua chạy sau nên
-- vẫn đè lên lúc khởi động/nạp lại.
local inputDefaults = {
    sensitivity    = 0,       -- tốc độ con trỏ, -1…1 (libinput)
    natural_scroll = false,   -- chuột
    scroll_factor  = 1.0,     -- chuột
    repeat_delay   = 600,     -- ms giữ phím trước khi lặp
    repeat_rate    = 25,      -- lần lặp mỗi giây
    touchpad = {
        natural_scroll = true,
        tap_to_click   = true,
    },
}
-- Khoảng nhận cho số (ngoài khoảng / NaN → mặc định).
local inputRange = {
    sensitivity   = { -1, 1 },
    scroll_factor = { 0.1, 10 },
    repeat_delay  = { 100, 2000 },
    repeat_rate   = { 1, 100 },
}
local function mergeInput(defaults, user)
    local out = {}
    for k, v in pairs(defaults) do
        local u = nil
        if type(user) == "table" then
            u = user[k]
        end
        local range = inputRange[k]
        if type(v) == "table" then
            out[k] = mergeInput(v, u)
        elseif type(u) ~= type(v) or (range and not (u >= range[1] and u <= range[2])) then
            out[k] = v
        else
            out[k] = u
        end
    end
    return out
end
function samhie_input_apply()
    local input = mergeInput(inputDefaults, userTable("input.lua"))
    input.repeat_delay = math.floor(input.repeat_delay)
    input.repeat_rate = math.floor(input.repeat_rate)
    pcall(hl.config, { input = input })
end
samhie_input_apply()

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
-- Màn tổng quan tự làm hiệu ứng thu nhỏ kiểu Ubuntu trong QML
-- (quickshell/overview/Overview.qml) — Hyprland mà mờ/trượt thêm là chồng hai lớp.
hl.layer_rule({ match = { namespace = "^samhie-overview$" }, no_anim = true })
-- Dock trên desktop tự trượt/mờ trong QML (quickshell/dock/DesktopDock.qml).
hl.layer_rule({ match = { namespace = "^samhie-dock$" }, no_anim = true })
-- Popup thông báo: từng thẻ tự trượt vào trong QML (quickshell/notifications).
hl.layer_rule({ match = { namespace = "^samhie-notifications$" }, no_anim = true })
-- Hộp thoại phủ cả màn của app Settings ("Keep these display settings?"):
-- hiệu ứng mở layer sẽ trượt/phóng cả lớp nền tối — tắt, hiện ngay.
hl.layer_rule({ match = { namespace = "^samhie-dialog$" }, no_anim = true })
-- Hình nền (swaybg, namespace "wallpaper") đổi theo sáng/tối: bản mới lên
-- trên rồi tắt bản cũ — có hiệu ứng thì bản cũ mờ dần ĐÈ lên bản mới, chớp
-- lại hình cũ (đo container 2026-10-20). Không hiệu ứng: đổi gọn một nhịp.
hl.layer_rule({ match = { namespace = "^wallpaper$" }, no_anim = true })

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
-- NHẤN Win trái (chưa thả): bảo Quickshell dựng sẵn màn tổng quan ở lớp vô
-- hình, để lúc thả là chạy hiệu ứng ngay (khỏi chờ chụp hình cửa sổ). Nghe sự
-- kiện phím thay vì thêm bind: không đụng tới bind "khi thả" ở trên và mọi
-- tổ hợp Win+phím. 133 = mã XKB của Super_L (đo trong máy ảo 2026-10-02),
-- state 1 = nhấn (0 nhả, 2 lặp).
hl.on("input.keyboard.key", function(code, _, state)
    if code == 133 and state == 1 then
        hl.dispatch(hl.dsp.global("samhie:overview-prepare"))
    end
end)
hl.bind(mod .. " + R",         overview)                      -- Win+R: chạy app
hl.bind(mod .. " + SPACE",     overview)
hl.bind(mod .. " + L",         hl.dsp.exec_cmd("loginctl lock-session"))  -- Win+L: khoá máy
hl.bind("CTRL + ALT + T",      hl.dsp.exec_cmd(terminal))     -- quen tay từ Ubuntu
hl.bind(mod .. " + T",         hl.dsp.exec_cmd(terminal))

-- Win+V: lịch sử clipboard (Quickshell, quickshell/clipboard/).
hl.bind(mod .. " + V",         hl.dsp.global("samhie:clipboard"))
-- Chụp màn hình kiểu Windows: tự lưu vào Storage/Pictures/Screenshots + copy
-- vào clipboard; bấm vào thông báo để vẽ lên ảnh (swappy).
hl.bind(mod .. " + SHIFT + S", hl.dsp.exec_cmd("/usr/libexec/samhie/screenshot region"))  -- chọn vùng
hl.bind("Print",               hl.dsp.exec_cmd("/usr/libexec/samhie/screenshot"))         -- cả màn hình

hl.bind("ALT + F4",            hl.dsp.window.close())
hl.bind(mod .. " + Q",         hl.dsp.window.close())
hl.bind(mod .. " + J",         hl.dsp.layout("togglesplit"))
hl.bind(mod .. " + SHIFT + M", hl.dsp.exec_cmd("hyprctl dispatch 'hl.dsp.exit()'"))  -- đăng xuất

-- Win + mũi tên như Windows (snap.lua): ← → nửa trái/phải, ↑ phóng kín, ↓
-- về cửa sổ nổi. Thay Super+F (bật/tắt nổi) — user 2026-10-05.
hl.bind(mod .. " + left",  function() snap.snap("left") end)
hl.bind(mod .. " + right", function() snap.snap("right") end)
hl.bind(mod .. " + up",    snap.maximize)
hl.bind(mod .. " + down",  snap.restore)
-- Chuyển focus sang cửa sổ bên cạnh (trước là Super + mũi tên).
hl.bind(mod .. " + ALT + left",  hl.dsp.focus({ direction = "left" }))
hl.bind(mod .. " + ALT + right", hl.dsp.focus({ direction = "right" }))
hl.bind(mod .. " + ALT + up",    hl.dsp.focus({ direction = "up" }))
hl.bind(mod .. " + ALT + down",  hl.dsp.focus({ direction = "down" }))

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

-- MỌI app mở NỔI giữa màn hình, cỡ theo phần trăm màn hình (snap.lua:
-- M.default, riêng từng app ở M.float) — chồng lên nhau như Windows/Ubuntu,
-- app mở sau / vừa chọn nằm trên (user 2026-10-09). Super+← / → / ↑ chia
-- đôi / phóng to (snap.lua).
-- `float = false`: hộp thoại của app Hyprland đã cho nổi sẵn với cỡ riêng →
-- không bị kéo giãn theo (container 2026-10-05: thiếu điều kiện này,
-- Properties của Dolphin rộng 1152px).
--
-- Hộp thoại KDE chạy thành chương trình RIÊNG (không phải hộp thoại con nên
-- Hyprland không tự cho nổi) thì nổi với cỡ APP TỰ CHỌN, không 80%:
--   org.kde.keditfiletype: nút "Change…" của Properties (KMimeTypeEditor) —
--   user 2026-10-09: xếp kín thì nằm dưới Dolphin; nổi cỡ riêng 620×755 gọn.
--   org.quickshell: cửa sổ thường của Quickshell — app Settings
--   (quickshell/settings.qml) tự chọn cỡ 900×640 như System Settings macOS.
local helpers = { "org.kde.keditfiletype", "org.quickshell" }

local function class_regex(classes)
    local parts = {}
    for _, c in ipairs(classes) do parts[#parts + 1] = (c:gsub("%.", "\\.")) end
    return "^(" .. table.concat(parts, "|") .. ")$"
end

local function float_rule(name, class, fraction)
    hl.window_rule({
        name   = name,
        match  = { class = class, float = false },
        float  = true,
        size   = fraction and { "monitor_w * " .. fraction, "monitor_h * " .. fraction } or nil,
        center = true,
    })
end

-- Luật chung KHÔNG khớp app có luật riêng (`negative:` của Hyprland) — hai
-- luật cùng khớp thì không phải lo luật nào thắng.
local special = {}
for _, class in ipairs(helpers) do special[#special + 1] = class end
for class in pairs(snap.float) do special[#special + 1] = class end
table.sort(special)
float_rule("float-all", "negative:" .. class_regex(special), snap.default)
for class, fraction in pairs(snap.float) do
    float_rule("float-" .. class, class_regex({ class }), fraction)
end
float_rule("float-kde-helpers", class_regex(helpers), nil)

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
