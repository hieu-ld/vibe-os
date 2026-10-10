// Plugin Kate "samhieterminal" — terminal làm TAB trong vùng editor, cạnh các
// file đang mở, như VS Code (user 2026-10-10: mở IDE việc đầu tiên là mở
// terminal gõ lệnh chạy Claude CLI).
//
// Plugin terminal có sẵn của Kate (katekonsoleplugin) chỉ đặt terminal ở khung
// dưới. Kate 26.08 cho plugin đặt một widget bất kỳ thành tab trong vùng
// editor: KTextEditor::MainWindow::addWidget (Compiler Explorer, kết quả Search
// dùng đường này — đọc mã Kate 26.08.1). Tab tự đổi tên theo windowTitle của
// widget, và Kate gọi Q_INVOKABLE shouldClose() trước khi đóng tab / thoát.
//
// Terminal là KonsolePart (kf6/parts/konsolepart) — cùng Konsole của Samhie:
// không có DefaultProfile trong katerc thì Konsole lấy DefaultProfile của
// konsolerc (ProfileManager.cpp) → profile Samhie, tên tab từ samhie-title.sh.
//
//   Ctrl+`        mở/về terminal; đang ở terminal thì quay lại file trước đó
//   Ctrl+Shift+`  terminal mới
// Shell thoát (gõ exit) thì tab tự đóng.

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
#include <kde_terminal_interface.h>

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QKeyEvent>
#include <QPointer>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
const QString kTitle = QStringLiteral("Terminal");

KPluginFactory *konsoleFactory()
{
    static KPluginFactory *factory = KPluginFactory::loadFactory(KPluginMetaData(QStringLiteral("kf6/parts/konsolepart"))).plugin;
    return factory;
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

class SamhieTerminalView : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    explicit SamhieTerminalView(KTextEditor::MainWindow *mw)
        : QObject(mw)
        , m_mw(mw)
    {
        setComponentName(QStringLiteral("samhieterminal"), kTitle);
        setXMLFile(QStringLiteral("ui.rc"));

        QAction *a = actionCollection()->addAction(QStringLiteral("samhie_terminal_new"));
        a->setText(QStringLiteral("New Terminal"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("utilities-terminal")));
        actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_QuoteLeft));
        connect(a, &QAction::triggered, this, &SamhieTerminalView::newTerminal);

        a = actionCollection()->addAction(QStringLiteral("samhie_terminal_toggle"));
        a->setText(QStringLiteral("Switch Between Terminal and File"));
        actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft));
        connect(a, &QAction::triggered, this, &SamhieTerminalView::toggleTerminal);

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
    }

    ~SamhieTerminalView() override
    {
        m_mw->guiFactory()->removeClient(this);
    }

private:
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
    QPointer<TerminalTab> m_lastTerminal;
    QPointer<QWidget> m_lastEditor;
};

class SamhieTerminalPlugin : public KTextEditor::Plugin
{
    Q_OBJECT
public:
    explicit SamhieTerminalPlugin(QObject *parent, const QVariantList & = QVariantList())
        : KTextEditor::Plugin(parent)
    {
    }

    QObject *createView(KTextEditor::MainWindow *mw) override
    {
        return new SamhieTerminalView(mw);
    }
};

K_PLUGIN_FACTORY_WITH_JSON(SamhieTerminalPluginFactory, "samhie-terminal.json", registerPlugin<SamhieTerminalPlugin>();)

#include "samhie-terminal.moc"
