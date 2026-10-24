-- Chia đôi màn hình kiểu Windows (Win + mũi tên) — user chốt 2026-10-05.
--
--   Super+← / →  cửa sổ vào nửa trái / phải.
--                - Đã có cửa sổ xếp kín khác: cửa sổ này xếp kín cạnh nó, đúng
--                  phía đã bấm (đang xếp sai phía thì đổi chỗ).
--                - Chỉ có mình nó: NỔI, chiếm đúng nửa màn, nửa kia trống. Mở
--                  app xếp kín sau đó thì cửa sổ này vào xếp kín ở phía của
--                  nó, app mới lấp nửa còn lại (như Snap của Windows).
--   Super+↑      phóng kín vùng làm việc, vẫn là cửa sổ nổi (xem maximize).
--   Super+↓      đang phóng to → về chỗ cũ (đang chia đôi thì về chia đôi);
--                đang xếp kín / nửa màn → về cửa sổ nổi cỡ mặc định, giữa màn.
--
-- Nạp từ samhie.lua. Cỡ nổi mặc định của từng app (float) cũng dùng cho luật
-- cửa sổ trong samhie.lua — một chỗ khai.

local M = {}

-- Cỡ cửa sổ nổi theo phần màn hình (cả rộng lẫn cao). MỌI app mở nổi (user
-- 2026-10-09: app mở sau / vừa chọn phải đè lên app trước — Hyprland luôn vẽ
-- cửa sổ nổi TRÊN cửa sổ xếp kín, trộn hai loại thì VS Code/Edge xếp kín
-- không bao giờ lên trên Dolphin/Konsole nổi). Mặc định 80% (= Dolphin, user
-- 2026-10-08); Konsole 60% (user 2026-10-09; trước 50%).
M.float = {
    ["org.kde.konsole"] = 0.6,
    -- Hộp chọn file KDE của app Settings (nút Choose… hình nền): tự chọn cỡ
    -- 1728×972 trên màn 1920×1080 — che cả Settings (container 2026-10-21).
    ["org.kde.kdialog"] = 0.6,
}
M.default = 0.8

-- Cửa sổ đang "nổi nửa màn" (bấm khi chỉ có mình nó): address → { side, x, y, w, h }.
-- Người dùng kéo/đổi cỡ nó rồi thì toạ độ khác → coi như hết chia đôi.
local snapped = {}
-- Cửa sổ đang phóng to: address → { x, y, w, h } chỗ trước khi phóng to.
local maximized = {}

local function side_value(v, key)
    if type(v) == "table" then return v[key] or 0 end
    return v or 0
end

-- Vùng xếp cửa sổ của màn hình: trừ thanh trên (reserved) và khe ngoài —
-- đúng chỗ dwindle đặt cửa sổ kín màn (đo container: 9,39 1902×1032 trên
-- 1920×1080, thanh 30, khe 8, viền 1).
local function work_area(mon)
    local r = mon.reserved or {}
    local gaps = hl.get_config("general.gaps_out")
    local scale = mon.scale or 1
    local x = mon.x + side_value(r, "left") + side_value(gaps, "left")
    local y = mon.y + side_value(r, "top") + side_value(gaps, "top")
    local w = mon.width / scale - side_value(r, "left") - side_value(r, "right")
        - side_value(gaps, "left") - side_value(gaps, "right")
    local h = mon.height / scale - side_value(r, "top") - side_value(r, "bottom")
        - side_value(gaps, "top") - side_value(gaps, "bottom")
    return x, y, w, h
end

-- Nửa trái/phải như hai cửa sổ dwindle chia đôi: giữa hai nửa là 2 × khe
-- trong; toạ độ cửa sổ KHÔNG tính viền.
local function half_rect(mon, side)
    local x, y, w, h = work_area(mon)
    local gap = side_value(hl.get_config("general.gaps_in"), "left") * 2
    local border = hl.get_config("general.border_size") or 0
    local half = math.floor((w - gap) / 2)
    local left = side == "left" and x or x + w - half
    return left + border, y + border, half - 2 * border, h - 2 * border
end

