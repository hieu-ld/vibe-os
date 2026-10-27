// Dolphin: hàng màu + ký hiệu thư mục (mục 47), hộp Create Folder (48).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

// Mục 47: ký hiệu của menu Symbol — tên (theme/icons/hugeicons/symbol-<tên>,
// tools/build-folder-icons.py SYMBOLS) → chữ, theo bảng chữ cái của chữ.
const QList<QPair<QString, QString>> FOLDER_SYMBOLS = {
    {QStringLiteral("cloud"), QStringLiteral("Cloud")},
    {QStringLiteral("tar"), QStringLiteral("Compressed")},
    {QStringLiteral("design"), QStringLiteral("Design")},
    {QStringLiteral("development"), QStringLiteral("Development")},
    {QStringLiteral("documents"), QStringLiteral("Documents")},
    {QStringLiteral("download"), QStringLiteral("Downloads")},
    {QStringLiteral("important"), QStringLiteral("Important")},
    {QStringLiteral("library"), QStringLiteral("Library")},
    {QStringLiteral("mail"), QStringLiteral("Mail")},
    {QStringLiteral("music"), QStringLiteral("Music")},
    {QStringLiteral("pictures"), QStringLiteral("Pictures")},
    {QStringLiteral("print"), QStringLiteral("Print")},
    {QStringLiteral("temp"), QStringLiteral("Temporary")},
    {QStringLiteral("videos"), QStringLiteral("Videos")},
};
constexpr int SYMBOL_MENU_ICON = 16;  // icon trong menu có icon (samhie-symbol-* căn lưới 16)
constexpr int SYMBOL_MENU_GAP = 8;    // icon → chữ

// Menu có icon dù Samhie tắt icon menu (Kvantum iconless_menu, kdeglobals
// ShowIconsInMenuItems=false — QMenu không đưa icon vào option nữa): chữ của
// dòng có icon lùi bằng dấu cách, drawControl (CE_MenuItem) vẽ icon của
// chính QAction vào chỗ đó. Dòng không icon giữ nguyên chữ.
void enableMenuIcons(QMenu *menu)
{
    menu->setProperty("_samhie_menu_icons", true);
    const int space = qMax(1, menu->fontMetrics().horizontalAdvance(QLatin1Char(' ')));
    const QString indent(int(std::ceil(double(SYMBOL_MENU_ICON + SYMBOL_MENU_GAP) / space)), QLatin1Char(' '));
    for (QAction *a : menu->actions()) {
        if (a->isSeparator() || a->icon().isNull() || a->property("_samhie_indented").toBool())
            continue;
        a->setProperty("_samhie_indented", true);
        a->setText(indent + a->text());
    }
}
// Màu bỏ khỏi hàng (user 2026-10-08: còn Default, Red, Yellow, Teal + Blue —
// Blue thay Grey 2026-10-20).
const QSet<QString> FOLDER_COLORS_DROPPED = {QStringLiteral("orange"), QStringLiteral("green")};

// Mục 47: khoá trong [Desktop Entry] của <thư mục>/.directory.
const QString FOLDER_SYMBOL_KEY = QStringLiteral("X-Samhie-FolderSymbol");
const QString FOLDER_COLOR_KEY = QStringLiteral("X-Samhie-FolderColor");
const QStringList FOLDER_COLORS = {
    QStringLiteral("red"), QStringLiteral("yellow"), QStringLiteral("orange"), QStringLiteral("green"),
    QStringLiteral("cyan"), QStringLiteral("blue"), QStringLiteral("violet"), QStringLiteral("brown"),
    QStringLiteral("grey"),
};

// Ký hiệu KIO tự gán cho thư mục XDG khi .directory không có Icon
// (KFileItem, KIO 6.30): Music → music…; Home thì không tính.
QString naturalFolderSymbol(const QString &dir)
{
    static const QList<QPair<QStandardPaths::StandardLocation, QString>> xdg = {
        {QStandardPaths::MusicLocation, QStringLiteral("music")},
        {QStandardPaths::PicturesLocation, QStringLiteral("pictures")},
        {QStandardPaths::MoviesLocation, QStringLiteral("videos")},
        {QStandardPaths::DocumentsLocation, QStringLiteral("documents")},
        {QStandardPaths::DownloadLocation, QStringLiteral("download")},
    };
    const QString path = QFileInfo(dir).canonicalFilePath();
    for (const auto &[location, symbol] : xdg) {
        const QString x = QStandardPaths::writableLocation(location);
        if (!x.isEmpty() && QFileInfo(x).canonicalFilePath() == path && path != QDir::homePath())
            return symbol;
    }
    return QString();
}

