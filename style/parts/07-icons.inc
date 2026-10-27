// Icon: icon nét (mục 45), vạch ngăn sidebar (46), hộp chọn icon (43).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// Thư mục con của bộ icon đang dùng, đọc từ index.theme: (thư mục, Context=,
// có phải Type=Scalable). Bộ icon không đổi trong lúc app chạy → đọc một lần.
struct IconThemeDir {
    QString path;  // đường dẫn đầy đủ, có "/" ở cuối
    QString context;
    bool scalable;
};

const QList<IconThemeDir> &iconThemeDirs()
{
    static const QList<IconThemeDir> dirs = [] {
        QList<IconThemeDir> out;
        const QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                    QStringLiteral("icons/%1/index.theme").arg(QIcon::themeName()));
        if (file.isEmpty())
            return out;
        const QString root = QFileInfo(file).absolutePath() + QLatin1Char('/');
        QSettings theme(file, QSettings::IniFormat);
        for (const QString &dir : theme.value(QStringLiteral("Icon Theme/Directories")).toStringList()) {
            out.append({root + dir + QLatin1Char('/'), theme.value(dir + QStringLiteral("/Context")).toString(),
                        theme.value(dir + QStringLiteral("/Type")).toString() == QLatin1String("Scalable")});
        }
        return out;
    }();
    return dirs;
}

// Mục 45: icon NÉT = SVG trong thư mục Scalable, tô theo màu chữ (class
// ColorScheme-Text — tools/build-icons.py). Thư mục màu
// (tools/build-folder-icons.py), icon app thì không; bản 16px chỉ có ký hiệu
// (places/16) cũng không — chọn nó thì cỡ 20 của sidebar lại ra thư mục màu.
bool isStrokeIconFile(const QString &path)
{
    static QHash<QString, bool> cache;
    auto it = cache.constFind(path);
    if (it != cache.constEnd())
        return *it;
    bool stroke = false;
    if (path.endsWith(QLatin1String(".svg"))) {
        for (const IconThemeDir &d : iconThemeDirs()) {
            if (d.scalable && path.startsWith(d.path) && !path.mid(d.path.size()).contains(QLatin1Char('/'))) {
                QFile f(path);
                stroke = f.open(QIODevice::ReadOnly) && f.readAll().contains("ColorScheme-Text");
                break;
            }
        }
    }
    cache.insert(path, stroke);
    return stroke;
}

bool isStrokeIconName(const QString &name)
{
    for (const IconThemeDir &d : iconThemeDirs()) {
        const QString path = d.path + name + QStringLiteral(".svg");
        if (d.scalable && QFile::exists(path))
            return isStrokeIconFile(path);
    }
    return false;
}

// Icon nét thay cho icon thư mục MÀU của một mục sidebar (thư mục cục bộ).
// Home và Download có bản nét riêng như mục mặc định (user-places.xbel.in).
QString strokePlaceIcon(const QString &icon)
{
    if (icon == QLatin1String("user-home"))
        return QStringLiteral("samhie-user-home");
    if (icon == QLatin1String("folder-download"))
        return QStringLiteral("samhie-folder-download");
    return QStringLiteral("samhie-folder");
}

