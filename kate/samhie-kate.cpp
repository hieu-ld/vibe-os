// Plugin Kate "samhiekate" — Kate kiểu VS Code của Samhie (user 2026-10-10).
// Bật cho từng tài khoản ở user-setup bước 0h (Kate chỉ tự bật plugin trong
// danh sách viết cứng của nó).
//
// 1. Terminal làm TAB trong vùng editor, cạnh các file đang mở, như VS Code
//    (user: mở IDE việc đầu tiên là mở terminal gõ lệnh chạy Claude CLI).
//    Plugin terminal có sẵn của Kate (katekonsoleplugin) chỉ đặt terminal ở
//    khung dưới. Kate 26.08 cho plugin đặt một widget bất kỳ thành tab trong
//    vùng editor: KTextEditor::MainWindow::addWidget (Compiler Explorer, kết
//    quả Search dùng đường này — đọc mã Kate 26.08.1). Tab tự đổi tên theo
//    windowTitle của widget, và Kate gọi Q_INVOKABLE shouldClose() trước khi
//    đóng tab / thoát.
//    Terminal là KonsolePart (kf6/parts/konsolepart) — cùng Konsole của Samhie:
//    không có DefaultProfile trong katerc thì Konsole lấy DefaultProfile của
//    konsolerc (ProfileManager.cpp) → profile Samhie, tên tab từ samhie-title.sh.
//      Ctrl+`        mở/về terminal; đang ở terminal thì quay lại file trước đó
//      Ctrl+Shift+`  terminal mới
//    Shell thoát (gõ exit) thì tab tự đóng.
//
// 2. Thanh menu gọn như VS Code (user chọn): ẩn Projects, LSP Client,
//    Sessions, Tools. Chỉ ẩn mục trên thanh menu — hành động vẫn còn, phím tắt
//    vẫn chạy, tìm được bằng Ctrl+Alt+I. Làm ở đây chứ không vá file menu: Tools
//    gom từ 5 nguồn (kateui.rc, katepart5ui.rc dùng chung với KWrite, 3 plugin)
//    — xoá ở một nơi thì nơi khác lại dựng menu Tools mới.
//
// 3. Ẩn ô "Filter…" dưới cây thư mục Projects (VS Code không có; tìm file
//    dùng Ctrl+Alt+O hoặc Search). Ô này là KLineEdit con trực tiếp của
//    KateProjectView (kateprojectview.cpp), mỗi dự án mở ra một cái.
//
// 4. Bố cục theo bản phác Figma của user (2026-10-10):
//    - Nút ☰ ở cuối hàng tab, sau quick open + split. Thanh menu ẩn hẳn bằng
//      khoá của chính Kate (Show Menu Bar=false, /etc/xdg/katerc); nút là hành
//      động "hamburger_menu" có sẵn của Kate (KHamburgerMenu) — Kate đặt nó
//      trên thanh hamburgerBar, thanh đó ẩn cùng mainToolBar. Mỗi view space
//      (khi chia đôi) có hàng tab riêng; nút chỉ nằm ở view space trên cùng
//      bên phải, góc cửa sổ như VS Code. Thanh hamburgerBar (giờ trống) ẩn.
//    - Đầu khung Projects: bỏ nút ✕ (đóng dự án), thêm « thu khung lại. Đóng
//      dự án (hiếm dùng) còn ở Ctrl+Alt+I › "Close Project" — menu Projects
//      đã ẩn ở mục 2.

#include <KActionCollection>
#include <KMessageBox>
#include <KParts/ReadOnlyPart>
#include <KPluginFactory>
#include <KStandardGuiItem>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>
#include <KXmlGuiWindow>
#include <kde_terminal_interface.h>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidgetAction>

