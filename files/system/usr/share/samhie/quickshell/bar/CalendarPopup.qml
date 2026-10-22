// Bấm đồng hồ trên thanh thì mở cửa sổ này, ngay dưới đồng hồ. Chia đôi theo
// chiều ngang như bản 1.0: trái là đồng hồ kim + giờ to + ngày + ngày âm,
// phải là lịch tháng, mỗi ô hai dòng — ngày dương trên, ngày âm dưới.
//
// Bố cục lịch chép theo node "Calendar" (146:1065) trong file Figma
// "Reminder" của user (bản 1.0): tuần bắt đầu Thứ 2, header có tháng dương
// to kèm dòng tháng âm nhỏ, mùng 1 âm ghi "1/6" và tô đỏ (`danger`), ô hôm nay
// là hình vuông bo góc tô `accent`.
//
// Âm lịch Việt Nam (UTC+7) tính bằng lunar.js — kiểm bằng
// `node tools/test-lunar.js` (bộ kiểm đó rút luôn hàm buildCells ở dưới ra
// chạy, nên đổi tên/chữ ký hàm thì sửa cả bộ kiểm).
//
// Esc hoặc bấm ra ngoài: đóng. Mũi tên trái/phải: lật tháng (phím do
// PopupLayer.qml nhận).
import QtQuick
import QtQuick.Layouts
import Quickshell
import ".."
import "../colors.js" as Colors
import "lunar.js" as Lunar