// Mục 46: vạch ngăn mục mặc định của sidebar (Home … Recycle Bin) với thư mục
// người dùng ghim thêm, trên dưới thoáng bằng khe giữa hai mục (user
// 2026-10-08). Mục ghim mới luôn nằm SAU Recycle Bin trong nhóm Places (KIO
// thêm vào cuối; mục mặc định của Samhie kết thúc bằng trash:/ —
// user-places.xbel.in). Khe thêm chia đôi: dòng Recycle Bin cao thêm ở dưới,
// dòng ghim đầu tiên cao thêm ở trên, vạch ở ranh giới — rê chuột/bấm vào
// nửa khe nào thì trúng dòng gần nó, như phần đệm của các dòng khác.
// Qt hỏi cỡ dòng KHÔNG kèm số dòng (QListViewPrivate::itemSize, qtbase
// 6.11) nên style không nới được một dòng → gắn delegate riêng cho đúng hai
// dòng đó (setItemDelegateForRow). Delegate chung của KIO giữ nguyên — KIO ép
// kiểu nó ở nhiều chỗ (xem mục 2) — và vẫn vẽ nội dung dòng.
class PinnedGapDelegate : public QAbstractItemDelegate
{
public:
    explicit PinnedGapDelegate(QListView *view)
        : QAbstractItemDelegate(view)
        , m_view(view)
    {
        setObjectName(QStringLiteral("samhie-pinned-gap"));
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return base()->sizeHint(option, index) + QSize(0, HALF_GAP);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        constexpr int LATERAL_MARGIN = 4;  // KIO s_lateralMargin, kfileplacesview.cpp 6.30
        const bool above = index.row() == m_above;  // dòng Recycle Bin: khe ở dưới
        QStyleOptionViewItem o(option);
        if (above)
            o.rect.setBottom(o.rect.bottom() - HALF_GAP);
        else
            o.rect.setTop(o.rect.top() + HALF_GAP);
        base()->paint(p, o, index);
        if (!above)
            return;
        // Lề trái thẳng icon; phải bớt 1: khung sidebar 1px nằm ngoài ô bên
        // trái nhưng trong ô bên phải → hai đầu cùng cách viền 12px (đo
        // container 2026-10-08; viewport đã lùi SIDEBAR_INDENT). Hàng điểm
        // ảnh cuối của dòng: rê chuột dòng dưới không xoá vạch.
        const int y = option.rect.bottom();
        p->save();
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(samhie::FRAME_LINE));
        p->drawRect(QRect(QPoint(o.rect.left() + LATERAL_MARGIN, y),
                          QPoint(o.rect.right() - LATERAL_MARGIN - SIDEBAR_INDENT + 1, y)));
        p->restore();
    }

    bool editorEvent(QEvent *e, QAbstractItemModel *m, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return base()->editorEvent(e, m, o, i);
    }

    bool helpEvent(QHelpEvent *e, QAbstractItemView *v, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return base()->helpEvent(e, v, o, i);
    }

    // Tìm dòng Recycle Bin có ngay sau nó một dòng hiện cùng nhóm, gắn
    // delegate cho hai dòng đó; gỡ khỏi dòng cũ (dòng xê dịch khi thêm/bớt/
    // kéo mục, KIO ẩn/hiện dòng).
    void update()
    {
        constexpr int URL_ROLE = 0x069CD12B, GROUP_ROLE = 0x0a5b64ee;  // kfileplacesmodel.h
        const QAbstractItemModel *model = m_view->model();
        if (!model)
            return;
        int above = -1, below = -1;
        for (int r = 0; r < model->rowCount() && above < 0; ++r) {
            const QModelIndex i = model->index(r, 0);
            if (m_view->isRowHidden(r) || i.data(URL_ROLE).toUrl().scheme() != QLatin1String("trash"))
                continue;
            int next = r + 1;
            while (next < model->rowCount() && m_view->isRowHidden(next))
                ++next;
            if (next < model->rowCount() && model->index(next, 0).data(GROUP_ROLE) == i.data(GROUP_ROLE)) {
                above = r;
                below = next;
            }
        }
        if (above == m_above && below == m_below)
            return;
        for (int r : {m_above, m_below}) {
            if (r >= 0 && m_view->itemDelegateForRow(r) == this)
                m_view->setItemDelegateForRow(r, nullptr);
        }
        m_above = above;
        m_below = below;
        for (int r : {above, below}) {
            if (r >= 0)
                m_view->setItemDelegateForRow(r, this);
        }
    }

private:
    // Mỗi dòng thêm 12px → vạch cách icon trên/dưới 23–24px trống, bằng khe
    // giữa hai mục (dòng 41, icon 20 — hình icon nét không kín khung; đo
    // container 2026-10-08).
    static constexpr int HALF_GAP = 12;

    QAbstractItemDelegate *base() const { return m_view->itemDelegate(); }

    QListView *m_view;
    int m_above = -1;
    int m_below = -1;
};

// Mục 43: hộp chọn icon (KIconDialog) chỉ liệt kê icon của bộ đang dùng
// (Samhie, user 2026-10-07), không lấy Breeze/hicolor mà bộ đó thừa kế.
// KIconDialogModel (KF 6.30) giữ đường dẫn file ở Qt::UserRole; KIconLoader
// đã bỏ tên trùng, giữ bản của bộ đứng trước → bản Samhie thắng bản Breeze.
// Mục 45: chọn icon cho mục sidebar thì chỉ icon nét.
class IconThemeFilter : public QSortFilterProxyModel
{
public:
    IconThemeFilter(bool strokeOnly, QObject *parent)
        : QSortFilterProxyModel(parent)
        , m_dir(QStringLiteral("/icons/%1/").arg(QIcon::themeName()))
        , m_strokeOnly(strokeOnly)
    {
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        const QString path = sourceModel()->index(row, 0, parent).data(Qt::UserRole).toString();
        return m_strokeOnly ? isStrokeIconFile(path) : path.contains(m_dir);
    }

private:
    const QString m_dir;
    const bool m_strokeOnly;
};

