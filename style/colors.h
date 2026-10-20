// SINH TỰ ĐỘNG từ theme/palette.toml bởi tools/gen-theme.py — đừng sửa tay.
#pragma once

namespace samhie {
inline const char *ACTIVE_TEXT = "#00c0e8";  // sidebar: mục đang chọn — chữ + icon cyan, không nền
inline const char *HOVER_TEXT = "#fafafa";  // sidebar: mục đang trỏ chuột — chữ nổi lên, không nền
inline const char *HOVER_ACCENT = "#00c0e8";  // thanh đường dẫn Dolphin: tên thư mục đang trỏ — chữ cyan, không nền
inline const char *PANE = "#171717";  // vùng nội dung (file Dolphin, chữ Konsole) trên nền cửa sổ secondary
inline const char *FRAME_LINE = "#393939";  // viền trên + trái vùng file Dolphin
inline const char *WINDOW = "#262626";  // nền cửa sổ (window.color Kvantum) — vẽ lại chỗ app tô đè
inline const char *POPUP = "#262626";  // thẻ thanh tìm Dolphin (dạng popup, đục)
inline const char *POPUP_BORDER = "#393939";  // viền thẻ thanh tìm
inline const char *FOCUS_RING = "#b300c0e8";  // khung focus của ô file (accent 70%)
inline const char *ROW_HOVER = "#262626";  // ô dòng file Dolphin chế độ Details (vuông) — như itemview của Kvantum
inline const char *ROW_SELECTED = "#393939";  // ô dòng file đang chọn, không viền (= itemview Kvantum)
inline const char *KEY_FOCUS_FILL = "#262626";  // radio/checkbox đi bằng Tab, trên nền card (popup, menu)
inline const char *KEY_FOCUS_FILL_RAISED = "#393939";  // radio/checkbox đi bằng Tab, trên nền cửa sổ secondary
inline const char *POPUP_ITEM = "#393939";  // ô đang chọn/trỏ trong popup nền secondary (Quick Open Kate)
inline const char *KEYCAP = "#1fbdbec1";  // mặt phím tắt trong Find Action (chữ 12%, nổi trên thẻ và ô chọn)
inline const char *TREE_ROW_SELECTED = "#393939";  // cây Projects Kate: dòng đang chọn, không viền
inline const char *TREE_LINE_SELECTED = "#5a5a5b";  // cây Projects Kate: đường nhánh trên dòng đang chọn
inline const char *PAGE_ITEM_SELECTED = "#393939";  // danh sách trang hộp cài đặt: trang đang chọn, không viền
inline const char *SIDEBAR_TAB_OPEN = "#444445";  // cột icon Kate: nút panel đang mở (chữ 20% trên secondary)

// Đổi cả bộ màu sang bản sáng (true) / tối (false).
inline void useLight(bool light)
{
    ACTIVE_TEXT = light ? "#0d728f" : "#00c0e8";
    HOVER_TEXT = light ? "#0a0a0a" : "#fafafa";
    HOVER_ACCENT = light ? "#0d728f" : "#00c0e8";
    PANE = light ? "#fafafa" : "#171717";
    FRAME_LINE = light ? "#d0d0d0" : "#393939";
    WINDOW = light ? "#ededed" : "#262626";
    POPUP = light ? "#ededed" : "#262626";
    POPUP_BORDER = light ? "#d0d0d0" : "#393939";
    FOCUS_RING = light ? "#b30d728f" : "#b300c0e8";
    ROW_HOVER = light ? "#ededed" : "#262626";
    ROW_SELECTED = light ? "#d0d0d0" : "#393939";
    KEY_FOCUS_FILL = light ? "#ededed" : "#262626";
    KEY_FOCUS_FILL_RAISED = light ? "#d0d0d0" : "#393939";
    POPUP_ITEM = light ? "#d0d0d0" : "#393939";
    KEYCAP = light ? "#1f1c1c1c" : "#1fbdbec1";
    TREE_ROW_SELECTED = light ? "#d0d0d0" : "#393939";
    TREE_LINE_SELECTED = light ? "#a3a3a3" : "#5a5a5b";
    PAGE_ITEM_SELECTED = light ? "#d0d0d0" : "#393939";
    SIDEBAR_TAB_OPEN = light ? "#c3c3c3" : "#444445";
}

inline const char *const TERMINAL = "#171717";  // [terminal] background — vùng chữ + tab đang mở Konsole
inline const char *const TERMINAL_TEXT = "#bdbec1";  // dark.text — chữ tab đang mở Konsole, × khi trỏ
inline const char *const TERMINAL_TEXT_MUTED = "#8f8f8f";  // dark.text-muted — × tab đang mở Konsole

constexpr double WINDOW_OPACITY = 0.9;  // [window] opacity
constexpr int ITEM_RADIUS = 10;  // góc ô file = RADIUS["itemview"] (tools/kvantum.py)
constexpr int MENUITEM_RADIUS = 6;  // góc ô hover dòng menu = RADIUS["menuitem"] (tools/kvantum.py)
constexpr int TAB_RADIUS = 8;  // góc trên tab = RADIUS["tab"] (tools/kvantum.py)
constexpr int TAB_INSET = 4;  // khe trên tab = TAB_INSET (tools/kvantum.py)
constexpr int FONT_SIZE = 20;  // [fonts] size — cỡ chữ giao diện (Dongle)
constexpr int FONT_MONO_SIZE = 11;  // [fonts] mono-size — cỡ chữ đều nét cùng độ lớn mắt thấy
}
