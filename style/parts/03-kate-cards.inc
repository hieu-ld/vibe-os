// Kate: thẻ bo góc (mục 60), hạ độ đục editor (mục 62).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// Mục 60: một góc thẻ — tô nền cửa sổ phần nằm ngoài cung tròn, đè hẳn
// (Source, giữ độ trong của cửa sổ) lên thứ app tự vẽ vuông bên dưới.
class CardCorner : public QWidget
{
public:
    CardCorner(QWidget *card, Qt::Corner corner)
        : QWidget(card)
        , m_corner(corner)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        setObjectName(QStringLiteral("samhieCardCorner"));
        resize(CARD_RADIUS, CARD_RADIUS);
    }

    // Không có Q_OBJECT (lớp nội bộ) nên tìm theo tên
    static QList<CardCorner *> of(const QWidget *card)
    {
        QList<CardCorner *> list;
        for (QWidget *c : card->findChildren<QWidget *>(QStringLiteral("samhieCardCorner"), Qt::FindDirectChildrenOnly))
            list.append(static_cast<CardCorner *>(c));
        return list;
    }

    void place(const QRect &card)
    {
        const bool right = m_corner == Qt::TopRightCorner || m_corner == Qt::BottomRightCorner;
        const bool bottom = m_corner == Qt::BottomLeftCorner || m_corner == Qt::BottomRightCorner;
        move(right ? card.right() + 1 - CARD_RADIUS : card.left(), bottom ? card.bottom() + 1 - CARD_RADIUS : card.top());
        setVisible(card.isValid());
        raise();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        const bool right = m_corner == Qt::TopRightCorner || m_corner == Qt::BottomRightCorner;
        const bool bottom = m_corner == Qt::BottomLeftCorner || m_corner == Qt::BottomRightCorner;
        QRectF circle(0, 0, 2 * CARD_RADIUS, 2 * CARD_RADIUS);
        if (right)
            circle.moveRight(CARD_RADIUS);
        if (bottom)
            circle.moveBottom(CARD_RADIUS);
        QPainterPath outside;
        outside.addRect(QRectF(rect()));
        QPainterPath arc;
        arc.addEllipse(circle);
        outside = outside.subtracted(arc);
        QColor bg(samhie::WINDOW);
        bg.setAlphaF(samhie::WINDOW_OPACITY);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillPath(outside, bg);
    }

private:
    Qt::Corner m_corner;
};

// Mục 60: tô nền thẻ trước khi widget/con vẽ, và giữ bốn góc đúng chỗ.
class CardPainter : public QObject
{
public:
    using QObject::QObject;

    void attach(QWidget *w)
    {
        w->installEventFilter(this);
        if (auto *area = qobject_cast<QAbstractScrollArea *>(w)) {
            area->setFrameShape(QFrame::NoFrame);
            area->viewport()->setAutoFillBackground(false);
        }
        // Đổi bố cục bên trong view space (breadcrumb ẩn/hiện, editor đổi
        // cỡ) không làm view space đổi cỡ — nghe cả hai con.
        for (QObject *o : w->children()) {
            auto *c = qobject_cast<QWidget *>(o);
            if (c && (c->inherits("KateUrlBar") || qobject_cast<QStackedWidget *>(c)))
                c->installEventFilter(this);
            // Breadcrumb (BreadCrumbView, một QListView) tô nền bằng màu cửa sổ
            // (kateurlbar.cpp updatePalette) → dải đục giữa thẻ; để trong.
            if (c && c->inherits("KateUrlBar")) {
                for (auto *view : c->findChildren<QAbstractItemView *>())
                    view->viewport()->setAutoFillBackground(false);
            }
        }
        if (CardCorner::of(w).isEmpty()) {
            for (auto corner : {Qt::TopLeftCorner, Qt::TopRightCorner, Qt::BottomLeftCorner, Qt::BottomRightCorner})
                new CardCorner(w, corner);
        }
        schedulePlace(w);
    }