namespace
{
const QString kTitle = QStringLiteral("Terminal");

KPluginFactory *konsoleFactory()
{
    static KPluginFactory *factory = KPluginFactory::loadFactory(KPluginMetaData(QStringLiteral("kf6/parts/konsolepart"))).plugin;
    return factory;
}

// Mục ẩn khỏi thanh menu: tên <Menu name> trong file .rc = objectName của
// QMenu (KXMLGUIBuilder); Sessions là một hành động (KActionMenu) đặt thẳng
// lên thanh menu, objectName = tên hành động.
bool isHiddenMenu(const QAction *a)
{
    static const QStringList menus{QStringLiteral("projects"), QStringLiteral("LSPClient Menubar"), QStringLiteral("tools")};
    // Sessions (KActionMenu) cũng có menu() — xét tên hành động trước
    return a->objectName() == QLatin1String("sessions") || (a->menu() && menus.contains(a->menu()->objectName()));
}

// Ô Filter… của KateProjectView. Lớp đó (và KateProjectViewTree) không có
// Q_OBJECT nên không nhận ra bằng tên lớp — nhận theo bố cục trong
// kateprojectview.cpp: layout đúng hai mục [cây, ô lọc], và KateProjectView
// nằm trong QStackedWidget của khung Projects. Ô lọc của panel Git có ba mục
// (hàng nút, cây, ô lọc) nên không dính.
bool isProjectFilter(QLineEdit *edit)
{
    QWidget *view = edit->parentWidget();
    QLayout *layout = view ? view->layout() : nullptr;
    return layout && layout->count() == 2 && qobject_cast<QTreeView *>(layout->itemAt(0)->widget())
        && layout->itemAt(1)->widget() == edit && qobject_cast<QStackedWidget *>(view->parentWidget());
}

// Hàng [ô chọn dự án, ⟳, ✕] đầu khung Projects (kateprojectpluginview.cpp):
// một QHBoxLayout trong layout của khung, ô chọn đứng đầu. Sau khi plugin sửa,
// hàng có 4 mục nên không khớp lần hai.
QHBoxLayout *projectHeader(QComboBox *combo)
{
    QWidget *toolView = combo->parentWidget();
    QLayout *outer = toolView ? toolView->layout() : nullptr;
    for (int i = 0; outer && i < outer->count(); ++i) {
        auto *row = qobject_cast<QHBoxLayout *>(outer->itemAt(i)->layout());
        if (row && row->count() == 3 && row->itemAt(0)->widget() == combo && qobject_cast<QToolButton *>(row->itemAt(1)->widget())
            && qobject_cast<QToolButton *>(row->itemAt(2)->widget())) {
            return row;
        }
    }
    return nullptr;
}

bool isViewSpace(const QObject *obj)
{
    return obj->inherits("KateViewSpace");
}

void trimMenuBar(QMenuBar *bar)
{
    for (QAction *a : bar->actions()) {
        if (a->isVisible() && isHiddenMenu(a)) {
            a->setVisible(false);
        }
    }
}

// Phím để lại cho Kate thay vì gửi xuống shell. Mọi phím khác thuộc về
// terminal (Ctrl+C ngắt lệnh, Ctrl+W xoá từ… — KonsolePart mặc định giành hết).
bool isKateKey(const QKeyEvent *e)
{
    const auto mods = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier | Qt::MetaModifier);
    const int key = e->key();
    if (key == Qt::Key_QuoteLeft || key == Qt::Key_AsciiTilde) { // ` và Shift+` (~)
        return mods == Qt::ControlModifier || mods == (Qt::ControlModifier | Qt::ShiftModifier);
    }
    // Đổi tab: Ctrl+PgUp/PgDn (Ctrl+Tab thì Kate vốn tự bắt)
    return mods == Qt::ControlModifier && (key == Qt::Key_PageUp || key == Qt::Key_PageDown);
}
}

class TerminalTab : public QWidget
{
    Q_OBJECT
public:
    TerminalTab(KParts::ReadOnlyPart *part, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_part(part)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        // KonsolePart tự có dải tab riêng — một tab Kate = một shell, ẩn dải đó
        if (auto *tabs = qobject_cast<QTabWidget *>(part->widget())) {
            tabs->setTabBarAutoHide(true);
        }
        layout->addWidget(part->widget());
        setFocusProxy(part->widget());
        setWindowTitle(kTitle);
        setWindowIcon(QIcon::fromTheme(QStringLiteral("utilities-terminal")));
        connect(part, &KParts::Part::setWindowCaption, this, [this](const QString &caption) {
            setWindowTitle(caption.isEmpty() ? kTitle : caption);
        });
    }

    KParts::ReadOnlyPart *part() const
    {
        return m_part;
    }

