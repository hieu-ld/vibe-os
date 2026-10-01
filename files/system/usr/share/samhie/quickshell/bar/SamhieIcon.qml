// Icon nét của bộ Samhie (/usr/share/icons/Samhie), tô lại bằng `color` —
// giống SamhieIcon của theme SDDM. Đọc thẳng file SVG: icon user vẽ ghi cứng
// màu #fcfcfc, chỉ app KDE mới tự tô lại theo bảng màu.
import QtQuick
import QtQuick.Controls.impl

IconImage {
    // Không đặt tên `name`: IconImage đã có `name` (tên trong bộ icon hệ thống).
    property string iconName
    property int size: 20

    // Mọi icon thanh trên dùng đều nằm trong actions/.
    source: iconName !== "" ? "file:///usr/share/icons/Samhie/actions/scalable/" + iconName + ".svg" : ""
    sourceSize: Qt.size(size, size)
}
