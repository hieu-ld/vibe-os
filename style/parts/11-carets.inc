// Dấu nháy ngắn (mục 22), ô đổi tên Dolphin (49), nút trong dòng menu (12).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// Mục 22. Vị trí lấy từ ImCursorRectangle (toạ độ ô nhập): đỉnh = đỉnh hộp
// dòng; Qt nới ô đó 5px mỗi bên quanh dấu nháy (rectForPos) → lấy giữa ô. Nhấp nháy theo nhịp hệ thống, hiện lại ngay khi
// gõ / dời con trỏ như Qt.
class ShortCaret : public QWidget
{
public:
    explicit ShortCaret(QLineEdit *edit)
        : QWidget(edit)
        , m_edit(edit)
    {
        setObjectName(QStringLiteral("samhie-caret"));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        const int flash = QApplication::cursorFlashTime();
        m_timer.setInterval(flash > 0 ? flash / 2 : 0);
        QObject::connect(&m_timer, &QTimer::timeout, this, [this] {
            m_on = !m_on;
            update();
        });
        QObject::connect(edit, &QLineEdit::cursorPositionChanged, this, [this] { restart(); });
        QObject::connect(edit, &QLineEdit::textChanged, this, [this] { restart(); });
        edit->installEventFilter(this);
        edit->setProperty("samhieShortCaret", true);
        setGeometry(edit->rect());
        raise();
        if (edit->hasFocus())
            restart();
    }

protected:
    bool eventFilter(QObject *, QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::Resize:
            setGeometry(m_edit->rect());
            break;
        case QEvent::FocusIn:
            restart();
            break;
        case QEvent::FocusOut:
            m_timer.stop();
            update();
            break;
        default:
            break;
        }
        return false;
    }

    void paintEvent(QPaintEvent *) override
    {
        if (!m_on || !m_edit->hasFocus() || m_edit->isReadOnly())
            return;
        const QRect line = m_edit->inputMethodQuery(Qt::ImCursorRectangle).toRect();
        const QFontMetricsF fm(m_edit->font());
        const qreal base = line.top() + fm.ascent();
        const qreal top = qRound(base - CARET_UP * fm.capHeight());
        const qreal bottom = qRound(base + CARET_DOWN * fm.capHeight());
        QPainter p(this);
        p.fillRect(QRectF(line.left() + line.width() / 2, top, 1, bottom - top),
                   m_edit->palette().color(QPalette::Text));
    }

private:
    static constexpr qreal CARET_UP = 1.3;    // hơi trên đỉnh chữ hoa
    static constexpr qreal CARET_DOWN = 0.4;  // tới chân chữ g, y

    void restart()
    {
        m_on = true;
        if (m_edit->hasFocus() && m_timer.interval() > 0)
            m_timer.start();
        update();
    }

    QLineEdit *m_edit;
    QTimer m_timer;
    bool m_on = true;
};

// Mục 49: ô đổi tên tại chỗ của Dolphin (KItemListRoleEditor — QTextEdit,
// kitemlistroleeditor.cpp 26.08.1). Dolphin đặt ô bằng hộp dòng của chữ
// (Dongle: cao 1,45 em, chữ hoa 0,4 em) → ô cao gấp mấy lần chữ, sát icon
// thư mục (user 2026-10-08). Giữ nguyên khung Dolphin đặt (chữ đứng đúng chỗ
// nhãn cũ), nền trong suốt; style vẽ ô bo góc ôm các dòng chữ + RENAME_PAD.
constexpr qreal RENAME_UP = 1.45;   // trên chân chữ: chữ hoa có dấu (Ố, Ẫ)
constexpr qreal RENAME_DOWN = 0.4;  // dưới chân chữ: chân g, y
constexpr int RENAME_PAD = 6;

bool isRenameEditor(const QWidget *w)
{
    return w && w->inherits("KItemListRoleEditor") && qobject_cast<const QTextEdit *>(w);
}

// Dải chữ (theo chân chữ) của từng dòng, toạ độ ô đổi tên: (dòng, x1, x2,
// chân chữ). Dòng = QTextLine trong khối đầu (tên file là một khối).
struct RenameLine {
    qreal x1, x2, baseline;
};

