/*
 * Mặt đồng hồ kim cho cửa sổ bật lên của org.samhie.clock.
 *
 * Vì sao tự vẽ: ISO không có plasmoid org.kde.plasma.analogclock (đã kiểm
 * `ls /usr/share/plasma/plasmoids/` trong chroot). Mà kể cả có thì cũng không
 * nhúng được — plasmoid là applet, không phải component để đặt vào layout của
 * applet khác.
 *
 * Dùng Canvas chứ không phải QtQuick.Shapes: Shapes cần bộ dựng hình riêng và
 * mỗi cây kim phải là một PathLine có sẵn trong scene graph, còn ở đây mỗi giây
 * vẽ lại toàn bộ mặt số nên vẽ tuần tự bằng ngữ cảnh 2D vừa ngắn vừa dễ đọc.
 *
 * Toạ độ Canvas: góc 0 rad hướng sang phải (3 giờ), nên mọi góc phải trừ đi
 * PI/2 để 0 về hướng 12 giờ. Đó là lý do có hằng số TOP.
 *
 * Màu lấy từ thuộc tính truyền vào, KHÔNG đọc thẳng Kirigami.Theme ở đây: file
 * này chỉ biết vẽ, còn chọn màu là việc của cửa sổ gọi nó. Nhờ vậy đổi bảng màu
 * SamhieDark là mặt đồng hồ đi theo mà không phải sửa file này.
 */

import QtQuick

Canvas {
    id: dial

    // Thời điểm cần vẽ. Cửa sổ bật lên cập nhật mỗi giây khi đang mở.
    property date time: new Date()

    property color faceColor
    property color tickColor
    property color handColor
    property color secondColor

    // Vẽ lại khi giờ đổi, và cả khi bảng màu đổi — nếu chỉ nghe time thì đổi
    // theme lúc cửa sổ đang mở sẽ giữ nguyên màu cũ tới tận giây sau.
    onTimeChanged:        requestPaint()
    onFaceColorChanged:   requestPaint()
    onTickColorChanged:   requestPaint()
    onHandColorChanged:   requestPaint()
    onSecondColorChanged: requestPaint()

    antialiasing: true

    onPaint: {
        const ctx = getContext( "2d" );
        ctx.reset();

        const size = Math.min( width, height );
        const r    = size / 2;
        const cx   = width  / 2;
        const cy   = height / 2;

        const TOP = -Math.PI / 2;

        // ── Mặt số ────────────────────────────────────────────────────────
        ctx.beginPath();
        ctx.arc( cx, cy, r * 0.97, 0, Math.PI * 2 );
        ctx.fillStyle = dial.faceColor;
        ctx.fill();

        ctx.lineWidth   = Math.max( 1, r * 0.02 );
        ctx.strokeStyle = dial.tickColor;
        ctx.stroke();

        // ── 12 vạch giờ ───────────────────────────────────────────────────
        // Vạch ở 12/3/6/9 dài và dày hơn để liếc một cái là định hướng được.
        for ( let i = 0; i < 12; i++ ) {
            const quarter = ( i % 3 === 0 );
            const a       = TOP + i * Math.PI / 6;
            const inner   = r * ( quarter ? 0.76 : 0.84 );
            const outer   = r * 0.90;

            ctx.beginPath();
            ctx.lineWidth   = Math.max( 1, r * ( quarter ? 0.045 : 0.025 ) );
            ctx.lineCap     = "round";
            ctx.strokeStyle = dial.tickColor;
            ctx.moveTo( cx + Math.cos( a ) * inner, cy + Math.sin( a ) * inner );
            ctx.lineTo( cx + Math.cos( a ) * outer, cy + Math.sin( a ) * outer );
            ctx.stroke();
        }

        const h = dial.time.getHours();
        const m = dial.time.getMinutes();
        const s = dial.time.getSeconds();

        // Kim giờ và kim phút chạy liên tục (cộng phần lẻ của đơn vị nhỏ hơn)
        // chứ không nhảy nấc — đồng hồ cơ thật là vậy, và nhìn đỡ giả.
        const hourAngle   = TOP + ( ( h % 12 ) + m / 60 ) * Math.PI / 6;
        const minuteAngle = TOP + ( m + s / 60 ) * Math.PI / 30;
        const secondAngle = TOP + s * Math.PI / 30;

        function hand( angle, length, width, color, tail ) {
            ctx.beginPath();
            ctx.lineWidth   = Math.max( 1, width );
            ctx.lineCap     = "round";
            ctx.strokeStyle = color;
            // Đuôi kim thò ngược ra sau tâm một đoạn ngắn, cho cân mắt.
            ctx.moveTo( cx - Math.cos( angle ) * length * tail,
                        cy - Math.sin( angle ) * length * tail );
            ctx.lineTo( cx + Math.cos( angle ) * length,
                        cy + Math.sin( angle ) * length );
            ctx.stroke();
        }

        hand( hourAngle,   r * 0.50, r * 0.060, dial.handColor,   0.20 );
        hand( minuteAngle, r * 0.74, r * 0.042, dial.handColor,   0.20 );
        hand( secondAngle, r * 0.80, r * 0.020, dial.secondColor, 0.26 );

        // ── Chốt tâm ──────────────────────────────────────────────────────
        ctx.beginPath();
        ctx.arc( cx, cy, Math.max( 1.5, r * 0.045 ), 0, Math.PI * 2 );
        ctx.fillStyle = dial.secondColor;
        ctx.fill();
    }
}