    void detach(QWidget *w)
    {
        w->removeEventFilter(this);
        // Ẩn, KHÔNG xoá: detach chạy trong QApplication::setStyle (đổi chế độ
        // sáng/tối, mục 80), lúc Qt đang duyệt danh sách mọi widget — xoá góc
        // ở đây thì Qt chạm widget đã xoá → Kate crash (container 2026-10-20,
        // gdb: QApplication::setStyle → testAttribute_helper). attach dùng lại.
        for (CardCorner *corner : CardCorner::of(w))
            corner->hide();
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        auto *w = static_cast<QWidget *>(obj);
        if (!w->property("samhieCard").toBool()) {
            // con của view space
            if (w->parentWidget() && w->parentWidget()->property("samhieCard").toBool()) {
                switch (event->type()) {
                case QEvent::Move:
                case QEvent::Resize:
                case QEvent::Show:
                case QEvent::Hide:
                    schedulePlace(w->parentWidget());
                    break;
                default:
                    break;
                }
            }
            return false;
        }
        switch (event->type()) {
        case QEvent::Paint: {
            QPainter p(w);
            p.setRenderHint(QPainter::Antialiasing);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            QPainterPath path;
            path.addRoundedRect(QRectF(cardRect(w)), CARD_RADIUS, CARD_RADIUS);
            p.fillPath(path, paneColor(w));
            // Cây: nền thẻ thay khung Kvantum (QFrame::paintEvent)
            return qobject_cast<QAbstractScrollArea *>(w) != nullptr;
        }
        case QEvent::Resize:
        case QEvent::Show:
        case QEvent::LayoutRequest:
        case QEvent::ChildAdded:
            schedulePlace(w);
            break;
        default:
            break;
        }
        return false;
    }

private:
    // Sau vòng sự kiện: lúc này bố cục đã xếp xong con
    void schedulePlace(QWidget *card)
    {
        QTimer::singleShot(0, card, [card] {
            const QRect r = cardRect(card);
            for (CardCorner *c : CardCorner::of(card))
                c->place(r);
            card->update(r);
        });
    }
};

// Mục 62: để widget của KTextEditor tự vẽ, rồi hạ độ đục trong bộ đệm cửa sổ.
class EditorAlpha : public QObject
{
public:
    using QObject::QObject;

    static bool wants(const QWidget *w)
    {
        return w->inherits("KateViewInternal") || w->inherits("KateIconBorder") || w->inherits("KateScrollBar")
            || w->inherits("KTextEditor::ViewPrivate");
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() != QEvent::Paint || m_painting)
            return false;
        auto *w = static_cast<QWidget *>(obj);
        if (!w->window()->testAttribute(Qt::WA_TranslucentBackground))
            return false;
        // Mục 76: vùng xem trước trong popup (thẻ đục) — đục hẳn. Kate tô nền
        // 90% (WA_OpaquePaintEvent: thẻ không vẽ bên dưới) → lót màu PANE
        // trước khi Kate vẽ.
        if (qobject_cast<const QMenu *>(w->window())) {
            if (!w->inherits("KTextEditor::ViewPrivate")) {
                QPainter p(w);
                p.setClipRegion(static_cast<QPaintEvent *>(event)->region());
                p.fillRect(w->rect(), QColor(samhie::PANE));
            }
            return false;
        }
        if (w->inherits("KTextEditor::ViewPrivate")) {
            fillGutterPad(w, static_cast<QPaintEvent *>(event)->region());  // mục 64
            return false;
        }
        // Gọi thẳng QObject::event (public) — chạy paintEvent của Kate ngay
        // trong lượt vẽ này, rồi mới sửa điểm ảnh nó vừa vẽ.
        m_painting = true;
        static_cast<QObject *>(w)->event(event);
        m_painting = false;
        clamp(w, static_cast<QPaintEvent *>(event)->region());
        if (w->inherits("KateScrollBar"))
            airMinimap(w, static_cast<QPaintEvent *>(event)->region());  // mục 65
        // Mục 64
        if (w->inherits("KateIconBorder")) {
            const int width = w->sizeHint().width() + GUTTER_EXTRA;
            if (w->minimumWidth() != width)
                w->setMinimumWidth(width);
            // ô đệm bên trái: cất màu mép trái các hàng vừa vẽ, báo vẽ lại ô đệm
            if (w->x() > 0) {
                const QRect rows = static_cast<QPaintEvent *>(event)->region().boundingRect().intersected(w->rect());
                storeEdgeColors(w, rows);
                w->parentWidget()->update(QRect(0, w->y() + rows.top(), w->x(), rows.height()));
                // Kate cuộn cột số bằng QWidget::scroll (dời hình, chỉ vẽ phần
                // lộ ra — kateviewinternal.cpp) → màu đã cất lệch hàng: vẽ cả
                // cột lần sau (cột nhỏ, rẻ)
                if (rows.top() > 0 || rows.bottom() < w->height() - 1)
                    w->update();
            }
        }
        return true;
    }

