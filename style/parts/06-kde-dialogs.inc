// Hộp thoại KDE: phím tắt (mục 35), menu Help (40), trang Context Menu (38), Find Action (41, 75), danh sách trang (77), bảng màu Theme Editor (86).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// Mục 35: hành động không còn dùng được ở Samhie → bỏ khỏi danh sách phím tắt.
// Đo 2026-10-07 (Dolphin 26.08.1, danh sách đầy đủ in từ hộp thoại).
const QSet<QString> REMOVED_SHORTCUTS = {
    // dolphin-ui.sh ẩn (visible=false → phím tắt chết theo)
    QStringLiteral("compact"), QStringLiteral("toggle_selection_mode"), QStringLiteral("view_properties"),
    // Selection Mode bản thanh công cụ: cùng tính năng đã bỏ
    QStringLiteral("toggle_selection_mode_tool_bar"),
    // Group By / Show Additional Information (dolphin-ui.sh ẩn menu cha)
    QStringLiteral("group_by_none"), QStringLiteral("group_by_same_as_sort"),
    QStringLiteral("group_accesstime"), QStringLiteral("group_comment"), QStringLiteral("group_creationtime"),
    QStringLiteral("group_modificationtime"), QStringLiteral("group_text"), QStringLiteral("group_rating"),
    QStringLiteral("group_size"), QStringLiteral("group_tags"), QStringLiteral("group_type"),
    QStringLiteral("show_accesstime"), QStringLiteral("show_comment"), QStringLiteral("show_creationtime"),
    QStringLiteral("show_modificationtime"), QStringLiteral("show_rating"), QStringLiteral("show_size"),
    QStringLiteral("show_tags"), QStringLiteral("show_type"),
    // Panel Information (style mục 24: hành động ẩn, F11 chết)
    QStringLiteral("show_information_panel"),
    // Dolphin tự ẩn khi thiếu phần mềm mà image không cài: kio-stash, KFind
    QStringLiteral("split_stash"), QStringLiteral("open_preferred_search_tool"),
    // Bỏ cùng tầng Configure của menu ☰ (mục 29)
    QStringLiteral("window_color_sheme"), QStringLiteral("switch_application_language"),
    QStringLiteral("options_configure_toolbars"),
    // Đợt 2 (user 2026-10-07: "dòng nào dư thừa thì bỏ luôn"):
    // cần app image không cài — Kompare; Filelight (không có thì hành động chỉ
    // mời cài)
    QStringLiteral("compare_files"), QStringLiteral("manage_disk_space"),
    // sắp theo dữ liệu Baloo: image không có bộ đánh chỉ mục, và chỗ gắn nhãn/
    // điểm/ghi chú đã bỏ (Assign Tags — mục 38, panel Information — mục 24)
    QStringLiteral("sort_by_rating"), QStringLiteral("sort_by_tags"), QStringLiteral("sort_by_comment"),
    // Unlock Panels: đã gỡ khỏi menu sidebar (mục 19) — mở khoá phá bố cục
    QStringLiteral("lock_panels"),
    // bản "toggle" của Search…/Filter… (Ctrl+F, Ctrl+I) — cùng việc, chỉ để
    // gắn lên thanh công cụ; nút 🔍 vẫn chạy
    QStringLiteral("toggle_search"), QStringLiteral("toggle_filter"),
    // không phải lệnh: "Location Bar" là chính thanh đường dẫn đặt trên thanh
    // công cụ; "Change View Mode" là nút chọn chế độ xem mà thanh của Samhie
    // không dùng (có ▦, Icons Ctrl+1, Details Ctrl+2)
    QStringLiteral("url_navigators"), QStringLiteral("view_mode"),
    // Mục 40: menu ☰ bỏ More và bớt Help (user 2026-10-07); dolphin-ui.sh xoá
    // luôn phím tắt Ctrl+M, Ctrl+B, F1, Shift+F1
    QStringLiteral("options_show_menubar"),
    QStringLiteral("add_bookmark"), QStringLiteral("add_bookmarks_list"), QStringLiteral("edit_bookmarks"),
    QStringLiteral("help_contents"), QStringLiteral("help_whats_this"), QStringLiteral("help_report_bug"),
    QStringLiteral("help_donate"), QStringLiteral("help_about_kde"),
};

