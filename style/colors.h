// SINH TỰ ĐỘNG từ theme/palette.toml bởi tools/gen-theme.py — đừng sửa tay.
#pragma once

namespace samhie {
constexpr const char *ACTIVE_TEXT = "#00c0e8";  // sidebar: mục đang chọn — chữ + icon cyan, không nền
constexpr const char *HOVER_TEXT = "#fafafa";  // sidebar: mục đang trỏ chuột — chữ sáng lên, không nền
constexpr const char *HOVER_ACCENT = "#00c0e8";  // thanh đường dẫn Dolphin: tên thư mục đang trỏ — chữ cyan, không nền
constexpr const char *PANE = "#171717";  // vùng nội dung (file Dolphin, chữ Konsole) trên nền cửa sổ secondary
constexpr const char *FRAME_LINE = "#393939";  // viền trên + trái vùng file Dolphin
constexpr const char *WINDOW = "#262626";  // nền cửa sổ (window.color Kvantum) — vẽ lại chỗ app tô đè
constexpr const char *POPUP = "#262626";  // thẻ thanh tìm Dolphin (dạng popup, đục)
constexpr const char *POPUP_BORDER = "#393939";  // viền thẻ thanh tìm
constexpr const char *FOCUS_RING = "#b300c0e8";  // khung focus của ô file (accent 70%)
constexpr const char *ROW_HOVER = "#262626";  // ô dòng file Dolphin chế độ Details (vuông) — như itemview của Kvantum
constexpr const char *ROW_SELECTED_BORDER = "#097389";  // viền ô dòng file đang chọn (= itemview Kvantum)
constexpr const char *KEY_FOCUS_FILL = "#262626";  // radio/checkbox đi bằng Tab, trên nền card (popup, menu)
constexpr const char *KEY_FOCUS_FILL_RAISED = "#393939";  // radio/checkbox đi bằng Tab, trên nền cửa sổ secondary
constexpr const char *POPUP_ITEM = "#393939";  // ô đang chọn/trỏ trong popup nền secondary (Quick Open Kate)
constexpr const char *TREE_ROW_SELECTED = "#393939";  // cây Projects Kate: dòng đang chọn, không viền
constexpr const char *TREE_LINE_SELECTED = "#5a5a5b";  // cây Projects Kate: đường nhánh trên dòng đang chọn
constexpr const char *SIDEBAR_TAB_OPEN = "#444445";  // cột icon Kate: nút panel đang mở (chữ 20% trên secondary)
constexpr double WINDOW_OPACITY = 0.9;  // [window] opacity
constexpr int ITEM_RADIUS = 10;  // góc ô file = RADIUS["itemview"] (tools/kvantum.py)
constexpr int MENUITEM_RADIUS = 6;  // góc ô hover dòng menu = RADIUS["menuitem"] (tools/kvantum.py)
constexpr int TAB_RADIUS = 8;  // góc trên tab = RADIUS["tab"] (tools/kvantum.py)
constexpr int TAB_INSET = 4;  // khe trên tab = TAB_INSET (tools/kvantum.py)
}