private:
    static constexpr const char *EDGE_COLORS = "_samhie_edge_colors";

    // Mục 65
    static void airMinimap(QWidget *w, const QRegion &region)
    {
        if (w->width() < MINIMAP_MIN_WIDTH)
            return;
        QBackingStore *store = w->backingStore();
        auto *image = store ? dynamic_cast<QImage *>(store->paintDevice()) : nullptr;
        if (!image || image->format() != QImage::Format_ARGB32_Premultiplied)
            return;
        const qreal dpr = image->devicePixelRatio();
        const QPoint win = w->mapTo(w->window(), QPoint());
        const int ink = qRound(MINIMAP_INK * 256);
        const auto mix = [ink](QRgb base, QRgb c, bool gap) {
            if (gap)
                return base;
            const auto ch = [&](int b, int v) { return b + (((v - b) * ink) >> 8); };
            return qRgba(ch(qRed(base), qRed(c)), ch(qGreen(base), qGreen(c)), ch(qBlue(base), qBlue(c)),
                         ch(qAlpha(base), qAlpha(c)));
        };
        for (const QRect &r : region.intersected(w->rect())) {
            // cột nền phải nằm trong vùng vừa vẽ, không thì là màu cũ
            if (r.left() > MINIMAP_BASE_X || r.right() < MINIMAP_BASE_X)
                continue;
            const int x0 = qRound((win.x() + qMax(r.left(), MINIMAP_TEXT_X)) * dpr);
            const int x1 = qRound((win.x() + qMin(r.right(), w->width() - 3)) * dpr);
            const int baseX = qRound((win.x() + MINIMAP_BASE_X) * dpr);
            const int y0 = qRound((win.y() + r.top()) * dpr);
            const int y1 = qRound((win.y() + r.bottom() + 1) * dpr) - 1;
            for (int y = qMax(y0, 0); y <= qMin(y1, image->height() - 1); ++y) {
                auto *line = reinterpret_cast<QRgb *>(image->scanLine(y));
                const QRgb base = line[baseX];
                const bool gap = (qFloor(y / dpr) - win.y()) % 2 == 0;  // hàng 0 = lề trên
                for (int x = qMax(x0, 0); x <= qMin(x1, image->width() - 1); ++x)
                    line[x] = mix(base, line[x], gap);
            }
        }
    }

    // Mục 64: màu cột 0 của cột số từng hàng (toạ độ widget) — đọc bộ đệm cửa
    // sổ ngay sau khi Kate vẽ các hàng đó
    static void storeEdgeColors(QWidget *border, const QRect &rows)
    {
        QBackingStore *store = border->backingStore();
        auto *image = store ? dynamic_cast<QImage *>(store->paintDevice()) : nullptr;
        if (rows.isEmpty() || !image || image->format() != QImage::Format_ARGB32_Premultiplied)
            return;
        QByteArray colors = border->property(EDGE_COLORS).toByteArray();
        colors.resize(border->height() * int(sizeof(QRgb)), '\0');
        auto *out = reinterpret_cast<QRgb *>(colors.data());
        const qreal dpr = image->devicePixelRatio();
        const QPoint win = border->mapTo(border->window(), QPoint());
        const int x = qRound(win.x() * dpr);
        for (int y = rows.top(); y <= rows.bottom(); ++y) {
            const int iy = qRound((win.y() + y) * dpr);
            if (x >= 0 && x < image->width() && iy >= 0 && iy < image->height())
                out[y] = reinterpret_cast<const QRgb *>(image->constScanLine(iy))[x];
        }
        border->setProperty(EDGE_COLORS, colors);
    }

    // Mục 64: tô ô đệm trái cột số bằng màu đã cất, từng hàng
    static void fillGutterPad(QWidget *view, const QRegion &region)
    {
        QWidget *border = nullptr;
        for (QObject *o : view->children()) {
            if (o->inherits("KateViewInternal")) {
                for (QObject *c : o->children()) {
                    if (c->inherits("KateIconBorder"))
                        border = static_cast<QWidget *>(c);
                }
            }
            if (o->inherits("KateIconBorder"))
                border = static_cast<QWidget *>(o);
        }
        if (!border || !border->isVisible())
            return;
        const QPoint origin = border->mapTo(view, QPoint());
        const QRect pad = QRect(0, origin.y(), origin.x(), border->height()).intersected(region.boundingRect());
        QBackingStore *store = view->backingStore();
        auto *image = store ? dynamic_cast<QImage *>(store->paintDevice()) : nullptr;
        if (pad.isEmpty() || !image || image->format() != QImage::Format_ARGB32_Premultiplied)
            return;
        const QByteArray colors = border->property(EDGE_COLORS).toByteArray();
        const auto *edge = reinterpret_cast<const QRgb *>(colors.constData());
        const int known = int(colors.size() / sizeof(QRgb));
        const qreal dpr = image->devicePixelRatio();
        const QPoint win = view->mapTo(view->window(), QPoint());
        const QRect px = QRectF(QPointF(pad.topLeft() + win) * dpr, QSizeF(pad.size()) * dpr).toAlignedRect().intersected(image->rect());
        for (int y = px.top(); y <= px.bottom(); ++y) {
            const int row = qFloor(y / dpr) - win.y() - origin.y();  // hàng trong cột số
            if (row < 0 || row >= known)
                continue;
            auto *line = reinterpret_cast<QRgb *>(image->scanLine(y));
            std::fill(line + px.left(), line + px.right() + 1, edge[row]);
        }
    }

    static void clamp(QWidget *w, const QRegion &region)
    {
        QBackingStore *store = w->backingStore();
        auto *image = store ? dynamic_cast<QImage *>(store->paintDevice()) : nullptr;
        if (!image || image->format() != QImage::Format_ARGB32_Premultiplied)
            return;
        const int limit = qRound(samhie::WINDOW_OPACITY * 255);
        const qreal dpr = image->devicePixelRatio();
        const QPoint offset = w->mapTo(w->window(), QPoint());
        const QRect bounds = image->rect();
        for (const QRect &r : region.intersected(w->rect())) {
            const QRect px = QRectF(QRectF(r.translated(offset)).topLeft() * dpr, QRectF(r).size() * dpr)
                                 .toAlignedRect()
                                 .intersected(bounds);
            for (int y = px.top(); y <= px.bottom(); ++y) {
                auto *line = reinterpret_cast<QRgb *>(image->scanLine(y));
                for (int x = px.left(); x <= px.right(); ++x) {
                    const QRgb c = line[x];
                    const int a = qAlpha(c);
                    if (a <= limit)
                        continue;
                    // Màu đã nhân sẵn alpha: nhân cả bốn kênh cùng tỉ lệ
                    const auto scale = [&](int v) { return (v * limit + a / 2) / a; };
                    line[x] = qRgba(scale(qRed(c)), scale(qGreen(c)), scale(qBlue(c)), limit);
                }
            }
        }
    }

    bool m_painting = false;
};