// Nhóm (Context=) có icon trong bộ đang dùng (strokeOnly: có icon nét), theo
// giá trị KIconLoader::Context mà ô chọn nhóm của KIconDialog lưu ở dữ liệu
// từng dòng (kiconloader.h KF 6.30: Any 0, Action 1, Application 2, Device 3,
// MimeType 4, Animation 5, Category 6, Emblem 7, Emote 8, International 9,
// Place 10, StatusIcon 11 — cố định trong ABI KF6; style không link
// KIconThemes nên ghi số). Bộ thêm nhóm mới thì ô chọn tự có.
QSet<int> iconThemeContexts(bool strokeOnly)
{
    static const QHash<QString, int> ids = {
        {QStringLiteral("Actions"), 1}, {QStringLiteral("Applications"), 2},
        {QStringLiteral("Devices"), 3}, {QStringLiteral("MimeTypes"), 4},
        {QStringLiteral("Categories"), 6}, {QStringLiteral("Emblems"), 7},
        {QStringLiteral("Emotes"), 8}, {QStringLiteral("Places"), 10},
        {QStringLiteral("Status"), 11},
    };
    QSet<int> found = {0};  // All
    for (const IconThemeDir &d : iconThemeDirs()) {
        if (!ids.contains(d.context))
            continue;
        bool any = !strokeOnly;
        if (strokeOnly && d.scalable) {
            const QStringList files = QDir(d.path).entryList({QStringLiteral("*.svg")}, QDir::Files);
            any = std::any_of(files.begin(), files.end(),
                              [&d](const QString &f) { return isStrokeIconFile(d.path + f); });
        }
        if (any)
            found.insert(ids.value(d.context));
    }
    return found;
}

// Mục 45: mục sidebar là thư mục cục bộ mang icon thư mục màu (Add to Places
// của Dolphin lấy icon của chính thư mục, kéo thả lấy KIO::iconNameForUrl)
// → đổi sang icon nét; icon thư mục trong vùng file giữ nguyên. Đổi qua
// KFilePlacesModel::editPlace (lưu vào ~/.local/share/user-places.xbel, mọi
// app thấy). Hàm đó không gọi được qua meta-object; style không link KIO
// (style nạp vào MỌI app Qt) → tìm hàm trong thư viện KIO mà app ĐÃ nạp (có
// KFilePlacesModel nghĩa là đã nạp). Hàm không ảo, gọi với `this` là tham
// số đầu (Itanium C++ ABI). Không tìm thấy (KIO đổi ABI) thì thôi, không đổi.
// appName rỗng: mục thêm kiểu này không gắn app; Add Entry thì hộp thoại đã
// đổi sẵn icon (polish) nên không tới đây.
void strokePlaceIcons(QAbstractItemModel *model, int first, int last)
{
    using EditPlace = void (*)(QAbstractItemModel *, const QModelIndex &, const QString &, const QUrl &,
                               const QString &, const QString &);
    static const auto editPlace = reinterpret_cast<EditPlace>(
        dlsym(RTLD_DEFAULT, "_ZN16KFilePlacesModel9editPlaceERK11QModelIndexRK7QStringRK4QUrlS5_S5_"));
    constexpr int URL_ROLE = 0x069CD12B, HIDDEN_ROLE = 0x0741CAAC, ICON_NAME_ROLE = 0x00a45c00;  // kfileplacesmodel.h
    if (!editPlace || !QIcon::hasThemeIcon(QStringLiteral("samhie-folder")))
        return;
    for (int row = first; row <= last; ++row) {
        const QModelIndex index = model->index(row, 0);
        bool device = false;
        QMetaObject::invokeMethod(model, "isDevice", Qt::DirectConnection, Q_RETURN_ARG(bool, device),
                                  Q_ARG(QModelIndex, index));
        const QString icon = index.data(ICON_NAME_ROLE).toString();
        const QUrl url = index.data(URL_ROLE).toUrl();
        if (device || index.data(HIDDEN_ROLE).toBool() || !url.isLocalFile() || icon.isEmpty()
            || icon.startsWith(QLatin1Char('/')) || isStrokeIconName(icon))
            continue;
        editPlace(model, index, index.data(Qt::DisplayRole).toString(), url, strokePlaceIcon(icon), QString());
    }
}