// Mục 40: dòng bỏ khỏi menu Help (tên hành động của KHelpMenu, KF 6.30).
const QSet<QString> HELP_HIDDEN = {
    QStringLiteral("help_contents"),    // Dolphin Handbook: tài liệu theo giao diện Dolphin gốc
    QStringLiteral("help_whats_this"),  // What's This?: đã có tooltip
    QStringLiteral("help_report_bug"),  // Report Bug / Donate: tới KDE, không phải Samhie
    QStringLiteral("help_donate"),
    QStringLiteral("help_about_kde"),   // About KDE: Samhie không dùng Plasma
};

// Mục 35: xoá dòng của hành động trong REMOVED_SHORTCUTS, rồi nhóm con rỗng.
// ObjectRole = Qt::UserRole + 2 (kshortcutsdialog_p.h, KF 6.30) trả QAction.
void trimShortcutList(QTreeWidgetItem *parent)
{
    for (int i = parent->childCount() - 1; i >= 0; --i) {
        QTreeWidgetItem *it = parent->child(i);
        auto *a = qobject_cast<QAction *>(it->data(0, Qt::UserRole + 2).value<QObject *>());
        if (a && REMOVED_SHORTCUTS.contains(a->objectName())) {
            delete it;
            continue;
        }
        if (!a && it->childCount()) {
            trimShortcutList(it);
            if (!it->childCount())
                delete it;
        }
    }
}

// Mục 38: dòng trang Context Menu không dùng được ở Samhie (id =
// ServiceModel::DesktopEntryNameRole, contextmenusettingspage.cpp 26.08.1).
const QSet<QString> CONTEXT_MENU_HIDDEN = {
    // Assign Tags: nhãn cần Baloo (image không cài) — kservicemenurc đã tắt
    QStringLiteral("tagsfileitemaction"),
    // Link Files to Activities: Activity của Plasma, Hyprland không có
    QStringLiteral("kactivitymanagerd_fileitem_linking_plugin"),
    // Dropbox (cần ứng dụng Dropbox), Bazaar (quản lý phiên bản đã ngừng),
    // Mercurial/Subversion (plugin gọi lệnh hg/svn — image không cài; cài
    // trong toolbox thì Dolphin ở ngoài cũng không thấy)
    QStringLiteral("_version_control_Dropbox"),
    QStringLiteral("_version_control_Bazaar"),
    QStringLiteral("_version_control_Mercurial"),
    QStringLiteral("_version_control_Subversion"),
};

class ContextMenuRowFilter : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::DecorationRole)  // mục 39
            return {};
        return QSortFilterProxyModel::data(index, role);
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        // Dòng mới chèn chưa có id (addRow: insertRow rồi mới setData) →
        // nhận trước, setData đổi dữ liệu thì lọc lại (dynamicSortFilter).
        const QString id = sourceModel()->index(row, 0, parent).data(Qt::UserRole).toString();
        return !CONTEXT_MENU_HIDDEN.contains(id);
    }
};

// Mục 41: Find Action… (KCommandBar) không liệt kê hành động Samhie đã bỏ.
// KCommandBarModel (KF 6.30) trả QAction ở Qt::UserRole, chỉ bỏ hành động
// TẮT, và lấy cả hành động con trong menu của hành động đã ẩn (đo container
// 2026-10-07: Group By > Audio…, Window Color Scheme: Breeze…).
class CommandBarFilter : public QSortFilterProxyModel
{
public:
    CommandBarFilter(QWidget *window, QObject *parent)
        : QSortFilterProxyModel(parent)
        , m_window(window)
    {
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        return !hidden(sourceModel()->index(row, 0, parent).data(Qt::UserRole).value<QAction *>());
    }

private:
    static bool removed(const QAction *a)
    {
        return !a->isVisible() || REMOVED_SHORTCUTS.contains(a->objectName())
            || a->objectName() == QLatin1String("new_bookmark_folder")  // menu Bookmarks (mục 40)
            || a->inherits("KToggleToolBarAction");                     // Show Toolbar (More → Settings)
    }

