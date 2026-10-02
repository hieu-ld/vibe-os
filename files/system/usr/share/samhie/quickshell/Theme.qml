// Bảng màu đang dùng của giao diện Quickshell — đổi theo chế độ sáng/tối
// (ô "Dark mode" của quick settings → quicksettings/Settings.qml đặt `light`).
// Mọi phần giao diện đọc màu ở đây: `Theme.c.card`, `Theme.c.barText`…
// (thư mục con: `import ".."`). Màu sinh từ theme/palette.toml (colors.js).
//
// KHÔNG theo chế độ: màn hình khoá (lock.qml — luôn tối như SDDM) và lớp
// chụp màn hình (screenshot-*.qml — màu Colors.shared).
pragma Singleton
import QtQuick
import Quickshell
import "colors.js" as Colors

Singleton {
    property bool light: false
    readonly property var c: light ? Colors.light : Colors.dark
}
