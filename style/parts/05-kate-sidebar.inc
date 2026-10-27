// Kate: nút icon cột trái/phải (mục 66), ô gõ Find Action không đóng (mục 57).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// Mục 66: nút chỉ-có-icon của cột trái/phải Kate (Kate xoá chữ ở đó,
// Sidebar::updateButtonStyle); nút có chữ ở đáy không tính.
bool isIconOnlySidebarTab(const QWidget *w)
{
    auto *b = qobject_cast<const QAbstractButton *>(w);
    return b && w->inherits("KMultiTabBarTab") && b->text().isEmpty();
}

// Mục 66: style tự vẽ cả nút (thay paintEvent của nút VÀ bộ lọc Kvantum —
// bộ lọc đó tô đặc nút ở trạng thái thường). Cài SAU Kvantum nên chạy TRƯỚC.
// Không dùng paintEvent của KMultiTabBarTab: nút đang mở (và mọi nút nếu
// Kate dùng kiểu KDEV3ICON) đặt icon theo lề tính từ ô CHUẨN 48 của
// sizeFromContents, không theo chiều cao thật → plugin Kate hạ nút xuống
// cao bằng hàng tên dự án (mục 13 của plugin) thì icon lệch lên nửa phần
// chênh (máy ảo 2026-10-13: ô 41, icon cao hơn tâm 3,5px).
class SidebarTabPaint : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() != QEvent::Paint || !isIconOnlySidebarTab(static_cast<QWidget *>(obj)))
            return false;
        auto *b = static_cast<QAbstractButton *>(obj);
        QPainter p(b);
        if (b->isChecked() || b->isDown()) {
            QColor c(samhie::SIDEBAR_TAB_OPEN);
            c.setAlphaF(samhie::WINDOW_OPACITY);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(b->rect(), c);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
        // Như KMultiTabBarTab::paintEvent (KWidgetsAddons 6.30): luôn trạng
        // thái On, sáng (Active) khi trỏ chuột
        const QIcon::Mode mode = !b->isEnabled() ? QIcon::Disabled : b->underMouse() ? QIcon::Active : QIcon::Normal;
        const QPixmap pixmap = b->icon().pixmap(b->iconSize(), b->devicePixelRatioF(), mode, QIcon::On);
        b->style()->drawItemPixmap(&p, b->rect(), Qt::AlignCenter, pixmap);
        return true;
    }
};

// Mục 57: ô gõ của KCommandBar không đóng vì popup (menu) vừa đóng. Lọc này
// cài SAU lọc của KCommandBar nên chạy TRƯỚC nó.
class KeepCommandBarFocus : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Show)
            obj->setProperty("samhieShownAt", QDateTime::currentMSecsSinceEpoch());
        if (event->type() != QEvent::FocusOut)
            return false;
        const Qt::FocusReason reason = static_cast<QFocusEvent *>(event)->reason();
        const qint64 age = QDateTime::currentMSecsSinceEpoch() - obj->property("samhieShownAt").toLongLong();
        if (reason != Qt::PopupFocusReason && !(reason == Qt::ActiveWindowFocusReason && age < 1000))
            return false;
        auto *w = static_cast<QWidget *>(obj);
        QTimer::singleShot(0, w, [w] { w->setFocus(Qt::OtherFocusReason); });
        return true;
    }
};