    // Kate hỏi trước khi đóng tab (và trước khi thoát): đang chạy chương trình
    // (vd Claude CLI) thì hỏi lại như Konsole/VS Code; chỉ có shell thì đóng luôn.
    Q_INVOKABLE bool shouldClose()
    {
        auto *term = qobject_cast<TerminalInterface *>(m_part.data());
        if (!term) {
            return true;
        }
        const int fg = term->foregroundProcessId();
        if (fg <= 0 || fg == term->terminalProcessId()) {
            return true;
        }
        return KMessageBox::warningContinueCancel(this,
                                                  QStringLiteral("“%1” is still running in this terminal. Close it anyway?").arg(term->foregroundProcessName()),
                                                  QStringLiteral("Close Terminal"),
                                                  KStandardGuiItem::close())
            == KMessageBox::Continue;
    }

private:
    QPointer<KParts::ReadOnlyPart> m_part;
};

class SamhieKateView : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    explicit SamhieKateView(KTextEditor::MainWindow *mw)
        : QObject(mw)
        , m_mw(mw)
    {
        setComponentName(QStringLiteral("samhiekate"), kTitle);
        setXMLFile(QStringLiteral("ui.rc"));

        QAction *a = actionCollection()->addAction(QStringLiteral("samhie_terminal_new"));
        a->setText(QStringLiteral("New Terminal"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("utilities-terminal")));
        actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_QuoteLeft));
        connect(a, &QAction::triggered, this, &SamhieKateView::newTerminal);

        a = actionCollection()->addAction(QStringLiteral("samhie_terminal_toggle"));
        a->setText(QStringLiteral("Switch Between Terminal and File"));
        actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft));
        connect(a, &QAction::triggered, this, &SamhieKateView::toggleTerminal);

        // Nhớ tab terminal dùng gần nhất (bấm chuột vào tab cũng tính)
        connect(qApp, &QApplication::focusChanged, this, [this](QWidget *, QWidget *now) {
            for (QWidget *w = now; w; w = w->parentWidget()) {
                if (auto *tab = qobject_cast<TerminalTab *>(w)) {
                    m_lastTerminal = tab;
                    return;
                }
            }
        });

        m_mw->guiFactory()->addClient(this);

        // Thanh menu: ẩn ngay, và ẩn lại mỗi khi KXmlGui thêm/sửa mục (plugin
        // nạp sau, đổi view thì phần editor gỡ ra lắp vào).
        if (auto *win = qobject_cast<QMainWindow *>(m_mw->window())) {
            m_menuBar = win->menuBar();
            if (auto *xmlWin = dynamic_cast<KXmlGuiWindow *>(win)) {
                m_showMenuBar = xmlWin->actionCollection()->action(QStringLiteral("options_show_menubar"));
            }
            m_menuBar->installEventFilter(this);
            trimMenuBar(m_menuBar);
        }
        // Ô Filter… của mỗi dự án: bắt lúc nó sắp hiện
        qApp->installEventFilter(this);
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::ActionAdded:
        case QEvent::ActionChanged:
            if (obj == m_menuBar) {
                trimMenuBar(m_menuBar);
            }
            break;
        case QEvent::Show:
            // Kate ẩn thanh menu theo Show Menu Bar=false rồi có thứ hiện lại
            // (đo container: khoá false mà thanh vẫn hiện; chưa rõ ai — nghi
            // KXmlGui dựng lại thanh khi plugin thêm menu). Giữ đúng ý khoá.
            if (obj == m_menuBar && m_showMenuBar && !m_showMenuBar->isChecked()) {
                QTimer::singleShot(0, m_menuBar, &QWidget::hide);
            }
            // Thanh hamburgerBar của Kate (kateui.rc) chỉ chứa nút ☰ — nút đã
            // về hàng tab, thanh này để lại một dải trống trên cùng cửa sổ.
            // Trạng thái hiện/ẩn thanh được nhớ trong katestaterc nên bắt ở đây
            // thay vì sửa file .rc (tài khoản cũ không nhận).
            if (obj->objectName() == QLatin1String("hamburgerBar") && obj->inherits("KToolBar")) {
                QTimer::singleShot(0, static_cast<QWidget *>(obj), &QWidget::hide);
            }
            if (auto *edit = qobject_cast<QLineEdit *>(obj); edit && isProjectFilter(edit)) {
                edit->hide();
            } else if (auto *combo = qobject_cast<QComboBox *>(obj)) {
                if (QHBoxLayout *row = projectHeader(combo)) {
                    fixProjectHeader(row, combo->parentWidget());
                }
            }
            if (isViewSpace(obj)) {
                schedulePlaceMenuButton();
            }
            break;
        case QEvent::Hide:
        case QEvent::Resize:
            if (isViewSpace(obj)) {
                schedulePlaceMenuButton();
            }
            break;
        default:
            break;
        }
        return QObject::eventFilter(obj, event);
    }

    ~SamhieKateView() override
    {
        m_mw->guiFactory()->removeClient(this);
    }