    bool hidden(const QAction *a, int depth = 0) const
    {
        if (!a)
            return false;
        if (removed(a))
            return true;
        // Nhóm vai trò của Group By / Show Additional Information: mọi
        // hành động có tên (trừ menu con Audio, Image…) đều đã bỏ. Dòng con
        // của menu con: nhóm của nó thuộc hành động menu con → xét lên.
        if (const QActionGroup *g = a->actionGroup(); g && depth < 3) {
            bool any = false, all = true;
            for (const QAction *m : g->actions()) {
                if (m->objectName().isEmpty() || m->menu())
                    continue;
                any = true;
                all = all && removed(m);
            }
            if (any && all)
                return true;
            if (hidden(qobject_cast<const QAction *>(g->parent()), depth + 1))
                return true;
        }
        // Dòng chỉ nằm trong menu của hành động đã bỏ — Window Color Scheme
        // (mục 29): Dolphin gắn menu của KColorSchemeMenu vào một QAction
        // con của cửa sổ (dolphinmainwindow.cpp 26.08.1).
        if (!m_scanned) {
            m_scanned = true;
            for (const QAction *w : m_window ? m_window->findChildren<QAction *>() : QList<QAction *>()) {
                if (w->menu() && removed(w))
                    m_hiddenMenus.insert(w->menu());
            }
        }
        bool inMenu = false;
        for (const QObject *o : a->associatedObjects()) {
            if (auto *menu = qobject_cast<const QMenu *>(o)) {
                if (!m_hiddenMenus.contains(menu))
                    return false;
                inMenu = true;
            }
        }
        return inMenu;
    }

    QPointer<QWidget> m_window;
    mutable bool m_scanned = false;
    mutable QSet<const QMenu *> m_hiddenMenus;
};

// Mục 69, 75: ô đang chọn / chỉ trỏ chuột trong danh sách của thẻ popup.
// Dòng nhiều cột (Ctrl+Tab của Kate: thư mục | tên file) — mỗi ô là một
// đoạn của cùng một dải, chỉ bo ở đầu và cuối dòng.
void paintPopupItem(QPainter *p, const QRectF &rect, bool selected,
                    QStyleOptionViewItem::ViewItemPosition pos = QStyleOptionViewItem::OnlyOne)
{
    constexpr qreal R = samhie::MENUITEM_RADIUS;
    QColor fill(samhie::POPUP_ITEM);
    if (!selected)
        fill.setAlphaF(0.5);  // chỉ trỏ chuột: nhạt hơn ô đang chọn
    QRectF band = rect;
    if (pos == QStyleOptionViewItem::Beginning || pos == QStyleOptionViewItem::Middle)
        band.setRight(band.right() + R);
    if (pos == QStyleOptionViewItem::End || pos == QStyleOptionViewItem::Middle)
        band.setLeft(band.left() - R);
    p->save();
    p->setClipRect(rect, Qt::IntersectClip);
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(fill);
    p->drawRoundedRect(band, R, R);
    p->restore();
}

// Mục 77: trang đang chọn trong danh sách trang hộp cài đặt — nền, không viền
void paintPageSelected(QPainter *p, const QRectF &rect)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(samhie::PAGE_ITEM_SELECTED));
    p->drawRoundedRect(rect, samhie::ITEM_RADIUS, samhie::ITEM_RADIUS);
    p->restore();
}

// Chế độ của style đang nạp (mục 80) — cho chỗ vẽ không đi qua style.
bool g_light = false;

