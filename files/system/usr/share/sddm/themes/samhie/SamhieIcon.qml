// Icon của bộ Samhie (/usr/share/icons/Samhie), tô lại bằng màu chữ.
// Đọc thẳng file SVG: greeter không nạp bộ icon hệ thống như app KDE.
import QtQuick
import QtQuick.Controls.impl
import "colors.js" as Colors

IconImage {
    // Không đặt tên `name`: IconImage đã có `name` (tên trong bộ icon hệ thống).
    property string iconName
    property int size: 20

    // Mọi icon màn hình đăng nhập dùng đều nằm trong actions/.
    source: iconName !== "" ? "file:///usr/share/icons/Samhie/actions/scalable/" + iconName + ".svg" : ""
    sourceSize: Qt.size(size, size)
    color: Colors.dark.text
}