// Khoá → giá trị trong [Desktop Entry] (đọc tay: QSettings đổi tên nhóm có
// dấu cách, KConfig thì style không link).
QHash<QString, QString> folderDesktopEntry(const QString &dir)
{
    QHash<QString, QString> keys;
    QFile file(dir + QStringLiteral("/.directory"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return keys;
    bool inGroup = false;
    for (const QString &line : QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'))) {
        if (line.startsWith(QLatin1Char('[')))
            inGroup = line.trimmed() == QLatin1String("[Desktop Entry]");
        else if (const int eq = line.indexOf(QLatin1Char('=')); inGroup && eq > 0)
            keys.insert(line.left(eq).trimmed(), line.mid(eq + 1).trimmed());
    }
    return keys;
}

// (ký hiệu, màu) hiện tại của thư mục; "" = không ký hiệu / màu default.
// Không có khoá của Samhie (thư mục tô màu trước 2026-10-08) thì đoán từ Icon.
QPair<QString, QString> folderLook(const QString &dir)
{
    const QHash<QString, QString> keys = folderDesktopEntry(dir);
    QString symbol = naturalFolderSymbol(dir), color;
    if (keys.contains(FOLDER_SYMBOL_KEY) || keys.contains(FOLDER_COLOR_KEY)) {
        if (keys.contains(FOLDER_SYMBOL_KEY))
            symbol = keys.value(FOLDER_SYMBOL_KEY);
        color = keys.value(FOLDER_COLOR_KEY);
    } else if (keys.contains(QStringLiteral("Icon"))) {
        QString icon = keys.value(QStringLiteral("Icon"));
        symbol.clear();
        if (icon.startsWith(QLatin1String("folder-"))) {
            icon = icon.mid(7);
            for (const QString &c : FOLDER_COLORS) {
                if (icon == c || icon.endsWith(QLatin1Char('-') + c)) {
                    color = c;
                    icon.chop(icon == c ? icon.size() : c.size() + 1);
                    break;
                }
            }
            symbol = icon;
        }
    }
    if (symbol == QLatin1String("none"))
        symbol.clear();
    return {symbol, color};
}

// Thư mục vừa được chuột phải trong vùng file Dolphin, lấy qua giao diện trợ
// năng của Dolphin (cài sẵn — QAccessible::installFactory trong
// kitemlistview.cpp 26.08.1): mục dưới chuột (KItemListViewAccessible::
// childAt nhận toạ độ SCENE, không phải toạ độ màn hình) cho tên, mô tả của
// view kết thúc bằng "… location <đường dẫn thư mục đang xem>" (giao diện
// tiếng Anh). Gọi lúc menu polish — chuột còn ở chỗ vừa bấm; mục phải đang
// được chọn (chuột phải chọn mục), không thì coi như không biết.
QString folderUnderCursor()
{
    const QPoint global = QCursor::pos();
    QGraphicsView *gv = nullptr;
    QWidget *container = nullptr;
    for (QWidget *p = QApplication::widgetAt(global); p && !container; p = p->parentWidget()) {
        if (!gv)
            gv = qobject_cast<QGraphicsView *>(p);
        if (p->inherits("KItemListContainer"))
            container = p;
    }
    QAccessibleInterface *c = container ? QAccessible::queryAccessibleInterface(container) : nullptr;
    QAccessibleInterface *view = c && gv && c->childCount() > 0 ? c->child(0) : nullptr;
    if (!view)
        return QString();
    const QPointF scene = gv->mapToScene(gv->viewport()->mapFromGlobal(global));
    QAccessibleInterface *cell = view->childAt(int(scene.x()), int(scene.y()));
    const QString description = view->text(QAccessible::Description);
    const int at = description.lastIndexOf(QLatin1String("location "));
    if (!cell || !cell->state().selected || at < 0)
        return QString();
    const QUrl dir = QUrl::fromUserInput(description.mid(at + 9));
    if (!dir.isLocalFile())
        return QString();
    const QString path = dir.toLocalFile() + QLatin1Char('/') + cell->text(QAccessible::Name);
    return QFileInfo(path).isDir() ? path : QString();
}

// Mục 47: hàng màu thư mục trong menu chuột phải của Dolphin
// (setfoldericonitemaction.cpp 26.08.1 — 5 nút màu + nút Other mở menu gồm
// Default, 4 màu nữa và các ký hiệu). User 2026-10-08:
//   [Default] [Red] [Yellow] [Teal] [Blue]  [Symbol ▾]
// Symbol mở menu No Symbol + ký hiệu; màu và ký hiệu đổi độc lập (đổi màu giữ
// ký hiệu…). Plugin chỉ ghi Icon=<tên icon của nút> vào .directory rồi báo
// KDirNotify.FilesChanged(url thư mục); đường dẫn thư mục nằm trong biến
// riêng của plugin → mỗi cú bấm: ghi nhớ ý định (armFolderChange), để plugin
// làm việc của nó (nút ta thêm bấm hộ hành động Default của plugin — chỉ
// cần plugin báo url), rồi FolderChangeWatcher sửa .directory theo ý định.
// Nút tự thêm/gỡ ở đây; bấm hành động của plugin (trigger) là menu tự đóng
// như khi bấm nút của nó (plugin nối triggered → QWidgetAction).
void setupFolderColorRow(QMenu *menu)
{
    for (QAction *a : menu->actions()) {
        auto *wa = qobject_cast<QWidgetAction *>(a);
        if (!wa)
            continue;
        for (QWidget *row : static_cast<WidgetActionAccess *>(wa)->createdWidgets()) {
            const QList<QPushButton *> buttons = row->findChildren<QPushButton *>(Qt::FindDirectChildrenOnly);
            auto other = std::find_if(buttons.begin(), buttons.end(), [](QPushButton *b) { return b->menu(); });
            auto *layout = qobject_cast<QBoxLayout *>(row->layout());
            if (other == buttons.end() || !layout)
                continue;
            QMenu *sub = (*other)->menu();
            QPointer<QAction> carrier;  // hành động Default của plugin
            QAction *blue = nullptr;
            for (QAction *s : sub->actions()) {
                if (s->icon().name() == QLatin1String("inode-directory"))
                    carrier = s;
                else if (s->icon().name() == QLatin1String("folder-blue"))
                    blue = s;
            }
            if (!carrier)
                return;  // plugin đổi — để nguyên hàng của KDE
            // Đánh dấu màu + ký hiệu đang dùng (user 2026-10-08) — biết được
            // thư mục thì mới đánh dấu; plugin tự tích chỉ khi trùng tên icon.
            // Trạng thái này cũng là "trước khi bấm" cho FolderChangeWatcher:
            // plugin ghi đè Icon trước khi ta đọc được.
            const QString dir = folderUnderCursor();
            const auto [currentSymbol, currentColor] = dir.isEmpty() ? QPair<QString, QString>() : folderLook(dir);
            const bool known = !dir.isEmpty();
            watchFolderChanges(dir, currentSymbol, currentColor);
            for (QPushButton *b : buttons) {
                const QString name = b->icon().name();
                if (b == *other || !name.startsWith(QLatin1String("folder-")))
                    continue;
                const QString color = name.mid(7);
                if (FOLDER_COLORS_DROPPED.contains(color)) {
                    b->hide();
                    continue;
                }
                b->setChecked(known && color == currentColor);
                QObject::connect(b, &QPushButton::clicked, b, [color] { armFolderChange(FolderChange::Color, color); });
            }
            auto addButton = [&](const QString &icon, const QString &color, const QString &tip, int at) {
                auto *b = new QPushButton(QIcon::fromTheme(icon), QString(), row);
                b->setToolTip(tip);
                b->setCheckable(true);
                b->setChecked(known && color == currentColor);
                QObject::connect(b, &QPushButton::clicked, b, [color, carrier] {
                    armFolderChange(FolderChange::Color, color);
                    if (carrier)
                        carrier->trigger();
                });
                layout->insertWidget(at, b);
            };
            addButton(QStringLiteral("inode-directory"), QString(), QStringLiteral("Set folder color to Default"), 0);
            if (blue)
                addButton(QStringLiteral("folder-blue"), QStringLiteral("blue"), QStringLiteral("Set folder color to Blue"),
                          layout->indexOf(*other));
            (*other)->setText(QStringLiteral("Symbol"));
            (*other)->setToolTip(QStringLiteral("Folder symbol"));
            for (QAction *s : sub->actions())
                sub->removeAction(s);  // hành động vẫn còn (con của menu), Default làm cầu
            // Có icon (enableMenuIcons); ô tròn (radio) đánh dấu ký hiệu đang dùng.
            auto *group = new QActionGroup(sub);
            auto addSymbol = [&](const QString &symbol, const QString &text) {
                QAction *s = symbol.isEmpty() ? sub->addAction(text)
                                              : sub->addAction(QIcon::fromTheme(QStringLiteral("samhie-symbol-") + symbol), text);
                s->setCheckable(true);
                s->setChecked(known && symbol == currentSymbol);
                s->setData(symbol);
                group->addAction(s);
                QObject::connect(s, &QAction::triggered, sub, [symbol, carrier] {
                    armFolderChange(FolderChange::Symbol, symbol);
                    if (carrier)
                        carrier->trigger();
                });
            };
            addSymbol(QString(), QStringLiteral("No Symbol"));
            sub->addSeparator();
            for (const auto &[symbol, text] : FOLDER_SYMBOLS)
                addSymbol(symbol, text);
            enableMenuIcons(sub);
            return;
        }
    }
}

// Tên icon cho (ký hiệu, màu): "" = không ký hiệu / màu default.
QString folderIconName(const QString &symbol, const QString &color)
{
    if (symbol.isEmpty() && color.isEmpty())
        return QStringLiteral("inode-directory");  // KIO: icon mặc định — không ghi .directory
    QString name = QStringLiteral("folder");
    if (!symbol.isEmpty())
        name += QLatin1Char('-') + symbol;
    if (!color.isEmpty())
        name += QLatin1Char('-') + color;
    return name;
}

// Mục 48: hộp thoại Create Folder (KNewFileMenu, knewfilemenu.cpp KIO 6.30)
// chọn màu/ký hiệu như hàng màu của menu chuột phải (mục 47, user
// 2026-10-08): lưới 20 ô của KIO (9 màu + 10 ký hiệu cũ, có cả bookmark/games
// Samhie đã bỏ) và nút Choose Icon… thay bằng một hàng 5 màu + ô ký hiệu
// (No Symbol + FOLDER_SYMBOLS). KIO lấy tên icon từ thuộc tính iconName của
// nhãn xem trước (iconLabel) lúc bấm OK rồi ghi .directory → chỉ cần cập
// nhật nhãn đó. Thư mục mới không có khoá X-Samhie-*: đổi sau bằng menu thì
// folderLook đoán lại từ Icon.
void setupNewFolderDialog(QWidget *dialog)
{
    auto *preview = dialog->findChild<QLabel *>(QStringLiteral("iconLabel"));
    auto *choose = dialog->findChild<QPushButton *>(QStringLiteral("chooseIconButton"));
    auto *grid = dialog->findChild<QGridLayout *>(QStringLiteral("folderIconGrid"));
    if (!preview || !choose || !grid || dialog->property("_samhie_folder_picker").toBool())
        return;
    dialog->setProperty("_samhie_folder_picker", true);
    while (QLayoutItem *item = grid->takeAt(0)) {
        if (item->widget())
            item->widget()->hide();
        delete item;
    }
    choose->hide();
    if (QWidget *box = dialog->findChild<QWidget *>(QStringLiteral("chooseIconBox")))
        box->setProperty("title", QStringLiteral("Choose color or symbol"));  // KCollapsibleGroupBox
    constexpr int PICKER_CELL = 40;

    auto state = std::make_shared<QPair<QString, QString>>();  // (ký hiệu, màu)
    auto update = [preview, state] {
        const QString name = folderIconName(state->first, state->second);
        preview->setProperty("iconName", name);
        preview->setPixmap(QIcon::fromTheme(name).pixmap(QSize(64, 64), preview->devicePixelRatioF()));
    };
    auto addButton = [&](const QString &icon, int iconSize, const QString &tip, int row, int column,
                         QButtonGroup *buttons, std::function<void()> pick) {
        auto *b = new QToolButton(dialog);
        b->setIcon(QIcon::fromTheme(icon));
        b->setIconSize(QSize(iconSize, iconSize));
        b->setToolTip(tip);
        b->setCheckable(true);
        b->setChecked(buttons->buttons().isEmpty());  // ô đầu nhóm: Default / No Symbol
        b->setFixedSize(PICKER_CELL, PICKER_CELL);    // ô vuông như nhau → cột màu thẳng cột ký hiệu
        buttons->addButton(b);
        QObject::connect(b, &QToolButton::clicked, b, [pick, update] {
            pick();
            update();
        });
        grid->addWidget(b, row, column);
    };
    auto *colors = new QButtonGroup(dialog);
    const QList<QPair<QString, QString>> colorList = {
        {QString(), QStringLiteral("Default")}, {QStringLiteral("red"), QStringLiteral("Red")},
        {QStringLiteral("yellow"), QStringLiteral("Yellow")}, {QStringLiteral("cyan"), QStringLiteral("Teal")},
        {QStringLiteral("blue"), QStringLiteral("Blue")},
    };
    for (int i = 0; i < colorList.size(); ++i) {
        const QString color = colorList[i].first;
        addButton(color.isEmpty() ? QStringLiteral("inode-directory") : QStringLiteral("folder-") + color, 32,
                  colorList[i].second, 0, i, colors, [state, color] { state->second = color; });
    }
    auto *symbols = new QButtonGroup(dialog);
    constexpr int PER_ROW = 8;
    addButton(QStringLiteral("dialog-cancel"), 24, QStringLiteral("No Symbol"), 1, 0, symbols,
              [state] { state->first.clear(); });
    for (int i = 0; i < FOLDER_SYMBOLS.size(); ++i) {
        const QString symbol = FOLDER_SYMBOLS[i].first;
        const int n = i + 1;
        addButton(QStringLiteral("samhie-symbol-") + symbol, 24, FOLDER_SYMBOLS[i].second, 1 + n / PER_ROW,
                  n % PER_ROW, symbols, [state, symbol] { state->first = symbol; });
    }
    update();
}