// Mục 77: bọc delegate danh sách trang dạng icon (KPageListViewDelegate —
// Configure Dolphin/Konsole). KDE vẽ icon trang đang chọn ở chế độ Selected:
// bộ icon tô chế độ đó bằng màu chữ-trên-cyan của CẢ app (KIconLoader, không
// theo palette widget) → bản sáng icon trắng trên ô PAGE_ITEM_SELECTED xám
// (máy ảo user 2026-10-20). Bản sáng: tô ô chọn (PE_PanelItemViewItem — mục
// 77 trong drawPrimitive) rồi để KDE vẽ icon + chữ như mục thường.
class PageListRow : public QAbstractItemDelegate
{
public:
    PageListRow(QAbstractItemDelegate *inner, QAbstractItemView *view)
        : QAbstractItemDelegate(view)
        , m_inner(inner)
    {
    }

    static void attach(QAbstractItemView *view)
    {
        QAbstractItemDelegate *inner = view->itemDelegate();
        if (inner && !dynamic_cast<PageListRow *>(inner))
            view->setItemDelegate(new PageListRow(inner, view));
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return m_inner->sizeHint(option, index);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (!g_light || !(option.state & QStyle::State_Selected)) {
            m_inner->paint(p, option, index);
            return;
        }
        QStyle *style = option.widget ? option.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, option.widget);
        QStyleOptionViewItem o(option);
        o.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver);
        m_inner->paint(p, o, index);
    }

    bool editorEvent(QEvent *e, QAbstractItemModel *m, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return m_inner->editorEvent(e, m, o, i);
    }

    bool helpEvent(QHelpEvent *e, QAbstractItemView *v, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return m_inner->helpEvent(e, v, o, i);
    }

private:
    QAbstractItemDelegate *m_inner;
};

// Mục 86: bọc delegate bảng màu Theme Editor Kate — nền ô trỏ tô TRƯỚC, ô màu
// + nút reset của KTextEditor vẽ đè lên. Dòng tiêu đề nhóm ("Editor Background
// Colors") KTextEditor không tô nền → giữ vậy. Trạng thái trỏ vẫn đưa xuống
// (nút reset sáng lên khi trỏ); nền ô thì CE_ItemViewItem bỏ (mục 86 ở đó).
class HoverUnderRow : public QAbstractItemDelegate
{
public:
    HoverUnderRow(QAbstractItemDelegate *inner, QAbstractItemView *view)
        : QAbstractItemDelegate(view)
        , m_inner(inner)
    {
    }

    static void attach(QAbstractItemView *view)
    {
        QAbstractItemDelegate *inner = view->itemDelegate();
        if (inner && !dynamic_cast<HoverUnderRow *>(inner))
            view->setItemDelegate(new HoverUnderRow(inner, view));
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return m_inner->sizeHint(option, index);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        // Nền ô = CE_ItemViewItem không chữ/icon: Kvantum tô ô trỏ ở đó,
        // PE_PanelItemViewItem gọi riêng thì nó không tô (container 2026-10-21)
        if ((option.state & QStyle::State_MouseOver) && index.parent().isValid()) {
            QStyleOptionViewItem o(option);
            o.index = index;
            o.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);
            o.features = QStyleOptionViewItem::None;
            QStyle *style = option.widget ? option.widget->style() : QApplication::style();
            s_panel = true;
            style->drawControl(QStyle::CE_ItemViewItem, &o, p, option.widget);
            s_panel = false;
        }
        m_inner->paint(p, option, index);
    }

    static inline bool s_panel = false;  // đang tô nền ô (CE_ItemViewItem giữ trạng thái trỏ)

    bool editorEvent(QEvent *e, QAbstractItemModel *m, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return m_inner->editorEvent(e, m, o, i);
    }

    bool helpEvent(QHelpEvent *e, QAbstractItemView *v, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return m_inner->helpEvent(e, v, o, i);
    }

private:
    QAbstractItemDelegate *m_inner;
};

