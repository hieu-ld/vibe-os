// SINH TỰ ĐỘNG từ theme/palette.toml bởi tools/gen-theme.py — đừng sửa tay.
#pragma once

namespace samhie {
constexpr const char *ACTIVE_TEXT = "#00c0e8";  // sidebar: mục đang chọn — chữ + icon cyan, không nền
constexpr const char *HOVER_TEXT = "#fafafa";  // sidebar: mục đang trỏ chuột — chữ sáng lên, không nền
constexpr const char *PANE = "#171717";  // vùng nội dung (file Dolphin, chữ Konsole) trên nền cửa sổ secondary
constexpr const char *FRAME_LINE = "#393939";  // viền trên + trái vùng file Dolphin
constexpr const char *POPUP = "#262626";  // thẻ thanh tìm Dolphin (dạng popup, đục)
constexpr const char *POPUP_BORDER = "#393939";  // viền thẻ thanh tìm
constexpr const char *FOCUS_RING = "#b300c0e8";  // khung focus của ô file (accent 70%)
constexpr const char *ROW_HOVER = "#363d3f";  // ô dòng file Dolphin chế độ Details (vuông) — như itemview của Kvantum
constexpr const char *ROW_SELECTED_BORDER = "#00c0e8";  // viền ô dòng file đang chọn
constexpr double WINDOW_OPACITY = 0.9;  // [window] opacity
constexpr int ITEM_RADIUS = 10;  // góc ô file = RADIUS["itemview"] (tools/kvantum.py)
constexpr int TAB_RADIUS = 8;  // góc trên tab = RADIUS["tab"] (tools/kvantum.py)
constexpr int TAB_INSET = 4;  // khe trên tab = TAB_INSET (tools/kvantum.py)
}