private:
    // ✕ ẩn, thêm « thu khung (hideToolView nhận đúng widget mà createToolView
    // trả về — ô chọn dự án là con trực tiếp của nó)
    void fixProjectHeader(QHBoxLayout *row, QWidget *toolView)
    {
        row->itemAt(2)->widget()->hide();
        auto *collapse = new QToolButton(toolView);
        collapse->setAutoRaise(true);
        collapse->setIcon(QIcon::fromTheme(QStringLiteral("sidebar-collapse-left")));
        collapse->setToolTip(QStringLiteral("Hide Sidebar"));
        connect(collapse, &QToolButton::clicked, this, [this, toolView] {
            m_mw->hideToolView(toolView);
        });
        row->addWidget(collapse);
    }

    // Đổi bố cục (chia đôi, đóng view space, kéo khung) sinh cả loạt sự kiện —
    // gom lại, xếp một lần sau vòng sự kiện
    void schedulePlaceMenuButton()
    {
        if (!m_placePending) {
            m_placePending = true;
            QTimer::singleShot(0, this, &SamhieKateView::placeMenuButton);
        }
    }

    void placeMenuButton()
    {
        m_placePending = false;
        QWidget *win = m_mw->window();
        QWidget *target = nullptr;
        QPoint best;
        for (QWidget *vs : win->findChildren<QWidget *>()) {
            if (!isViewSpace(vs) || !vs->isVisible()) {
                continue;
            }
            const QPoint corner = vs->mapTo(win, QPoint(vs->width(), 0));
            if (!target || corner.y() < best.y() || (corner.y() == best.y() && corner.x() > best.x())) {
                target = vs;
                best = corner;
            }
        }
        auto *grid = target ? qobject_cast<QGridLayout *>(target->layout()) : nullptr;
        if (!grid || (m_menuButton && m_menuButton->parentWidget() == target)) {
            return;
        }
        if (!m_menuButton) {
            auto *xmlWin = dynamic_cast<KXmlGuiWindow *>(win);
            auto *action = xmlWin ? qobject_cast<QWidgetAction *>(xmlWin->actionCollection()->action(QStringLiteral("hamburger_menu"))) : nullptr;
            m_menuButton = action ? action->requestWidget(target) : nullptr;
            if (!m_menuButton) {
                return;
            }
        } else if (auto *oldGrid = qobject_cast<QGridLayout *>(m_menuButton->parentWidget()->layout())) {
            oldGrid->removeWidget(m_menuButton);
            m_menuButton->setParent(target);
        }
        if (auto *button = qobject_cast<QToolButton *>(m_menuButton.data())) {
            button->setAutoRaise(true);
        }
        grid->addWidget(m_menuButton, 0, menuColumn(target, grid));
        m_menuButton->show();
    }

    // Cột cho nút ☰: thêm một cột sau nút split và nới hàng breadcrumb + vùng
    // editor (đang trải hết số cột cũ) sang cả cột mới — không thì editor hụt
    // đúng bề ngang nút. Mỗi view space chỉ nới một lần (nút đi rồi quay lại).
    static int menuColumn(QWidget *viewSpace, QGridLayout *grid)
    {
        static const char *const key = "samhieMenuColumn";
        if (const QVariant col = viewSpace->property(key); col.isValid()) {
            return col.toInt();
        }
        const int col = grid->columnCount();
        QList<std::tuple<QWidget *, int, int>> wide; // widget, hàng, số hàng — trải từ cột 0 tới hết
        for (int i = 0; i < grid->count(); ++i) {
            int r, c, rs, cs;
            grid->getItemPosition(i, &r, &c, &rs, &cs);
            if (c == 0 && cs == col && grid->itemAt(i)->widget()) {
                wide.append({grid->itemAt(i)->widget(), r, rs});
            }
        }
        for (auto [w, r, rs] : wide) {
            grid->removeWidget(w);
            grid->addWidget(w, r, 0, rs, col + 1);
        }
        viewSpace->setProperty(key, col);
        return col;
    }

    // Thư mục mở terminal: gốc dự án đang mở → thư mục của file đang xem → home
    QString startDirectory()
    {
        if (QObject *projects = m_mw->pluginView(QStringLiteral("kateprojectplugin"))) {
            const QString dir = projects->property("projectBaseDir").toString();
            if (!dir.isEmpty() && QFileInfo(dir).isDir()) {
                return dir;
            }
        }
        if (KTextEditor::View *view = m_mw->activeView()) {
            const QUrl url = view->document()->url();
            if (url.isLocalFile()) {
                return QFileInfo(url.toLocalFile()).absolutePath();
            }
        }
        return QDir::homePath();
    }

    TerminalTab *createTerminal()
    {
        KPluginFactory *factory = konsoleFactory();
        auto *part = factory ? factory->create<KParts::ReadOnlyPart>(this) : nullptr;
        if (!part) {
            KMessageBox::error(m_mw->window(), QStringLiteral("Konsole is not installed, so Kate cannot open a terminal."));
            return nullptr;
        }
        auto *tab = new TerminalTab(part);
        part->setParent(tab); // đóng tab → xoá part → shell nhận SIGHUP

        // clang-format off
        connect(part, SIGNAL(overrideShortcut(QKeyEvent*,bool&)), this, SLOT(overrideShortcut(QKeyEvent*,bool&)));
        // clang-format on
        // Shell thoát: KonsolePart tự huỷ (Part::terminalExited) → đóng tab.
        // Chạy sau vòng sự kiện, và chỉ khi tab còn đó (tab bị đóng từ Kate thì
        // part cũng huỷ theo — lúc đó không làm gì).
        connect(part, &QObject::destroyed, this, [this, tab = QPointer<TerminalTab>(tab)] {
            QTimer::singleShot(0, this, [this, tab] {
                if (tab && m_mw->widgets().contains(tab)) {
                    m_mw->removeWidget(tab);
                }
            });
        });

        qobject_cast<TerminalInterface *>(part)->showShellInDir(startDirectory());
        m_mw->addWidget(tab);
        m_lastTerminal = tab;
        return tab;
    }

    void focusTerminal(TerminalTab *tab)
    {
        m_mw->activateWidget(tab);
        tab->setFocus();
    }

    void newTerminal()
    {
        rememberEditor();
        if (TerminalTab *tab = createTerminal()) {
            focusTerminal(tab);
        }
    }

    void toggleTerminal()
    {
        // Đang ở terminal → về file trước đó
        if (qobject_cast<TerminalTab *>(m_mw->activeWidget())) {
            if (m_lastEditor) {
                if (auto *view = qobject_cast<KTextEditor::View *>(m_lastEditor.data())) {
                    m_mw->activateView(view->document());
                } else {
                    m_mw->activateWidget(m_lastEditor);
                }
            }
            return;
        }
        rememberEditor();
        TerminalTab *tab = m_lastTerminal && m_mw->widgets().contains(m_lastTerminal) ? m_lastTerminal.data() : nullptr;
        if (!tab) {
            for (QWidget *w : m_mw->widgets()) {
                if ((tab = qobject_cast<TerminalTab *>(w))) {
                    break;
                }
            }
        }
        if (!tab) {
            tab = createTerminal();
        }
        if (tab) {
            focusTerminal(tab);
        }
    }

    void rememberEditor()
    {
        QWidget *w = m_mw->activeWidget();
        if (w && !qobject_cast<TerminalTab *>(w)) {
            m_lastEditor = w;
        }
    }

private Q_SLOTS:
    void overrideShortcut(QKeyEvent *event, bool &override)
    {
        override = !isKateKey(event);
    }

private:
    KTextEditor::MainWindow *m_mw;
    QPointer<QMenuBar> m_menuBar;
    QPointer<QAction> m_showMenuBar; // Ctrl+M của Kate
    QPointer<QWidget> m_menuButton;
    bool m_placePending = false;
    QPointer<TerminalTab> m_lastTerminal;
    QPointer<QWidget> m_lastEditor;
};

class SamhieKatePlugin : public KTextEditor::Plugin
{
    Q_OBJECT
public:
    explicit SamhieKatePlugin(QObject *parent, const QVariantList & = QVariantList())
        : KTextEditor::Plugin(parent)
    {
    }

    QObject *createView(KTextEditor::MainWindow *mw) override
    {
        return new SamhieKateView(mw);
    }
};

K_PLUGIN_FACTORY_WITH_JSON(SamhieKatePluginFactory, "samhie-kate.json", registerPlugin<SamhieKatePlugin>();)

#include "samhie-kate.moc"