PopupMotion {
    id: root

    required property var bar
    readonly property bool open: bar.popup === "calendar"

    // Số đo bản 1.0 quy ra pixel (gridUnit 18): ô ngày 3,2 → 56, mặt đồng hồ
    // 9 → 162. Cột đồng hồ 13 → 234 thì dòng âm lịch "… năm Bính Ngọ" phải
    // xuống dòng (đo 2026-10-01) → 264.
    readonly property int cellSize: 56
    readonly property int clockWidth: 264
    readonly property int pad: 16

    readonly property color text: Theme.c.text
    readonly property color muted: Theme.c.textMuted

    // Ngày giờ tới GIÂY cho kim giây — chỉ chạy khi cửa sổ đang mở.
    SystemClock {
        id: tick
        enabled: root.open
        precision: SystemClock.Seconds
    }
    readonly property date now: tick.date

    // Date.getDay(): 0 = Chủ nhật. Tiêu đề cột thì tuần bắt đầu Thứ 2.
    readonly property var weekdayNames: ["Chủ nhật", "Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7"]
    readonly property var weekdayShort: ["Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7", "CN"]

    // Tháng đang xem — luôn là ngày mùng 1. Mở cửa sổ là về tháng hiện tại.
    property date shownMonth: new Date(now.getFullYear(), now.getMonth(), 1)
    onOpenChanged: if (open)
        shownMonth = new Date(now.getFullYear(), now.getMonth(), 1)

    function pad2(n) {
        return n < 10 ? "0" + n : "" + n
    }

    function stepMonth(delta) {
        shownMonth = new Date(shownMonth.getFullYear(), shownMonth.getMonth() + delta, 1)
    }

    function sameDay(a, b) {
        return a.getFullYear() === b.getFullYear()
            && a.getMonth() === b.getMonth()
            && a.getDate() === b.getDate()
    }

    // 42 ô (6 hàng × 7 cột — đủ cho mọi tháng). Ô đầu lùi về Thứ 2 gần nhất
    // trước mùng 1. Mùng 1 âm ghi kèm tháng ("1/6"), không đánh dấu nhuận ở
    // đây — chữ "nhuận" để trên header cho khỏi chật ô (giống app Reminder).
    function buildCells(monthDate) {
        const y = monthDate.getFullYear()
        const m = monthDate.getMonth()
        // getDay() coi Chủ nhật là 0; xoay để Thứ 2 thành 0.
        const lead = (new Date(y, m, 1).getDay() + 6) % 7
        const out = []
        for (let i = 0; i < 42; i++) {
            // Date tự dồn tháng/năm khi số ngày tràn ra ngoài.
            const d = new Date(y, m, 1 - lead + i)
            const l = Lunar.solar2Lunar(d.getDate(), d.getMonth() + 1, d.getFullYear())
            const newLunarMonth = l[0] === 1
            out.push({
                dayText: "" + d.getDate(),
                lunarText: newLunarMonth ? l[0] + "/" + l[1] : "" + l[0],
                newLunarMonth: newLunarMonth,
                inMonth: d.getMonth() === m,
                isToday: root.sameDay(d, root.now)
            })
        }
        return out
    }

    // Tháng âm ở header lấy theo mùng 1 dương của tháng đang xem.
    readonly property var shownLunar: Lunar.solar2Lunar(1, shownMonth.getMonth() + 1, shownMonth.getFullYear())
    readonly property var todayLunar: Lunar.solar2Lunar(now.getDate(), now.getMonth() + 1, now.getFullYear())

    // Hiện/ẩn có hiệu ứng (PopupMotion).
    showing: open
    transformOrigin: Item.Top
    width: card.width
    height: card.height

    Rectangle {
        id: card
        width: body.implicitWidth + 2 * root.pad
        height: body.implicitHeight + 2 * root.pad
        radius: 12
        color: Theme.c.card
        border.width: 1
        border.color: Theme.c.subSecondary

        // Hút cú bấm vào chỗ trống trong thẻ — không thì nó lọt xuống lớp
        // phủ (PopupLayer) và đóng popup.
        MouseArea { anchors.fill: parent }

        RowLayout {
            id: body
            x: root.pad
            y: root.pad
            spacing: root.pad

            // ── Phần 1: đồng hồ ──────────────────────────────────────────
            ColumnLayout {
                Layout.preferredWidth: root.clockWidth
                Layout.maximumWidth: root.clockWidth
                Layout.fillHeight: true
                spacing: 4

                Item { Layout.fillHeight: true }

                AnalogClock {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 162
                    Layout.preferredHeight: 162
                    time: root.now
                    // Mặt số nhấc khỏi nền bằng màu chữ pha độ đục thấp.
                    faceColor: Qt.rgba(root.text.r, root.text.g, root.text.b, 0.06)
                    tickColor: Qt.rgba(root.text.r, root.text.g, root.text.b, 0.45)
                    handColor: root.text
                    secondColor: Theme.c.accent
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 4
                    text: root.pad2(root.now.getHours()) + ":" + root.pad2(root.now.getMinutes())
                    color: root.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pointSize: Colors.fonts.size * 2.2
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: root.weekdayNames[root.now.getDay()] + ", " + root.pad2(root.now.getDate())
                        + " tháng " + root.pad2(root.now.getMonth() + 1) + " năm " + root.now.getFullYear()
                    color: root.text
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pointSize: Colors.fonts.size
                }

                // Ví dụ "Ngày 18 tháng 5 âm lịch — năm Bính Ngọ".
                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: "Ngày " + root.todayLunar[0] + " tháng " + root.todayLunar[1]
                        + (root.todayLunar[3] ? " nhuận" : "")
                        + " âm lịch — năm " + Lunar.canChiYear(root.todayLunar[2])
                    color: root.muted
                    font.family: Colors.fonts.ui
                    font.weight: Colors.fonts.uiWeight
                    font.pointSize: Colors.fonts.size * 0.8
                }

                Item { Layout.fillHeight: true }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: Theme.c.subSecondary
            }

            // ── Phần 2: lịch ─────────────────────────────────────────────
            ColumnLayout {
                spacing: 4

                // Header: chữ ở CHÍNH GIỮA, hai nút dạt ra hai biên. Item +
                // anchors chứ không RowLayout: trong RowLayout chữ chỉ nằm giữa
                // phần chỗ còn lại sau hai nút, lệch khi hai nút không bằng nhau.
                Item {
                    Layout.fillWidth: true
                    implicitHeight: Math.max(monthCol.implicitHeight, 36)

                    NavButton {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        iconName: "go-previous"
                        iconColor: root.text
                        onClicked: root.stepMonth(-1)
                    }

                    Column {
                        id: monthCol
                        anchors.centerIn: parent

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Tháng " + (root.shownMonth.getMonth() + 1) + ", " + root.shownMonth.getFullYear()
                            color: root.text
                            font.family: Colors.fonts.ui
                            font.weight: Font.Normal
                            font.pointSize: Colors.fonts.size * 1.15
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Tháng " + root.shownLunar[1] + (root.shownLunar[3] ? " nhuận" : "") + ", Âm Lịch"
                            color: root.muted
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pointSize: Colors.fonts.size * 0.8
                        }
                    }

                    NavButton {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        iconName: "go-next"
                        iconColor: root.text
                        onClicked: root.stepMonth(1)
                    }
                }

                // Hàng tiêu đề thứ.
                Row {
                    Repeater {
                        model: root.weekdayShort
                        delegate: Text {
                            required property string modelData
                            width: root.cellSize
                            horizontalAlignment: Text.AlignHCenter
                            text: modelData
                            color: root.muted
                            font.family: Colors.fonts.ui
                            font.weight: Colors.fonts.uiWeight
                            font.pointSize: Colors.fonts.size * 0.8
                        }
                    }
                }

                // Lưới 42 ô.
                Grid {
                    columns: 7

                    Repeater {
                        model: root.buildCells(root.shownMonth)

                        delegate: Item {
                            id: cell
                            required property var modelData
                            width: root.cellSize
                            height: root.cellSize

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 2
                                radius: 8
                                color: cell.modelData.isToday ? Theme.c.accent
                                    : dayHover.hovered ? Theme.c.secondary : "transparent"
                                Behavior on color {
                                    ColorAnimation { duration: 100 }
                                }
                            }

                            // Hai dòng đặt bằng y + chiều cao CỐ ĐỊNH: dòng của
                            // Dongle cao gần gấp đôi phần mực, xếp Column thì
                            // dòng âm tràn sang ô hàng dưới (đo 2026-10-01).
                            // Mực nằm giữa dòng (tools/fix-dongle-metrics.py)
                            // nên AlignVCenter đặt được tâm mực: dương ở 21,
                            // âm ở 39 trong ô 56.
                            Item {
                                anchors.fill: parent

                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    y: 8
                                    height: 26
                                    verticalAlignment: Text.AlignVCenter
                                    text: cell.modelData.dayText
                                    color: cell.modelData.isToday ? Theme.c.onAccent : root.text
                                    // Ngày của tháng khác mờ đi, vẫn đọc được.
                                    opacity: cell.modelData.inMonth ? 1 : 0.35
                                    font.family: Colors.fonts.ui
                                    font.weight: Colors.fonts.uiWeight
                                    font.pointSize: Colors.fonts.size
                                }
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    y: 30
                                    height: 18
                                    verticalAlignment: Text.AlignVCenter
                                    text: cell.modelData.lunarText
                                    color: cell.modelData.isToday ? Theme.c.onAccent
                                        : cell.modelData.newLunarMonth ? Theme.c.danger : root.muted
                                    opacity: cell.modelData.inMonth ? 1 : 0.45
                                    font.family: Colors.fonts.ui
                                    font.weight: Colors.fonts.uiWeight
                                    font.pointSize: Colors.fonts.size * 0.8
                                }
                            }

                            HoverHandler { id: dayHover }
                        }
                    }
                }
            }
        }
    }

    // Nút lật tháng: icon nét user vẽ, nền bo góc khi rê chuột.
    component NavButton: Rectangle {
        id: nav
        property alias iconName: navIcon.iconName
        property alias iconColor: navIcon.color
        signal clicked()

        width: 36
        height: 36
        radius: 8
        color: navHover.hovered ? Theme.c.secondary : "transparent"

        SamhieIcon {
            id: navIcon
            anchors.centerIn: parent
        }
        HoverHandler { id: navHover }
        TapHandler { onTapped: nav.clicked() }
    }
}