local function tiled_others(win)
    local list = {}
    if not win.workspace then return list end
    for _, o in ipairs(hl.get_windows({ workspace = tostring(win.workspace.id) })) do
        if o.address ~= win.address and not o.floating and o.mapped and not o.hidden then
            list[#list + 1] = o
        end
    end
    return list
end

-- Bỏ phóng to/toàn màn: `unset` phải kèm đúng chế độ đang bật, thiếu thì
-- Hyprland bỏ qua (container 2026-10-05: unset trơn không gỡ được maximized).
local function unfullscreen(win)
    hl.dispatch(hl.dsp.window.fullscreen({
        window = win, action = "unset",
        mode = win.fullscreen == 1 and "maximized" or "fullscreen",
    }))
end

local function pos(win)
    return win.at.x, win.at.y, win.size.x, win.size.y
end

local function same_rect(win, s)
    local x, y, w, h = pos(win)
    return math.abs(x - s.x) <= 2 and math.abs(y - s.y) <= 2
        and math.abs(w - s.w) <= 2 and math.abs(h - s.h) <= 2
end

-- Đưa cửa sổ nổi vào xếp kín ở đúng phía: dwindle chèn cửa sổ mới vào bên
-- trái/trên (force_split 1) hoặc phải/dưới (2) của cửa sổ đang có.
local function tile_on_side(win, side)
    local before = hl.get_config("dwindle.force_split")
    hl.config({ dwindle = { force_split = side == "left" and 1 or 2 } })
    hl.dispatch(hl.dsp.window.float({ action = "unset", window = win }))
    hl.config({ dwindle = { force_split = before } })
end

local function float_at(win, x, y, w, h)
    if not win.floating then
        hl.dispatch(hl.dsp.window.float({ action = "set", window = win }))
    end
    hl.dispatch(hl.dsp.window.resize({ window = win, x = w, y = h }))
    hl.dispatch(hl.dsp.window.move({ window = win, x = x, y = y }))
end

local MAX_TAG = "samhie-maximized"

local function full_rect(mon)
    local r = mon.reserved or {}
    local scale = mon.scale or 1
    local border = hl.get_config("general.border_size") or 0
    local x = mon.x + side_value(r, "left") + border
    local y = mon.y + side_value(r, "top") + border
    local w = mon.width / scale - side_value(r, "left") - side_value(r, "right") - 2 * border
    local h = mon.height / scale - side_value(r, "top") - side_value(r, "bottom") - 2 * border
    return x, y, w, h
end

-- Gắn/gỡ dấu phóng to: thẻ cho thanh trên + góc vuông lúc phóng to (bo 12
-- thì lộ hình nền ở hai góc dưới — container 2026-10-09; fullscreen cũ và
-- Windows 11 đều vuông khi phóng to).
local function set_max_tag(win, on)
    hl.dispatch(hl.dsp.window.tag({ tag = (on and "+" or "-") .. MAX_TAG, window = win }))
    hl.dispatch(hl.dsp.window.set_prop({ prop = "rounding", value = on and "0" or "unset", window = win }))
    hl.dispatch(hl.dsp.event("samhie-maximize"))
end

function M.snap(side)
    local win = hl.get_active_window()
    if not win or not win.monitor then return end
    if win.fullscreen ~= 0 then
        unfullscreen(win)
    end
    snapped[win.address] = nil
    if maximized[win.address] then
        maximized[win.address] = nil
        set_max_tag(win, false)
    end

    local others = tiled_others(win)
    if #others == 0 then
        local x, y, w, h = half_rect(win.monitor, side)
        float_at(win, x, y, w, h)
        snapped[win.address] = { side = side, x = x, y = y, w = w, h = h }
        return
    end

    if win.floating then
        tile_on_side(win, side)
        return
    end
    -- Đã xếp kín: có cửa sổ xếp kín nằm phía bên kia thì đổi chỗ với nó.
    local x = win.at.x
    for _, o in ipairs(others) do
        if (side == "left" and o.at.x < x) or (side == "right" and o.at.x > x) then
            hl.dispatch(hl.dsp.window.swap({ direction = side }))
            return
        end
    end
end

-- Phóng to = cửa sổ NỔI kéo kín vùng làm việc (dưới thanh trên, như Windows),
-- không dùng fullscreen "maximized" của Hyprland: cửa sổ nổi nào đã từng lên
-- trên cửa sổ fullscreen thì Hyprland vẫn vẽ nó trên mãi, chọn lại app phóng
-- to cũng không đè lại được (container 2026-10-09). Cũng không bỏ nổi cho xếp
-- kín: cửa sổ xếp kín luôn nằm DƯỚI cửa sổ nổi.
-- Thanh trên biết cửa sổ đang phóng to qua thẻ `samhie-maximized`
-- (WindowControls.qml), báo đọc lại bằng sự kiện "custom>>samhie-maximize".
-- Còn phóng to không: người dùng kéo/đổi cỡ cửa sổ rồi thì thôi (Hyprland
-- không báo sự kiện kéo — xét lúc cần).
local function is_maximized(win)
    if not maximized[win.address] then return false end
    local x, y, w, h = full_rect(win.monitor)
    if same_rect(win, { x = x, y = y, w = w, h = h }) then return true end
    maximized[win.address] = nil
    set_max_tag(win, false)
    return false
end

local function maximize(win)
    if not win or not win.monitor then return end
    if win.fullscreen ~= 0 then
        unfullscreen(win)
    end
    if is_maximized(win) then return end
    local x, y, w, h = pos(win)
    maximized[win.address] = { x = x, y = y, w = w, h = h }   -- chỗ cũ để thu về
    snapped[win.address] = nil
    float_at(win, full_rect(win.monitor))
    hl.dispatch(hl.dsp.window.alter_zorder({ mode = "top", window = win }))
    set_max_tag(win, true)
end

-- Thu về đúng chỗ trước khi phóng to (đang chia đôi thì về chia đôi).
local function unmaximize(win)
    local m = maximized[win.address]
    maximized[win.address] = nil
    float_at(win, m.x, m.y, m.w, m.h)
    set_max_tag(win, false)
end

function M.maximize()
    maximize(hl.get_active_window())
end

-- Nút ⤢ / ⧉ ở thanh trên (Windows.qml gọi qua hl.dispatch). `address` như
-- Quickshell ghi: hex không có "0x".
function M.toggle_maximize(address)
    for _, win in ipairs(hl.get_windows()) do
        if win.address == "0x" .. address then
            if is_maximized(win) then unmaximize(win) else maximize(win) end
            return
        end
    end
end

function M.restore()
    local win = hl.get_active_window()
    if not win or not win.monitor then return end
    if win.fullscreen ~= 0 then
        unfullscreen(win)
        return
    end
    if is_maximized(win) then
        unmaximize(win)
        return
    end
    if win.floating and not snapped[win.address] then return end
    snapped[win.address] = nil
    -- Cùng cỡ + chỗ như lúc app mở (luật cửa sổ trong samhie.lua: phần màn
    -- hình, `center`) — container: Konsole 960×540 ở 480,285 cả hai đường.
    local f = M.float[win.class] or M.default
    local scale = win.monitor.scale or 1
    if not win.floating then
        hl.dispatch(hl.dsp.window.float({ action = "set", window = win }))
    end
    hl.dispatch(hl.dsp.window.resize({ window = win,
        x = math.floor(win.monitor.width / scale * f), y = math.floor(win.monitor.height / scale * f) }))
    hl.dispatch(hl.dsp.window.center({ window = win }))
end

-- App xếp kín mở ra cạnh cửa sổ đang nổi nửa màn → cửa sổ đó vào xếp kín ở
-- phía của nó, app mới lấp nửa kia.
hl.on("window.open", function(win)
    if win.floating or not win.workspace then return end
    for _, o in ipairs(hl.get_windows({ workspace = tostring(win.workspace.id), floating = true })) do
        local s = snapped[o.address]
        if s then
            snapped[o.address] = nil
            if same_rect(o, s) then
                tile_on_side(o, s.side)
                hl.dispatch(hl.dsp.focus({ window = win }))
            end
            return
        end
    end
end)

hl.on("window.close", function(win)
    snapped[win.address] = nil
    maximized[win.address] = nil
end)

-- Chọn lại app đang TOÀN MÀN HÌNH (phim, YouTube) thì nó phải đè lên cửa sổ
-- vừa chọn trước đó. Hyprland không làm: cửa sổ nổi đã được focus lúc có
-- cửa sổ toàn màn hình thì mang cờ allowed_over_fullscreen và cứ vẽ trên mãi
-- (user 2026-10-24: phim → Win chọn Konsole → chọn lại phim, Konsole vẫn
-- dính). Cờ chỉ được xoá khi một cửa sổ VÀO toàn màn hình → đặt lại riêng
-- trạng thái phía Hyprland (internal 1 rồi 2), phía app (client) giữ nguyên:
-- set lại cùng trạng thái thì Hyprland bỏ qua, còn thoát hẳn rồi vào lại thì
-- trình duyệt tự thoát toàn màn hình. Đo máy thật (Kate toàn màn hình +
-- Konsole): cờ của Konsole True → False, Kate đè lên; chọn Konsole lại lên trên.
hl.on("window.active", function(win)
    if not win or win.fullscreen ~= 2 or not win.workspace then return end
    local client = win.fullscreen_client
    for _, o in ipairs(hl.get_windows({ workspace = tostring(win.workspace.id) })) do
        if o.address ~= win.address and o.allowed_over_fullscreen then
            hl.dispatch(hl.dsp.window.fullscreen_state({ window = win, internal = 1, client = client }))
            hl.dispatch(hl.dsp.window.fullscreen_state({ window = win, internal = 2, client = client }))
            return
        end
    end
end)

return M
