// Hộp Properties (mục 53): dòng ẩn, thanh dung lượng, tab Details, ô Comment.
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// Mục 53. Hộp Properties. Tên widget trang General: kfilepropspluginwidget.ui
// (KIO 6.30). KIO tự hiện lại vài dòng sau khi tính xong (setSizeOnDisk,
// dòng ổ đĩa) → không ẩn mà dời chúng sang một widget ẩn: setVisible(true)
// của KIO khi đó không lộ ra, layout cũng tự bỏ ô của chúng.
const QStringList PROPS_HIDDEN_ROWS = {
    QStringLiteral("sizeOnDiskLabelLeft"), QStringLiteral("sizeOnDiskLabel"),
    QStringLiteral("accessTimeLabel_Left"), QStringLiteral("accessTimeLabel"),
    QStringLiteral("fsLabel_Left"), QStringLiteral("fsLabel"),
    QStringLiteral("mountPointLabel_Left"), QStringLiteral("mountPointLabel"),
    QStringLiteral("mountSrcLabel_Left"), QStringLiteral("mountSrcLabel"),
};
constexpr int CAPACITY_BAR = 6;      // = progressbar_thickness (tools/kvantum.py)
constexpr int CAPACITY_GAP = 6;      // thanh → chữ dung lượng
constexpr qreal CAPACITY_TEXT = 0.85;  // cỡ chữ dung lượng so với chữ thường

QFont capacityFont(const QWidget *bar)
{
    QFont f = bar->font();
    f.setPointSizeF(f.pointSizeF() * CAPACITY_TEXT);
    return f;
}

// Thanh Free space: KCapacityBar vẽ thanh 12px rồi chữ cùng cỡ ngay dưới,
// cả khối canh giữa nhãn → thanh nằm cao hơn chữ "Free space:" (ảnh user
// 2026-10-08). Vẽ lại: thanh progress của Kvantum (giống mọi thanh tiến độ)
// ngang giữa dòng chữ đầu, chữ nhỏ dưới thanh, canh trái như giá trị khác.
// Nhãn canh trên (setupPropertiesDialog) để dòng chữ đầu của hai bên trùng.
class CapacityBarPainter : public QObject
{
public:
    using QObject::QObject;

    static int heightFor(const QWidget *bar)
    {
        return bar->fontMetrics().height() / 2 + CAPACITY_BAR / 2 + CAPACITY_GAP
            + QFontMetrics(capacityFont(bar)).height();
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() != QEvent::Paint)
            return false;
        auto *bar = static_cast<KCapacityBar *>(obj);
        const QRect r = bar->contentsRect();
        const int middle = r.top() + bar->fontMetrics().height() / 2;
        QPainter p(bar);
        QStyleOptionProgressBar o;
        o.initFrom(bar);
        o.rect = QRect(r.left(), middle - CAPACITY_BAR / 2, r.width(), CAPACITY_BAR);
        o.minimum = 0;
        o.maximum = 100;
        o.progress = bar->value();
        o.textVisible = false;
        o.state |= QStyle::State_Horizontal;
        bar->style()->drawControl(QStyle::CE_ProgressBar, &o, &p, bar);
        p.setFont(capacityFont(bar));
        p.setPen(bar->palette().color(QPalette::WindowText));
        const QRect text(r.left(), middle + CAPACITY_BAR / 2 + CAPACITY_GAP, r.width(), r.bottom());
        const Qt::Alignment side = bar->layoutDirection() == Qt::RightToLeft ? Qt::AlignRight : Qt::AlignLeft;
        p.drawText(text, side | Qt::AlignTop, p.fontMetrics().elidedText(bar->text(), Qt::ElideRight, r.width()));
        return true;
    }
};

void setupPropertiesDialog(QWidget *dialog)
{
    auto *holder = dialog->findChild<QWidget *>(QStringLiteral("samhie-hidden-rows"), Qt::FindDirectChildrenOnly);
    if (!holder) {
        holder = new QWidget(dialog);
        holder->setObjectName(QStringLiteral("samhie-hidden-rows"));
        holder->hide();
    }
    for (const QString &name : PROPS_HIDDEN_ROWS) {
        if (auto *row = dialog->findChild<QWidget *>(name); row && row->parentWidget() != holder)
            row->setParent(holder);
    }
    if (auto *label = dialog->findChild<QLabel *>(QStringLiteral("freespaceLabel")))
        label->setAlignment(Qt::AlignRight | Qt::AlignTop);
    if (auto *bar = dialog->findChild<KCapacityBar *>(QStringLiteral("capacityBar"))) {
        if (!bar->property("_samhie_capacity").toBool()) {
            bar->setProperty("_samhie_capacity", true);
            bar->installEventFilter(new CapacityBarPainter(bar));
            bar->setMinimumHeight(CapacityBarPainter::heightFor(bar));
        }
    }
    // Permissions: hai QFormLayout (Access Permissions, Ownership) trong
    // QGroupBox, mỗi cái tự đo cột nhãn và Kvantum canh nhãn trái.
    QList<QFormLayout *> forms;
    for (QGroupBox *box : dialog->findChildren<QGroupBox *>()) {
        if (auto *form = qobject_cast<QFormLayout *>(box->layout()))
            forms << form;
    }
    int labelWidth = 0;
    for (QFormLayout *form : forms) {
        for (int r = 0; r < form->rowCount(); ++r) {
            if (QLayoutItem *item = form->itemAt(r, QFormLayout::LabelRole); item && item->widget())
                labelWidth = qMax(labelWidth, item->widget()->sizeHint().width());
        }
    }
    for (QFormLayout *form : forms) {
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        for (int r = 0; r < form->rowCount(); ++r) {
            QLayoutItem *item = form->itemAt(r, QFormLayout::LabelRole);
            if (!item || !item->widget())
                continue;
            item->widget()->setMinimumWidth(labelWidth);
            if (auto *label = qobject_cast<QLabel *>(item->widget()))  // chữ canh phải trong ô đã nới
                label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    // Tab Details/Checksums thêm sau (Details: khi đọc xong metadata) — dải
    // tab không cắt chữ, không nút cuộn → bề ngang tối thiểu của hộp tính cả
    // dải tab, và hộp tự nới khi có tab mới.
    if (auto *tabs = dialog->findChild<QTabWidget *>()) {
        tabs->tabBar()->setElideMode(Qt::ElideNone);
        tabs->setUsesScrollButtons(false);
    }
    // KIO khoá cỡ tối thiểu = cỡ lúc dựng (gồm các dòng vừa bỏ). Bỏ khoá để
    // layout tự đặt lại, rồi co về cỡ nội dung mới.
    dialog->setMinimumSize(0, 0);
    dialog->resize(dialog->sizeHint());
}

// Chạy một lần, lúc hộp sắp hiện (Show đến trước khi cửa sổ lên màn hình):
// mọi trang đã dựng, mọi widget đã polish.
class PropertiesDialogSetup : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Show && !obj->property("_samhie_props").toBool()) {
            obj->setProperty("_samhie_props", true);
            setupPropertiesDialog(static_cast<QWidget *>(obj));
        }
        return false;
    }
};

