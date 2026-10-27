// Danh sách hộp cài đặt (mục 37, 30), dock, popup neo góc (26), thanh đường dẫn (20, 51).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// Mục 37: danh sách trong hộp thoại cài đặt (không tính danh sách trang).
bool isSettingsList(const QWidget *w)
{
    return qobject_cast<const QListView *>(w) && !w->inherits("KDEPrivate::KPageListView")
        && w->window()->inherits("KPageDialog");
}

// Mục 30: danh sách dùng KExtendableItemDelegate (ô có dải "›" bên trái).
const QAbstractItemView *extendableView(const QStyleOption *opt, const QWidget *w)
{
    auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
    auto *view = qobject_cast<const QAbstractItemView *>(w ? w : (vopt ? vopt->widget : nullptr));
    return view && view->itemDelegate() && view->itemDelegate()->inherits("KExtendableItemDelegate") ? view : nullptr;
}

class HideDock : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::ShowToParent)
            QMetaObject::invokeMethod(static_cast<QWidget *>(obj), &QWidget::hide, Qt::QueuedConnection);
        return false;
    }
};

// Mục 26. Popup neo góc trái — đặt lúc Show: cửa sổ nền (QWindow) đã có, Qt
// chưa gửi vị trí cho compositor (show_sys chạy sau QShowEvent).
class PopupAnchor : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Show) {
            if (QWindow *win = static_cast<QWidget *>(obj)->windowHandle())
                win->setProperty("_q_waylandPopupAnchor", QVariant::fromValue(Qt::Edges(Qt::TopEdge | Qt::LeftEdge)));
        }
        return false;
    }
};

// Mục 20. Màu chữ nút trên thanh đường dẫn khi trỏ chuột. Palette rỗng = lại
// thừa hưởng từ cha.
class UrlButtonHover : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        auto *w = static_cast<QWidget *>(obj);
        if (event->type() == QEvent::Enter || event->type() == QEvent::DragEnter) {
            QPalette pal = w->palette();
            pal.setColor(QPalette::Text, QColor(samhie::HOVER_ACCENT));
            w->setPalette(pal);
        } else if (event->type() == QEvent::Leave || event->type() == QEvent::DragLeave
                   || event->type() == QEvent::Hide) {
            w->setPalette(QPalette());
        }
        return false;
    }
};

// Mục 51. KUrlNavigator (KIO 6.30, updateButtonVisibility) chỉ hiện mũi tên
// đầu thanh (KUrlNavigatorDropDownButton) khi còn thư mục cha → thiếu nó thì
// tên thư mục đầu dính mép khung. Lề trái của layout thanh = bề ngang mũi
// tên (sizeHint: nửa chiều cao) khi mũi tên ẩn và đang ở dạng nút (ô sửa
// đường dẫn KUrlComboBox ẩn). Nghe Show/Hide của mũi tên và ô sửa; xét sau
// một nhịp vì KIO đổi hai cái lần lượt.
// Mục 74: chữ đều nét (<tt> <code> <pre>) trong nhãn rich text cùng cỡ mắt
// thấy với chữ thường. Qt vẽ đoạn đó bằng font đều nét ở CÙNG cỡ điểm của
// nhãn — Dongle 20pt trông như ~11pt nên Hack 20pt to gần gấp đôi (máy ảo user
// 2026-10-13: hộp xoá file của KIO, đường dẫn trong <tt> tràn ra hai mép).
// Ghi cỡ vào thẻ trước khi nhãn vẽ, theo tỉ lệ [fonts] mono-size/size; dấu
// MONO_MARK để không ghi lại.
class MonoSpans : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Paint)
            fix(static_cast<QLabel *>(obj));
        return false;
    }

private:
    static void fix(QLabel *label)
    {
        static const QString mark = QStringLiteral("<!--samhie-mono-->");
        static const QRegularExpression tag(QStringLiteral("<(tt|code|pre)(?=[\\s>])"),
                                            QRegularExpression::CaseInsensitiveOption);
        const QString text = label->text();
        if (label->textFormat() == Qt::PlainText || text.contains(mark) || !text.contains(tag)
            || (label->textFormat() == Qt::AutoText && !Qt::mightBeRichText(text)))
            return;
        const qreal size = label->font().pointSizeF();
        if (size <= 0)
            return;
        QString sized = text;
        sized.replace(tag, QStringLiteral("<\\1 style=\"font-size:%1pt\"").arg(size * samhie::FONT_MONO_SIZE / samhie::FONT_SIZE));
        label->setText(sized + mark);
    }
};

class CrumbIndent : public QObject
{
public:
    using QObject::QObject;

    static bool watches(const QWidget *w)
    {
        return w && w->parentWidget() && w->parentWidget()->inherits("KUrlNavigator")
            && (w->inherits("KDEPrivate::KUrlNavigatorDropDownButton") || w->inherits("KUrlComboBox"));
    }

    static void schedule(QWidget *nav)
    {
        QTimer::singleShot(0, nav, [nav] {
            QWidget *dropDown = nullptr, *pathBox = nullptr;
            for (QWidget *c : nav->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
                if (c->inherits("KDEPrivate::KUrlNavigatorDropDownButton"))
                    dropDown = c;
                else if (c->inherits("KUrlComboBox"))
                    pathBox = c;
            }
            if (!dropDown || !nav->layout())
                return;
            const int indent = dropDown->isHidden() && (!pathBox || pathBox->isHidden())
                ? dropDown->sizeHint().width() : 0;
            QMargins m = nav->layout()->contentsMargins();
            if (nav->layoutDirection() == Qt::RightToLeft)
                m.setRight(indent);
            else
                m.setLeft(indent);
            if (m != nav->layout()->contentsMargins())
                nav->layout()->setContentsMargins(m);
        });
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Show || event->type() == QEvent::Hide)
            schedule(static_cast<QWidget *>(obj)->parentWidget());
        return false;
    }
};
