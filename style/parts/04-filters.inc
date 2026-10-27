// Bộ lọc nhỏ: thẻ popup, bút chì thanh đường dẫn + panel Information (mục 24), theo palette khi đổi chế độ (mục 84).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

class PopupPainter : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        auto *w = static_cast<QWidget *>(obj);
        // Viewport đổi cỡ (thanh trượt xuống/lên): vẽ lại cả thẻ — Qt chỉ vẽ
        // phần mới lộ ra, viền đáy cũ sẽ nằm lại giữa thẻ.
        if (event->type() == QEvent::Resize && !isSearchContents(w)) {
            if (auto *area = qobject_cast<QScrollArea *>(w->parentWidget()); area && area->widget())
                area->widget()->update();
            return false;
        }
        if (event->type() == QEvent::Paint && isSearchContents(w)) {
            QPainter p(w);
            p.setRenderHint(QPainter::Antialiasing);
            // Mép dưới thẻ = mép dưới phần NHÌN THẤY của khung nội dung, không
            // phải mép dưới khung: khung cao hơn thanh vài px (máy ảo
            // 2026-10-06: Dolphin tính Bar::preferredHeight với đúng 4px dưới
            // dòng nút thứ hai, khung nội dung thì theo sizeHint) → viewport
            // cắt mất viền đáy + hai góc bo. Lúc thanh đang trượt, thẻ cũng
            // co theo phần nhìn thấy — góc dưới luôn bo.
            qreal bottom = w->height();
            for (const QWidget *a = w->parentWidget(); a; a = a->parentWidget()) {
                bottom = qMin<qreal>(bottom, w->mapFrom(a, QPoint(0, a->height())).y());
                if (a->inherits("Search::Bar"))
                    break;
            }
            const QRectF card(QPointF(POPUP_MARGIN + 0.5, POPUP_MARGIN + 0.5),
                              QPointF(w->width() - POPUP_MARGIN - 0.5, bottom - 0.5));
            p.setPen(QPen(QColor(samhie::POPUP_BORDER), 1));
            p.setBrush(QColor(samhie::POPUP));
            p.drawRoundedRect(card, POPUP_RADIUS, POPUP_RADIUS);
        }
        return false;
    }
};

// Mục 24. Thanh đường dẫn không có bút chì "sửa đường dẫn" ở mép phải khi trỏ
// chuột (user 2026-10-06: trỏ vào đã hiện con trỏ chữ I). Nút KIO vẽ nó là
// cả khoảng trống bên phải thanh — bấm chỗ nào cũng vào chế độ sửa, con trỏ
// chữ I cũng do nó đặt — nên giữ nút, chỉ không vẽ: lúc chưa bấm, nút KHÔNG
// vẽ gì khác ngoài bút chì (KUrlNavigatorToggleButton::paintEvent, KIO 6.30);
// lúc đang sửa nó vẽ ✓ như cũ.
class NoEditPencil : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        return event->type() == QEvent::Paint && !static_cast<QAbstractButton *>(obj)->isChecked();
    }
};

// Mục 24. Panel Information của Dolphin (F11) luôn ẩn: Dolphin dựng lại nó
// theo trạng thái lưu lần trước (QMainWindow::restoreState — ai đã mở nó ở
// bản cũ thì nó tự hiện) → hễ được hiện là ẩn lại, ngay vòng sự kiện sau.
// Mục 30: dải sửa phím tắt (ShortcutEditWidget : TabConnectedWidget) không vẽ
// nét Highlight — paintEvent của nó chỉ có nét đó.
class NoPaint : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *, QEvent *event) override
    {
        return event->type() == QEvent::Paint;
    }
};

// Mục 84: chỗ KDE chụp màu lúc dựng, không theo khi đổi chế độ lúc đang mở:
//  - samhieFollowPalette: widget giữ bảng màu app chép cứng (tiêu đề
//    KPageView); samhieFollowAlpha: chữ mờ đi (dòng mô tả hộp About, 85%)
//  - samhieRelink: nhãn có link — Qt tô màu link lúc đọc HTML, theo bảng màu
//    app LÚC ĐÓ → đặt lại chữ cho nó đọc lại
//  - samhieRichTitle: chữ HTML trong KTitleWidget (tiêu đề hộp About) — đổi
//    màu thì KTitleWidget gắn stylesheet cỡ chữ ×1,4 lúc mở chưa có (chữ to
//    ra) → gỡ đi, màu chữ đã theo palette của nhãn
class FollowAppPalette : public QObject
{
public:
    using QObject::QObject;

    static void sync(QWidget *w)
    {
        if (w->property("samhieFollowPalette").toBool()) {
            QPalette pal = QApplication::palette(w);
            const QVariant alpha = w->property("samhieFollowAlpha");
            if (alpha.isValid()) {
                QColor text = pal.color(QPalette::WindowText);
                text.setAlphaF(alpha.toDouble());
                pal.setColor(QPalette::WindowText, text);
            }
            w->setPalette(pal);
        }
        if (w->property("samhieRelink").toBool()) {
            auto *label = static_cast<QLabel *>(w);
            const QString text = label->text();
            label->setText(QString());  // cùng chữ thì QLabel bỏ qua
            label->setText(text);
        }
    }

    static void unstyleTitle(QLabel *label)
    {
        if (!label->styleSheet().isEmpty() && Qt::mightBeRichText(label->text()))
            label->setStyleSheet(QString());
    }

    // Gỡ ở nhịp sau, KHÔNG ngay lúc polish / lúc sự kiện: KTitleWidget đặt
    // stylesheet SAU bộ lọc (changeEvent của nó), và đổi stylesheet giữa lúc
    // QApplication::setStyle duyệt widget làm Kate crash (gdb 2026-10-21:
    // KTitleWidget::changeEvent → setStyle_helper trên style đã xoá)
    static void unstyleTitleLater(QLabel *label)
    {
        QPointer<QLabel> guard = label;
        QTimer::singleShot(0, label, [guard] {
            if (guard)
                unstyleTitle(guard);
        });
    }

    // Đổi chế độ = style nạp lại: MỌI widget nhận StyleChange, nhưng không phải
    // widget nào cũng nhận ApplicationPaletteChange (đo container: nhãn
    // "File Manager…" của hộp About không nhận) → làm cả hai, ở nhịp sau
    // (lúc đó bảng màu app đã là bản mới)
    static void syncLater(QWidget *w)
    {
        QPointer<QWidget> guard = w;
        QTimer::singleShot(0, w, [guard] {
            if (guard)
                sync(guard);
        });
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        auto *w = static_cast<QWidget *>(obj);
        if (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::StyleChange)
            syncLater(w);
        if (event->type() == QEvent::StyleChange && w->property("samhieRichTitle").toBool())
            unstyleTitleLater(static_cast<QLabel *>(w));
        return false;
    }
};
