// Dolphin: tô vùng file + viền (mục 3, 4, 27), thẻ thanh tìm (mục 7).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// QTabWidget và con của nó tự vẽ — con có nền trong suốt nên thấy lớp này.
class PanePainter : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Paint) {
            if (auto *tw = qobject_cast<QTabWidget *>(obj))
                paint(tw);
            else
                paintTerminalDock(static_cast<QWidget *>(obj));
        }
        return false;
    }

private:
    // Mục 27: dock Terminal là thẻ thứ hai dưới vùng file, cách TERMINAL_GAP
    // (phần trên dock để trống — nền cửa sổ): nền card bo góc trên-trái, viền
    // trên + trái. Konsole tô nền cùng màu (Samhie.colorscheme) trong phần
    // lùi vào, góc bo nằm ngoài nó. Mục 81: thẻ lấy nền terminal (tối cả hai
    // chế độ, như Konsole); viền chỉ khi thẻ cùng màu vùng file (bản tối) —
    // thẻ tối trên nền cửa sổ sáng đã tự tách.
    static void paintTerminalDock(QWidget *dock)
    {
        const QRect all = dock->rect().adjusted(0, TERMINAL_GAP, 0, 0);
        QColor fill(samhie::TERMINAL);
        fill.setAlphaF(samhie::WINDOW_OPACITY);
        QPainter p(dock);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillPath(leftOutline(QRectF(all), PANE_RADIUS, 0, true), fill);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        if (QColor(samhie::PANE) != QColor(samhie::TERMINAL))
            return;
        p.setPen(QPen(QColor(samhie::FRAME_LINE), 1));
        p.setBrush(Qt::NoBrush);
        p.drawPath(leftOutline(QRectF(all).adjusted(0.5, 0.5, 0, 0), PANE_RADIUS, 0, false));
    }

    static void paint(QTabWidget *tw)
    {
        const bool dolphin = tw->inherits("DolphinTabWidget");
        const QRect all = tw->rect();
        // Vùng tô: dưới dải tab (nếu dải đang hiện) tới đáy.
        QRect pane = all;
        const QTabBar *bar = tw->tabBar();
        if (bar && bar->isVisible() && tw->tabPosition() == QTabWidget::North)
            pane.setTop(bar->geometry().bottom() + 1);

        const int bottomRad = dolphin ? paneBottomRadius(tw) : 0;

        QPainter p(tw);
        p.setRenderHint(QPainter::Antialiasing);
        // Dolphin: góc bo ở ĐỈNH khung. Có dải tab thì dải nằm trong khung,
        // giữ nền cửa sổ — vùng file khi đó vuông góc ở trên.
        const QPainterPath fill = dolphin
            ? leftOutline(QRectF(pane), pane.top() == all.top() ? PANE_RADIUS : 0, bottomRad, true)
            : rectPath(QRectF(pane));
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillPath(fill, paneColor(tw));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        if (dolphin) {
            // Viền trên + trái (+ đáy), nét 1px rơi đúng giữa hàng điểm ảnh.
            // Không khép phải: đó là mép cửa sổ.
            p.setPen(QPen(QColor(samhie::FRAME_LINE), 1));
            p.setBrush(Qt::NoBrush);
            p.drawPath(paneBorder(tw));
        }
    }

    static QPainterPath rectPath(const QRectF &r)
    {
        QPainterPath path;
        path.addRect(r);
        return path;
    }

};

// Thẻ popup sau nội dung thanh tìm (vẽ trước các con của khung nội dung).
// Mục 60: vùng thẻ trong toạ độ của widget. View space Kate: breadcrumb
// (KateUrlBar) + editor (QStackedWidget) — hàng tab nằm ngoài thẻ.
QRect cardRect(const QWidget *w)
{
    if (!w->inherits("KateViewSpace"))
        return w->rect();
    QRect r;
    for (QObject *o : w->children()) {
        auto *c = qobject_cast<QWidget *>(o);
        if (c && !c->isHidden() && (c->inherits("KateUrlBar") || qobject_cast<QStackedWidget *>(c)))
            r |= c->geometry();
    }
    return r;
}