// Tab Details (baloo-widgets 26.08.1 FileMetaDataWidget): lưới nhãn | khe |
// giá trị, cột nhãn giãn 4 phần, giá trị 6 phần → nhãn trôi ra giữa hộp.
// Cột nhãn không giãn, rộng sao cho giá trị bắt đầu đúng chỗ giá trị trang
// General (mimeCommentLabel); ô Comment kéo tới mép phải ô tên file. Đo lại
// mỗi lần lưới đổi — baloo dựng lại lưới khi đọc xong metadata. Dòng đầu
// cách dải tab như icon đầu trang General (lưới của baloo không có lề).
constexpr int COMMENT_LINES = 4;

void alignDetails(QWidget *meta)
{
    QWidget *dialog = meta->window();
    auto *grid = qobject_cast<QGridLayout *>(meta->layout());
    auto *general = dialog->findChild<QWidget *>(QStringLiteral("mimeCommentLabel"));
    auto *nameEdit = dialog->findChild<QWidget *>(QStringLiteral("fileNameLineEdit"));
    if (!meta->isVisible() || !grid || !general || !dialog->inherits("KPropertiesDialog"))
        return;
    QWidget *value = nullptr;
    for (int r = 0; r < grid->rowCount() && !value; ++r) {
        if (QLayoutItem *item = grid->itemAtPosition(r, 2); item && item->widget() && !item->widget()->isHidden())
            value = item->widget();
    }
    if (!value)
        return;
    if (grid->columnStretch(0) != 0 || grid->columnStretch(2) != 1) {
        grid->setColumnStretch(0, 0);
        grid->setColumnStretch(2, 1);
        return;  // đo lại sau khi lưới xếp lại (LayoutRequest)
    }
    QWidget *icon = dialog->findChild<QWidget *>(QStringLiteral("iconButton"));
    if (!icon || icon->isHidden())
        icon = dialog->findChild<QWidget *>(QStringLiteral("iconLabel"));
    if (icon && !icon->isHidden()) {
        QMargins m = grid->contentsMargins();
        const int top = m.top() + icon->mapTo(dialog, QPoint()).y() - value->mapTo(dialog, QPoint()).y();
        if (top >= 0 && top != m.top()) {
            m.setTop(top);
            grid->setContentsMargins(m);
            return;
        }
    }
    const int delta = general->mapTo(dialog, QPoint()).x() - value->mapTo(dialog, QPoint()).x();
    if (delta != 0) {
        grid->setColumnMinimumWidth(0, qMax(0, grid->cellRect(0, 0).width() + delta));
        return;
    }
    if (!nameEdit || nameEdit->isHidden())
        return;
    for (QWidget *c : meta->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        if (!c->inherits("KCommentWidget"))
            continue;
        const int right = nameEdit->mapTo(dialog, QPoint(nameEdit->width(), 0)).x();
        const int width = right - c->mapTo(dialog, QPoint()).x();
        if (width > 0 && c->minimumWidth() != width)
            c->setMinimumWidth(width);
    }
}

class DetailsAlign : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        auto *w = static_cast<QWidget *>(obj);
        switch (event->type()) {
        case QEvent::LayoutRequest:
        case QEvent::Show:
        case QEvent::Resize:
            QTimer::singleShot(0, w, [w] { alignDetails(w); });
            break;
        default:
            break;
        }
        return false;
    }
};

// Ô Comment: KCommentWidget đặt chiều cao CỐ ĐỊNH = số dòng chữ + 1
// (setHeightToFitContent, mỗi lần chữ đổi) → trống thì chỉ 2 dòng. Giữ ít
// nhất COMMENT_LINES dòng, chữ dài hơn thì để KIO nới.
class CommentHeight : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Show) {
            auto *edit = static_cast<QPlainTextEdit *>(obj);
            QTimer::singleShot(0, edit, [edit] {
                const QMargins m = edit->contentsMargins();
                const int want = COMMENT_LINES * QFontMetrics(edit->font()).lineSpacing() + m.top() + m.bottom();
                if (edit->height() < want)
                    edit->setFixedHeight(want);
            });
        }
        return false;
    }
};