// Mục 75: bọc delegate của một cột trong danh sách Find Action. Cột tên (vẽ
// trước) tô ô cả dòng; mọi cột vẽ như dòng thường.
class CommandBarRow : public QAbstractItemDelegate
{
public:
    CommandBarRow(QAbstractItemDelegate *inner, QTreeView *view)
        : QAbstractItemDelegate(view)
        , m_inner(inner)
        , m_view(view)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return m_inner->sizeHint(keycapOption(option, index), index);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() == 0 && (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)))
            paintPopupItem(p, QRect(0, option.rect.y(), m_view->viewport()->width(), option.rect.height()),
                           option.state & QStyle::State_Selected);
        QStyleOptionViewItem o = keycapOption(option, index);
        o.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver | QStyle::State_HasFocus);
        p->save();
        p->setFont(o.font);  // KDE đo phím bằng o.fontMetrics nhưng viết bằng font của painter
        m_inner->paint(p, o, index);
        p->restore();
    }

    bool editorEvent(QEvent *e, QAbstractItemModel *m, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return m_inner->editorEvent(e, m, o, i);
    }

    bool helpEvent(QHelpEvent *e, QAbstractItemView *v, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return m_inner->helpEvent(e, v, o, i);
    }

private:
    // Cột phím tắt (ShortcutStyleDelegate, kcommandbar.cpp 6.30): mặt phím
    // màu cửa sổ + bóng màu shadow lệch 1px, cao bằng một dòng chữ → KEYCAP,
    // không bóng (lộ qua mặt phím trong suốt), chữ nhỏ hơn như VS Code (phím
    // 11px trên dòng 13px) — mặt phím cao gần hết dòng thì nặng.
    static QStyleOptionViewItem keycapOption(const QStyleOptionViewItem &option, const QModelIndex &index)
    {
        QStyleOptionViewItem o(option);
        if (index.column() == 0)
            return o;
        o.palette.setColor(QPalette::Window, QColor(samhie::KEYCAP));
        o.palette.setColor(QPalette::Shadow, Qt::transparent);
        o.font.setPointSizeF(o.font.pointSizeF() * KEYCAP_FONT_SCALE);
        o.fontMetrics = QFontMetrics(o.font);
        return o;
    }

    static constexpr qreal KEYCAP_FONT_SCALE = 0.85;
    QPointer<QAbstractItemDelegate> m_inner;
    QTreeView *m_view;
};

// Mục 75: dựng thẻ Find Action. KCommandBar được polish lúc hiện lần đầu, khi
// ô gõ (focusProxy) và danh sách đã dựng xong trong hàm tạo — chưa có thì để
// lần polish sau (không đánh dấu).
void setupCommandBar(QFrame *bar)
{
    auto *input = qobject_cast<QLineEdit *>(bar->focusProxy());
    auto *list = bar->findChild<QTreeView *>(QString(), Qt::FindDirectChildrenOnly);
    if (!input || !list || !bar->layout() || bar->property("samhiePopupPanel").toBool())
        return;
    bar->setProperty("samhiePopupPanel", true);  // PE_Frame vẽ thẻ (mục 69)
    // Bóng đổ KDE (QGraphicsDropShadowEffect màu Dark, mờ 48) thành vệt đen
    // quanh thẻ (máy ảo user 2026-10-15) — Quick Open không có bóng
    bar->setGraphicsEffect(nullptr);
    bar->layout()->setContentsMargins(POPUP_CARD_PAD, POPUP_CARD_PAD, POPUP_CARD_PAD, POPUP_CARD_PAD);
    bar->layout()->setSpacing(POPUP_CARD_GAP);
    input->setFrame(true);
    list->setFrameShape(QFrame::NoFrame);
    list->viewport()->setAutoFillBackground(false);
    bool wrapped = false;
    for (int c = 0; list->model() && c < list->model()->columnCount(); ++c) {
        if (QAbstractItemDelegate *inner = list->itemDelegateForColumn(c)) {
            list->setItemDelegateForColumn(c, new CommandBarRow(inner, list));
            wrapped = true;
        }
    }
    // HUDDialog (mục 79): một cột, delegate chung gọi CE_ItemViewItem → mục 69
    if (!wrapped)
        list->setProperty("samhiePopupList", true);
}