QList<RenameLine> renameLines(const QTextEdit *te, int from = -1, int to = -1)
{
    QList<RenameLine> out;
    const QTextBlock block = te->document()->firstBlock();
    const QTextLayout *layout = block.layout();
    if (!layout)
        return out;
    const QPointF origin = QPointF(te->frameWidth() - te->horizontalScrollBar()->value(),
                                   te->frameWidth() - te->verticalScrollBar()->value())
        + te->document()->documentLayout()->blockBoundingRect(block).topLeft() + layout->position();
    for (int i = 0; i < layout->lineCount(); ++i) {
        const QTextLine line = layout->lineAt(i);
        int a = line.textStart(), b = a + line.textLength();
        if (from >= 0) {
            a = qMax(a, from);
            b = qMin(b, to);
            if (a >= b)
                continue;
        }
        out.append({origin.x() + line.cursorToX(a), origin.x() + line.cursorToX(b),
                    origin.y() + line.y() + line.ascent()});
    }
    return out;
}

QRect renameBox(const QWidget *w)
{
    auto *te = static_cast<const QTextEdit *>(w);
    const QFontMetricsF fm(te->font());
    const QList<RenameLine> lines = renameLines(te);
    const qreal top = lines.isEmpty() ? te->frameWidth() + fm.ascent() : lines.first().baseline;
    const qreal bottom = lines.isEmpty() ? top : lines.last().baseline;
    const int y0 = qRound(top - RENAME_UP * fm.capHeight()) - RENAME_PAD;
    const int y1 = qRound(bottom + RENAME_DOWN * fm.capHeight()) + RENAME_PAD;
    return QRect(0, y0, w->width(), y1 - y0).intersected(w->rect());
}

// Dấu nháy ngắn cho ô đổi tên (như ShortCaret của ô nhập một dòng): Qt vẽ
// cao bằng hộp dòng → cursorWidth 0, lớp con trên viewport vẽ vạch theo chữ.
class TextEditCaret : public QWidget
{
public:
    explicit TextEditCaret(QTextEdit *edit)
        : QWidget(edit->viewport())
        , m_edit(edit)
    {
        setObjectName(QStringLiteral("samhie-text-caret"));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        edit->setCursorWidth(0);
        const int flash = QApplication::cursorFlashTime();
        m_timer.setInterval(flash > 0 ? flash / 2 : 0);
        QObject::connect(&m_timer, &QTimer::timeout, this, [this] {
            m_on = !m_on;
            update();
        });
        QObject::connect(edit, &QTextEdit::cursorPositionChanged, this, [this] { restart(); });
        QObject::connect(edit, &QTextEdit::textChanged, this, [this] { restart(); });
        edit->viewport()->installEventFilter(this);
        edit->installEventFilter(this);
        setGeometry(edit->viewport()->rect());
        raise();
        restart();
    }

protected:
    bool eventFilter(QObject *o, QEvent *event) override
    {
        if (event->type() == QEvent::Resize && o == m_edit->viewport())
            setGeometry(m_edit->viewport()->rect());
        else if (event->type() == QEvent::FocusIn)
            restart();
        else if (event->type() == QEvent::FocusOut) {
            m_timer.stop();
            update();
        }
        return false;
    }

    void paintEvent(QPaintEvent *) override
    {
        if (!m_on || !m_edit->hasFocus())
            return;
        const QRect line = m_edit->cursorRect();
        const QFontMetricsF fm(m_edit->font());
        const qreal base = line.top() + fm.ascent();
        QPainter p(this);
        p.fillRect(QRectF(line.left(), qRound(base - 1.3 * fm.capHeight()), 1,
                          qRound(base + 0.4 * fm.capHeight()) - qRound(base - 1.3 * fm.capHeight())),
                   m_edit->palette().color(QPalette::Text));
    }

private:
    void restart()
    {
        m_on = true;
        if (m_edit->hasFocus() && m_timer.interval() > 0)
            m_timer.start();
        update();
    }

    QTextEdit *m_edit;
    QTimer m_timer;
    bool m_on = true;
};

// Mục 12. Gửi bằng postEvent (hàng đợi giữ thứ tự bấm → thả): nút có thể mở
// danh sách bằng exec() — không chạy vòng lặp lồng trong lúc menu con còn
// đang xử lý cú bấm của chính nó.
class MenuButtonForwarder : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease)
            return false;
        auto *menu = static_cast<QMenu *>(obj);
        auto *me = static_cast<QMouseEvent *>(event);
        auto *button = qobject_cast<QAbstractButton *>(menu->childAt(me->position().toPoint()));
        if (!button || !button->isEnabled())
            return false;
        if (event->type() == QEvent::MouseButtonPress) {
            const auto actions = menu->actions();
            for (QAction *action : actions) {
                if (action->menu() && action->menu()->isVisible())
                    action->menu()->hide();
            }
        }
        const QPointF local = button->mapFrom(menu, me->position());
        QCoreApplication::postEvent(button, new QMouseEvent(event->type(), local, me->scenePosition(),
                                                            me->globalPosition(), me->button(), me->buttons(),
                                                            me->modifiers(), me->pointingDevice()));
        return true;
    }
};
