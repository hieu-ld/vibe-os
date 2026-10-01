// Đồng hồ giữa thanh: "13:36 - Thứ 2, 28/09/2026" — giờ TRƯỚC, ngày dạng số
// (user chốt 2026-09-28). Ngày giờ giữ tiếng Việt dù giao diện tiếng Anh
// (CLAUDE.md mục 8). Bấm vào mở CalendarPopup.
//
// Tên thứ tự tra bảng: locale tiếng Việt của Qt chỉ có "Thứ Hai" (dddd) và
// "Th 2" (ddd), không có "Thứ 2" (bản 1.0 đã gặp). Ngày và tháng đệm số 0 để
// chuỗi không co giãn khi sang ngày mới — đồng hồ nằm giữa thanh, co giãn là
// thấy nhảy.
import QtQuick
import Quickshell
import "../colors.js" as Colors

Item {
    id: root

    required property var bar

    // Date.getDay(): 0 = Chủ nhật, 1 = Thứ 2, … 6 = Thứ 7.
    readonly property var weekdays: ["Chủ nhật", "Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7"]

    function pad2(n) {
        return n < 10 ? "0" + n : "" + n
    }

    // SystemClock nhảy số đúng lúc giây về 0 và chỉ đánh thức mỗi phút một lần.
    SystemClock {
        id: clock
        precision: SystemClock.Minutes
    }

    implicitWidth: label.implicitWidth + 16
    implicitHeight: bar.height

    Text {
        id: label
        anchors.centerIn: parent
        readonly property date now: clock.date
        text: root.pad2(now.getHours()) + ":" + root.pad2(now.getMinutes())
            + " - " + root.weekdays[now.getDay()]
            + ", " + root.pad2(now.getDate()) + "/" + root.pad2(now.getMonth() + 1) + "/" + now.getFullYear()
        color: hover.hovered || root.bar.popup === "calendar" ? root.bar.hoverColor : root.bar.textColor
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pointSize: Colors.fonts.size
    }

    HoverHandler { id: hover }
    TapHandler { onTapped: root.bar.toggle("calendar") }
}
