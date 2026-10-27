// Widget style "samhie": bọc theme Kvantum (QProxyStyle) để vẽ thêm vài chỗ
// mà theme Kvantum KHÔNG vẽ được bằng SVG/kvconfig. Danh sách từng chỗ ("mục
// N" trong chú thích) và lý do: style/README.md.
//
// Mã chia theo app/chỗ dùng ở parts/NN-*.inc — chúng nằm TRONG namespace vô
// danh bên dưới và được #include theo đúng thứ tự (phần sau dùng phần trước).
// File này giữ: include, lớp SamhieStyle, các lớp có Q_OBJECT (moc chỉ đọc
// file này) và plugin.

#include <array>

#include <QAbstractButton>
#include <QApplication>
#include <QFontMetricsF>
#include <QtMath>
#include <QAbstractItemView>
#include <QAction>
#include <QActionGroup>
#include <QDir>
#include <QAbstractScrollArea>
#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QBackingStore>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStackedWidget>
#include <QLayout>
#include <QBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QProxyStyle>
#include <QPushButton>
#include <QScrollArea>
#include <QStyleFactory>
#include <QStyleOption>
#include <QStylePlugin>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBar>
#include <QDockWidget>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QSettings>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>
#include <QTextLayout>
#include <QTextBlock>
#include <QTextDocument>
#include <QScrollBar>
#include <QTextEdit>
#include <memory>
#include <functional>
#include <QGridLayout>
#include <QDialog>
#include <QButtonGroup>
#include <cmath>
#include <QCursor>
#include <QAccessible>
#include <QSaveFile>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QSlider>
#include <QFile>
#include <QFileInfo>
#include <dlfcn.h>
#include <QStandardPaths>
#include <QMainWindow>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRegularExpression>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QListView>
#include <QHash>
#include <QSortFilterProxyModel>
#include <QPointer>
#include <QSet>
#include <QTreeWidget>
#include <QTimer>
#include <QTreeView>
#include <QDateTime>
#include <QToolButton>
#include <QUrl>

#include <KCapacityBar>
#include <KPageView>
#include <KPageWidgetModel>
#include <QWindow>
#include <QWidgetAction>

#include <algorithm>
#include <climits>

#include "colors.h"  // sinh từ theme/palette.toml (tools/gen-theme.py)

// Mục 47 (định nghĩa cuối file — lớp có Q_OBJECT, không đặt được trong
// namespace vô danh): bấm một ô của hàng màu thư mục → ghi nhớ ý định, đổi
// .directory khi plugin của Dolphin báo thư mục nào vừa đổi.
enum class FolderChange { Color, Symbol };
void armFolderChange(FolderChange kind, const QString &value);
// Đăng ký nghe KDirNotify TRƯỚC cú bấm đầu tiên: đăng ký là việc bất đồng bộ
// với dbus-daemon — đăng ký lúc bấm thì thông báo của plugin (phát ngay
// trong cú bấm đó) đi trước, bị lỡ (container 2026-10-08).
void watchFolderChanges(const QString &dir, const QString &symbol, const QString &color);
// Mục 80 (định nghĩa cuối file): chế độ sáng/tối người dùng đang chọn, và
// nghe lệnh đổi chế độ khi app đang mở.
bool lightMode();
void watchColorMode(bool light);

namespace {

#include "parts/01-base.inc"
#include "parts/02-dolphin-pane.inc"
#include "parts/03-kate-cards.inc"
#include "parts/04-filters.inc"
#include "parts/05-kate-sidebar.inc"
#include "parts/06-kde-dialogs.inc"
#include "parts/07-icons.inc"
#include "parts/08-dolphin-folders.inc"
#include "parts/09-dolphin-misc.inc"
#include "parts/10-properties.inc"
#include "parts/11-carets.inc"

class SamhieStyle : public QProxyStyle
{
public:
    // Mục 80: bộ màu chọn TRƯỚC khi Kvantum nạp theme và trước khi vẽ gì.
    SamhieStyle() : SamhieStyle(lightMode()) {}
    explicit SamhieStyle(bool light)
        : QProxyStyle((samhie::useLight(light),
                       QStyleFactory::create(light ? QStringLiteral("kvantum") : QStringLiteral("kvantum-dark"))))
        , m_light(light)
    {
        g_light = light;
        watchColorMode(light);
    }

    // Nút + ở góc trái dải tab Dolphin; hành động new_tab (Ctrl+T, menu ☰)
    // nối lại sang openHomeTab khi dải hiện lần đầu (NewTabRewire).
    static void addNewTabButton(QTabWidget *tw)
    {
        if (tw->cornerWidget(Qt::TopLeftCorner))
            return;  // polish lại (đổi style…) — đã có nút
        auto *button = new QToolButton(tw);
        button->setObjectName(QStringLiteral("samhie-new-tab"));
        button->setAutoRaise(true);
        button->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
        button->setToolTip(QStringLiteral("New Tab (Ctrl+T)"));
        button->setFocusPolicy(Qt::NoFocus);
        QObject::connect(button, &QToolButton::clicked, tw, [tw] { openHomeTab(tw); });
        tw->setCornerWidget(button, Qt::TopLeftCorner);
        tw->installEventFilter(new NewTabRewire(tw));
    }

    void polish(QWidget *w) override
    {
        QProxyStyle::polish(w);
        // Lề vùng hiển thị dời cả mục lẫn tiêu đề nhóm (KIO/Dolphin không đặt
        // lề này). KHÔNG dùng setContentsMargins: KFilePlacesView là QFrame,
        // QFrame tự đặt lại contentsMargins theo độ dày khung (đo: mất lề).
        if (isPlacesView(w)) {
            const bool rtl = w->layoutDirection() == Qt::RightToLeft;
            static_cast<ScrollAreaAccess *>(static_cast<QAbstractScrollArea *>(w))
                ->setViewportMargins(rtl ? 0 : SIDEBAR_INDENT, 0, rtl ? SIDEBAR_INDENT : 0, 0);
        }
        if (w->property("samhieCard").toBool())
            m_card.attach(w);  // mục 60
        if (w->inherits("KMultiTabBarTab"))
            w->installEventFilter(&m_sidebarTab);  // mục 66 — sau bộ lọc Kvantum
        // Mục 84 (polish lại = style nạp lại lúc đổi chế độ → sync luôn)
        if (w->inherits("KTitleWidget") && w->objectName().startsWith(QLatin1String("KPageView::TitleWidget")))
            w->setProperty("samhieFollowPalette", true);
        if (auto *label = qobject_cast<QLabel *>(w)) {
            const QWidget *win = w->window();
            const bool about = win->inherits("KAboutApplicationDialog") || win->inherits("KAboutPluginDialog");
            // kabstractaboutdialog_p.cpp (KF 6.31): dòng mô tả = chữ 85%
            if (about && w->testAttribute(Qt::WA_SetPalette) && !w->property("samhieFollowPalette").isValid()
                && qAbs(w->palette().color(QPalette::WindowText).alphaF() - 0.85) < 0.01) {
                w->setProperty("samhieFollowPalette", true);
                w->setProperty("samhieFollowAlpha", 0.85);
            }
            if (label->text().contains(QLatin1String("<a "), Qt::CaseInsensitive))
                w->setProperty("samhieRelink", true);
            if (w->parentWidget() && w->parentWidget()->inherits("KTitleWidget")) {
                w->setProperty("samhieRichTitle", true);
                FollowAppPalette::unstyleTitleLater(label);
            }
        }
        if (w->property("samhieFollowPalette").toBool() || w->property("samhieRelink").toBool()
            || w->property("samhieRichTitle").toBool()) {
            if (w->property("samhieFollowPalette").toBool())
                FollowAppPalette::sync(w);
            w->installEventFilter(&m_followAppPalette);
        }
        if (EditorAlpha::wants(w))
            w->installEventFilter(&m_editorAlpha);  // mục 62
        auto *tw = qobject_cast<QTabWidget *>(w);
        if (tw && tw->documentMode())
            w->installEventFilter(&m_pane);  // cài lại cùng bộ lọc: Qt bỏ bản cũ
        if (isSearchContents(w)) {
            // Lề trong của thẻ = lề của CHÍNH khung nội dung (QWidget), cộng
            // thêm lề bố cục Dolphin (4px). Đặt được ngay lúc này, trước khi
            // Dolphin gắn bố cục — nên chiều cao thanh (Bar::preferredHeight,
            // tính từ toạ độ dòng nút thứ hai) có tính lề ngay lần mở đầu. Lề
            // bố cục thì không: đặt lúc Show là muộn (máy thử: dòng 2 bị cắt).
            // Đáy 0: Dolphin chỉ chừa đúng 4px dưới dòng thứ hai.
            const int m = POPUP_MARGIN + POPUP_PADDING - 4;
            w->setContentsMargins(m, m, m, 0);
            w->parentWidget()->setAutoFillBackground(false);  // viewport: để thấy vùng file trong
            w->installEventFilter(&m_popup);
            w->parentWidget()->installEventFilter(&m_popup);  // đổi cỡ → vẽ lại thẻ
        }
        // Mục 20: ô tìm / ô lọc không chớp viền cyan lúc mất focus. Viền
        // focus đã gỡ khi vẽ (drawPrimitive), nhưng bộ lọc sự kiện của Kvantum
        // bắt FocusOut rồi cho viền mờ dần TỪ trạng thái "focused" — ghi cứng,
        // không đọc cờ focus (eventFiltering.cpp 1.1.6, FocusOut) → chớp một
        // khung cyan (máy ảo 2026-10-06, bấm Filter hoặc bấm ra ngoài). Với
        // ô nhập, bộ lọc đó chỉ lo hiệu ứng focus + màu chữ trên thanh công
        // cụ (các ô này không nằm trên thanh công cụ) → gỡ khỏi riêng chúng.
        if (auto *edit = qobject_cast<QLineEdit *>(w); edit && inDolphinLocationOrSearch(w)) {
            w->removeEventFilter(baseStyle());
            // Mục 24: ô tìm không có nút "lưu tìm kiếm" (thêm một mục vào
            // sidebar — Samhie giữ sidebar gọn). Dolphin chỉ bật/tắt nó
            // (setEnabled, search/bar.cpp 26.08.1), không ẩn/hiện lại.
            const auto actions = edit->actions();
            for (QAction *a : actions) {
                if (a->icon().name() == QLatin1String("document-save-symbolic"))
                    a->setVisible(false);
            }
        }
        if (w->inherits("KItemListContainer")) {  // mục 13
            auto *view = qobject_cast<QGraphicsView *>(static_cast<QAbstractScrollArea *>(w)->viewport());
            if (view) {
                clearAlternateBase(view, view->palette());
                if (QGraphicsScene *scene = view->scene())
                    clearAlternateBase(scene, scene->palette());
            }
        }
        if (w->inherits("DolphinTabWidget"))
            addNewTabButton(static_cast<QTabWidget *>(w));
        // Tab Dolphin không icon, như bản vẽ và tab Konsole. Dolphin chỉ gán
        // icon khi dải tab ĐANG hiện → tab đầu lúc mở cửa sổ không có, mở
        // thêm tab mới có (container 2026-10-04). Icon cỡ 0 = không vẽ.
        if (w->inherits("DolphinTabBar"))
            static_cast<QTabBar *>(w)->setIconSize(QSize(0, 0));
        // Dòng widget trong menu (QWidgetAction — vd dòng Zoom ⊖ ⟲ ⊕ trong menu
        // ▦ của Dolphin) cao bằng dòng menu thường: Dolphin cho nó cao bằng nút
        // + 4px → ~31px giữa các dòng 41px (container 2026-10-04). ZoomWidget
        // không có Q_OBJECT nên nhận ra theo chỗ đứng: nút → widget → QMenu.
        if (isMenuRowToolButton(w)) {
            QWidget *row = w->parentWidget();
            row->setMinimumHeight(row->fontMetrics().height() + 2 * MENUITEM_FRAME);
            // Mục 21: chỉ ô hover, vuông.
            auto *tb = static_cast<QToolButton *>(w);
            tb->setAutoRaise(true);
            const int side = qMax(tb->iconSize().width(), tb->iconSize().height()) + 2 * TOOL_PADDING;
            tb->setFixedSize(side, side);
        }
        // DolphinStatusBar::paintEvent: drawRoundedRect(palette().window()).
        if (qobject_cast<QMenu *>(w))
            w->installEventFilter(&m_menuForwarder);  // mục 12
        if (isRenameEditor(w)) {  // mục 49
            auto *te = static_cast<QTextEdit *>(w);
            te->viewport()->setAutoFillBackground(false);
            if (!w->property("_samhie_highlight").isValid()) {
                QPalette pal = te->palette();
                w->setProperty("_samhie_highlight", pal.color(QPalette::Highlight));
                for (auto group : {QPalette::Active, QPalette::Inactive})
                    pal.setColor(group, QPalette::Highlight, Qt::transparent);
                te->setPalette(pal);
            }
            // Đổi vùng chọn / chữ → vẽ lại lớp nền (khung) chứ không chỉ viewport.
            QObject::connect(te, &QTextEdit::selectionChanged, te, [te] { te->update(); });
            QObject::connect(te, &QTextEdit::textChanged, te, [te] { te->update(); });
            if (!te->findChild<QWidget *>(QStringLiteral("samhie-text-caret")))
                new TextEditCaret(te);
        }
        // Mục 50: menu Open With / Open Folder With (KFileItemActions, KIO
        // 6.30: menuAction "openWith_submenu") hiện icon app (user
        // 2026-10-08). Polish lúc mở lần đầu — KIO đã điền đủ app.
        if (auto *menu = qobject_cast<QMenu *>(w);
            menu && menu->menuAction()->objectName() == QLatin1String("openWith_submenu"))
            enableMenuIcons(menu);
        // Mục 48: hộp Create Folder. Lưới được KIO điền sau khi dựng hộp —
        // polish có thể tới sớm hơn (tính cỡ lúc dựng) → làm sau một nhịp.
        if (qobject_cast<QDialog *>(w) && w->findChild<QWidget *>(QStringLiteral("chooseIconBox"))) {
            QTimer::singleShot(0, w, [w] { setupNewFolderDialog(w); });
        }
        if (w->inherits("DolphinContextMenu")) {
            hideItemCreateNew(static_cast<QMenu *>(w));
            hideViewportFolderColors(static_cast<QMenu *>(w));
            hideGitClone(static_cast<QMenu *>(w));  // mục 52
            setupFolderColorRow(static_cast<QMenu *>(w));  // mục 47
        }
        if (qobject_cast<QMenu *>(w) && !w->parentWidget() && !w->inherits("DolphinContextMenu")
            && QCoreApplication::applicationName() == QLatin1String("dolphin")) {
            auto *menu = static_cast<QMenu *>(w);  // mục 17: menu ☰
            removeHamburgerCreateNew(menu);
            flattenHamburgerConfigure(menu);  // mục 29
            trimHamburgerMoreHelp(menu);      // mục 40
            QObject::connect(menu, &QMenu::aboutToShow, menu, [menu] {
                removeHamburgerCreateNew(menu);
                flattenHamburgerConfigure(menu);
                trimHamburgerMoreHelp(menu);
                trimPlacesMenu(menu);  // mục 19
            });
        }
        // Mục 24: Dolphin không có dòng Zoom trong menu (⊖ ⟲ ⊕ — Ctrl + lăn
        // chuột, Ctrl +/−/0 vẫn chạy: đó là các hành động zoom_in/out/reset,
        // dòng Zoom chỉ là widget gom ba nút). Gỡ lúc menu sắp hiện: Dolphin
        // thêm hành động sau khi dựng menu. GỠ chứ không setVisible(false):
        // QMenu đã dựng widget của dòng, ẩn hành động thì widget vẫn nằm đè
        // lên dòng Sort By (container 2026-10-06). Cùng chỗ: "Focus Places
        // Panel" / "Focus Terminal Panel" trong Show Panels (user 2026-10-06)
        // — gỡ khỏi menu, hành động vẫn gắn cửa sổ (KXmlGui) nên Ctrl+P /
        // Ctrl+Shift+F4 vẫn chạy; ẩn hành động thì Qt tắt luôn phím tắt.
        // Cũng vậy: "Folders" (cây thư mục — trùng sidebar Places + thanh
        // đường dẫn; user 2026-10-06), F7 vẫn mở. Và Rating / Tags / Comment
        // trong ▦ → Sort By (user 2026-10-07): dữ liệu Baloo, Samhie không có
        // chỗ gắn — cùng lý do bỏ khỏi danh sách phím tắt (REMOVED_SHORTCUTS).
        if (auto *menu = qobject_cast<QMenu *>(w); menu && QCoreApplication::applicationName() == QLatin1String("dolphin")
                                                   && !menu->property("samhieNoZoom").toBool()) {
            menu->setProperty("samhieNoZoom", true);
            QObject::connect(menu, &QMenu::aboutToShow, menu, [menu] {
                const auto actions = menu->actions();
                for (QAction *a : actions) {
                    if (a->objectName() == QLatin1String("zoom")
                        || a->objectName() == QLatin1String("focus_places_panel")
                        || a->objectName() == QLatin1String("focus_terminal_panel")
                        || a->objectName() == QLatin1String("show_folders_panel")
                        || a->objectName() == QLatin1String("sort_by_rating")
                        || a->objectName() == QLatin1String("sort_by_tags")
                        || a->objectName() == QLatin1String("sort_by_comment"))
                        menu->removeAction(a);
                }
            });
        }
        if (auto *menu = qobject_cast<QMenu *>(w); menu && QCoreApplication::applicationName() == QLatin1String("konsole")) {
            trimKonsoleMenu(menu);  // mục 55 — chỉ gỡ khi menu có đúng các mục đó
            QObject::connect(menu, &QMenu::aboutToShow, menu, [menu] { trimKonsoleMenu(menu); });
        }
        if (qobject_cast<QMenu *>(w) && w->parentWidget() && w->parentWidget()->inherits("DolphinTabBar"))
            trimTabMenu(static_cast<QMenu *>(w));  // mục 19
        if (auto *menu = qobject_cast<QMenu *>(w); menu && qobject_cast<QTabBar *>(w->parentWidget())
            && w->parentWidget()->parentWidget() && w->parentWidget()->parentWidget()->inherits("Konsole::TabbedViewContainer")) {
            for (QAction *a : menu->actions()) {  // mục 54
                if (a->objectName() == QLatin1String("edit-rename"))
                    menu->removeAction(a);
            }
        }
        if (auto *edit = qobject_cast<QLineEdit *>(w); edit && edit->font().family() == QLatin1String("Dongle")
                                                       && !edit->property("samhieShortCaret").toBool())
            new ShortCaret(edit);  // mục 22 (polish lại: đã có thì thôi)
        // Nút trong ô nhập (⌫, lưu tìm kiếm — QLineEdit tự dựng) giữ nguyên.
        if (auto *tb = qobject_cast<QToolButton *>(w); tb && tb->toolButtonStyle() == Qt::ToolButtonIconOnly
                                                       && !qobject_cast<QLineEdit *>(w->parentWidget())
                                                       && inDolphinLocationOrSearch(w)) {
            const int side = qMax(tb->iconSize().width(), tb->iconSize().height()) + 2 * TOOL_PADDING;
            tb->setMinimumSize(side, side);  // mục 21: nút × thanh tìm
        }
        if (auto *tb = qobject_cast<QToolButton *>(w); tb && tb->toolButtonStyle() == Qt::ToolButtonIconOnly
                                                       && tb->arrowType() == Qt::NoArrow
                                                       && qobject_cast<QDialog *>(w->window())
                                                       && !qobject_cast<QLineEdit *>(w->parentWidget())
                                                       && !qobject_cast<QToolBar *>(w->parentWidget())
                                                       && !qobject_cast<QTabBar *>(w->parentWidget())) {
            const int side = qMax(tb->iconSize().width(), tb->iconSize().height()) + 2 * TOOL_PADDING;
            tb->setMinimumSize(qMax(side, tb->minimumWidth()), qMax(side, tb->minimumHeight()));  // mục 58
        }
        if (auto *tb = qobject_cast<QToolButton *>(w); tb && w->property("samhieSquareHover").toBool()) {
            const int side = qMax(tb->iconSize().width(), tb->iconSize().height()) + 2 * TOOL_PADDING;
            tb->setMinimumSize(qMax(side, tb->minimumWidth()), qMax(side, tb->minimumHeight()));  // mục 68
        }
        // Mục 24: thanh lọc không có 🔒 "Keep Filter When Changing Folders" và
        // Aᴮ "Match case" (user 2026-10-06: cho gọn). Nhận theo icon — nút
        // không có tên (filterbar.cpp 26.08.1). Ẩn lúc polish: con được polish
        // khi thanh hiện lần đầu, TRƯỚC khi Qt hiện các con — con đã ẩn tường
        // minh thì Qt bỏ qua. Bỏ chọn trước khi ẩn: Aᴮ đọc trạng thái cũ từ
        // cấu hình, ẩn mà còn bật thì không còn chỗ tắt.
        if (auto *tb = qobject_cast<QToolButton *>(w); tb && inFilterBar(w)) {
            const QString icon = tb->icon().name();
            if (icon == QLatin1String("object-unlocked") || icon == QLatin1String("object-locked")
                || icon == QLatin1String("format-text-superscript") || icon == QLatin1String("format-text-bold")) {
                tb->setChecked(false);
                tb->hide();
                if (icon.startsWith(QLatin1String("object-")))
                    tb->setProperty("samhieFilterBarHeight", true);  // xem lề dưới ngay sau
            }
        }
        // Bỏ 🔒 thì ô lọc dính viền trái vùng file, × dính góc bo cửa sổ →
        // lề trái/phải như thẻ tìm; lề dưới cho thanh khỏi dính đáy cửa sổ
        // (user 2026-10-06). Chiều cao thanh lọc là sizeHint cao nhất của các
        // con, TÍNH CẢ 🔒 đã ẩn (FilterBar::preferredHeight) — 🔒 khai cao
        // thêm đúng lề dưới (sizeFromContents), không thì lề cắt mất ô.
        if (qobject_cast<QLineEdit *>(w) && inFilterBar(w) && w->parentWidget()->layout())
            w->parentWidget()->layout()->setContentsMargins(POPUP_MARGIN, 0, POPUP_MARGIN, POPUP_MARGIN);
        // Mục 24: popup Filter của thanh tìm không có dòng "For more advanced
        // searches:" + nút cài/mở KFind (search/popup.cpp 26.08.1: nhãn, nút
        // ngay sau, khoảng trống 12px ngay trước — ẩn cả ba).
        if (auto *label = qobject_cast<QLabel *>(w);
            label && label->text() == QLatin1String("For more advanced searches:")) {
            if (auto *box = qobject_cast<QBoxLayout *>(label->parentWidget() ? label->parentWidget()->layout() : nullptr)) {
                const int i = box->indexOf(label);
                if (QLayoutItem *next = box->itemAt(i + 1); next && next->widget())
                    next->widget()->hide();
                if (QLayoutItem *gap = box->itemAt(i - 1); gap && gap->spacerItem())
                    gap->spacerItem()->changeSize(0, 0);
                label->hide();
            }
        }
        // Mục 24: popup Filter cũng không có nhóm "Search using:" — chọn giữa
        // Simple search và File Indexing (Baloo). Image không cài trình đánh
        // chỉ mục (kf6-baloo-file) nên File Indexing không ra kết quả và nút
        // ⚙ cạnh nó mở trang cài đặt không có; còn một công cụ thì Dolphin
        // build không Baloo cũng không hiện nhóm này. /etc/xdg/dolphinrc khoá
        // SearchTool=Filenamesearch. Thứ tự (search/popup.cpp 26.08.1): khoảng
        // trống 12, nhãn, dòng Simple search (nút + ⓘ), dòng File Indexing
        // (nút + ⓘ + ⚙), khoảng trống 12. Hai dòng là QHBoxLayout: chuyển các
        // con sang một widget ẩn chứ không hide() — Dolphin tự setVisible ⓘ
        // của Simple search mỗi lần đổi "Search in" (Popup::updateState).
        if (auto *label = qobject_cast<QLabel *>(w); label && label->text() == QLatin1String("Search using:")) {
            if (auto *box = qobject_cast<QBoxLayout *>(label->parentWidget() ? label->parentWidget()->layout() : nullptr)) {
                const int i = box->indexOf(label);
                auto *hidden = new QWidget(label->parentWidget());
                hidden->hide();
                for (int row = i + 1; row <= i + 2; ++row) {
                    QLayout *rowLayout = box->itemAt(row) ? box->itemAt(row)->layout() : nullptr;
                    QList<QWidget *> rowWidgets;
                    for (int j = 0; rowLayout && j < rowLayout->count(); ++j) {
                        if (QWidget *rw = rowLayout->itemAt(j)->widget())
                            rowWidgets << rw;
                    }
                    for (QWidget *rw : rowWidgets)
                        rw->setParent(hidden);  // Qt tự gỡ khỏi dòng
                }
                for (int gap : {i - 1, i + 3}) {
                    if (QLayoutItem *item = box->itemAt(gap); item && item->spacerItem())
                        item->spacerItem()->changeSize(0, 0);
                }
                label->hide();
            }
        }
        // Mục 28: Tab không dừng ở nhãn tiêu đề popup Filter ("Search in:"…,
        // popup.cpp 26.08.1) và nhãn dòng trạng thái ("1 folder",
        // dolphinstatusbar.cpp) — hiện con trỏ chữ trên dòng không gõ được.
        if (auto *label = qobject_cast<QLabel *>(w)) {
            bool inStatusBar = false;  // nhãn nằm trong khung trượt của dòng trạng thái
            for (const QWidget *a = label->parentWidget(); a && !inStatusBar; a = a->parentWidget())
                inStatusBar = a->inherits("DolphinStatusBar");
            if (inStatusBar || label->window()->inherits("Search::Popup"))
                dropKeyboardSelection(label);
            if (label->window()->inherits("Search::Popup"))
                label->setIndent(KEY_FOCUS_PAD);
        }
        // Mục 28: radio popup Filter lùi vào KEY_FOCUS_PAD → lề trái khung chứa
        // bớt đúng ngần ấy (một lần cho mỗi khung).
        if ((isPopupToggle(w) || (qobject_cast<QLabel *>(w) && w->window()->inherits("Search::Popup")))
            && w->parentWidget() && !w->parentWidget()->property("samhieFocusPad").toBool()) {
            QWidget *box = w->parentWidget();
            QMargins m = box->contentsMargins();
            m.setLeft(qMax(0, m.left() - KEY_FOCUS_PAD));
            box->setContentsMargins(m);
            box->setProperty("samhieFocusPad", true);
        }
        if (w->inherits("Search::Popup"))
            w->installEventFilter(&m_popupAnchor);  // mục 26
        if (w->inherits("KDEPrivate::KPageListView"))
            static_cast<QListView *>(w)->setSpacing(PAGE_LIST_GAP);  // mục 32
        // Mục 35: hộp thoại dựng xong danh sách rồi mới hiện → lúc polish đã đủ.
        if (auto *tree = qobject_cast<QTreeWidget *>(w);
            tree && tree->parentWidget() && tree->parentWidget()->inherits("KShortcutsEditor"))
            trimShortcutList(tree->invisibleRootItem());
        // Mục 34
        if (auto *tabs = qobject_cast<QTabWidget *>(w); tabs && tabs->parentWidget()
            && tabs->parentWidget()->inherits("ViewSettingsPage")) {
            for (int i = 0; i < tabs->count(); ++i) {
                if (tabs->tabIcon(i).name() == QLatin1String("view-list-details"))
                    tabs->setTabVisible(i, false);
            }
        }
        // Mục 41: như mục 38 — lớp lọc chèn giữa model và lớp tìm/xếp hạng.
        // Chèn SAU một nhịp: danh sách được polish giữa lúc KCommandBar còn
        // dựng (setModel chưa xong) — chèn ngay thì ô "current" cũ của view
        // không được xoá, setActions chọn dòng đầu là Dolphin crash (gdb,
        // container 2026-10-07). Nhịp đó KDE đã nạp danh sách và chọn dòng
        // đầu → chọn lại dòng đầu sau khi lọc (như reselectFirst của KDE).
        if (auto *tv = qobject_cast<QTreeView *>(w); tv && tv->parentWidget()
            && tv->parentWidget()->inherits("KCommandBar")) {
            QTimer::singleShot(0, tv, [tv] {
                auto *rank = qobject_cast<QSortFilterProxyModel *>(tv->model());
                if (!rank || !rank->sourceModel() || dynamic_cast<CommandBarFilter *>(rank->sourceModel()))
                    return;
                auto *filter = new CommandBarFilter(tv->window(), rank);
                filter->setSourceModel(rank->sourceModel());
                rank->setSourceModel(filter);
                tv->setCurrentIndex(rank->index(0, 0));
            });
        }
        // Mục 43. Mọi thứ được dựng trong hàm tạo KIconDialog; danh sách chỉ
        // nạp icon lúc hiện (showEvent) → polish là kịp. Lớp lọc chèn DƯỚI
        // lớp tìm/lọc symbolic của KDE: hộp thoại chọn sẵn icon hiện tại và
        // đọc icon được chọn bằng chỉ số của lớp đó — lớp đó vẫn nằm trên cùng.
        // Mục 45: hộp mở từ nút icon của Add/Edit Places Entry (KIconButton
        // "icon button" là cha của KIconDialog) → chỉ icon nét. Mỗi nút dựng
        // hộp của riêng nó một lần, nên chế độ không đổi.
        if (w->inherits("KIconDialog")) {
            const QWidget *button = w->parentWidget();
            const bool strokeOnly = button && button->objectName() == QLatin1String("icon button")
                && button->window()->inherits("KFilePlaceEditDialog");
            if (auto *canvas = w->findChild<QListView *>(QStringLiteral("canvas"))) {
                auto *search = qobject_cast<QSortFilterProxyModel *>(canvas->model());
                if (search && search->sourceModel() && !dynamic_cast<IconThemeFilter *>(search->sourceModel())) {
                    auto *filter = new IconThemeFilter(strokeOnly, search);
                    filter->setSourceModel(search->sourceModel());
                    search->setSourceModel(filter);
                }
            }
            // Ô chọn nhóm: bỏ nhóm bộ icon không có, bỏ "Other" (ảnh PNG rời
            // trong thư mục icon, không thuộc bộ nào) và vạch ngăn thừa.
            // KIconDialog::setup() đổi cờ của dòng CUỐI (vốn là Other) chỉ
            // khi khoá chọn icon riêng — Dolphin không khoá.
            if (auto *combo = w->findChild<QComboBox *>(QStringLiteral("contextCombo"))) {
                const QSet<int> keep = iconThemeContexts(strokeOnly);
                for (int i = combo->count() - 1; i >= 0; --i) {
                    const QVariant id = combo->itemData(i);
                    const bool separator = combo->itemData(i, Qt::AccessibleDescriptionRole).toString()
                        == QLatin1String("separator");
                    if ((!separator && (!id.isValid() || !keep.contains(id.toInt())))
                        || (separator && i == combo->count() - 1))
                        combo->removeItem(i);
                }
            }
            // Nút Browse… (icon folder-open — trùng nút chọn thư mục của
            // Configure Dolphin) dùng icon riêng (user 2026-10-07).
            if (auto *box = w->findChild<QDialogButtonBox *>(QStringLiteral("buttonBox"))) {
                for (QAbstractButton *b : box->buttons()) {
                    if (box->buttonRole(b) == QDialogButtonBox::HelpRole)
                        b->setIcon(QIcon::fromTheme(QStringLiteral("samhie-browse")));
                }
            }
        }
        // Mục 45: Add Entry mở với icon KIO::iconNameForUrl (thư mục màu)
        // → đổi sẵn sang icon nét. Hàm tạo hộp thoại đã đặt icon; polish lúc
        // hiện. Thuộc tính `icon` của KIconButton (Q_PROPERTY, KF 6.30).
        if (w->inherits("KFilePlaceEditDialog")) {
            QWidget *button = w->findChild<QWidget *>(QStringLiteral("icon button"));
            const QString icon = button ? button->property("icon").toString() : QString();
            if (!icon.isEmpty() && !icon.startsWith(QLatin1Char('/')) && !isStrokeIconName(icon)
                && QIcon::hasThemeIcon(QStringLiteral("samhie-folder")))
                button->setProperty("icon", strokePlaceIcon(icon));
        }
        // Mục 45: model sidebar (Dolphin dùng chung một model cho mọi cửa sổ;
        // hộp chọn file mỗi hộp một model) — móc một lần, sau một nhịp (view
        // được polish trước khi gắn model). Đổi luôn mục đã có từ trước.
        if (isPlacesView(w)) {
            QTimer::singleShot(0, w, [w] {
                QAbstractItemModel *model = static_cast<QAbstractItemView *>(w)->model();
                if (!model || !model->inherits("KFilePlacesModel"))
                    return;
                // Mục 46: mỗi view một delegate khe (model có thể dùng chung).
                // Xét lại sau một nhịp mỗi khi danh sách đổi — KIO ẩn/hiện
                // dòng (setRowHidden) ngay sau khi model báo.
                if (!w->findChild<QObject *>(QStringLiteral("samhie-pinned-gap"), Qt::FindDirectChildrenOnly)) {
                    auto *gap = new PinnedGapDelegate(static_cast<QListView *>(w));
                    auto later = [gap] { QTimer::singleShot(0, gap, [gap] { gap->update(); }); };
                    QObject::connect(model, &QAbstractItemModel::rowsInserted, gap, later);
                    QObject::connect(model, &QAbstractItemModel::rowsRemoved, gap, later);
                    QObject::connect(model, &QAbstractItemModel::rowsMoved, gap, later);
                    QObject::connect(model, &QAbstractItemModel::modelReset, gap, later);
                    QObject::connect(model, &QAbstractItemModel::layoutChanged, gap, later);
                    QObject::connect(model, &QAbstractItemModel::dataChanged, gap, later);
                    gap->update();
                }
                if (model->property("_samhie_stroke").toBool())
                    return;
                model->setProperty("_samhie_stroke", true);
                // Sau một nhịp: editPlace ngay trong lúc model đang báo thêm
                // dòng là sửa model giữa chừng. Xét lại cả danh sách (vài
                // chục dòng) — chỉ số dòng có thể đã đổi sau nhịp đó.
                auto strokeAll = [model] { strokePlaceIcons(model, 0, model->rowCount() - 1); };
                QObject::connect(model, &QAbstractItemModel::rowsInserted, model,
                                 [model, strokeAll] { QTimer::singleShot(0, model, strokeAll); });
                strokeAll();
            });
        }
        // Mục 38: lớp lọc chèn một lần (model của view là lớp sắp xếp/tìm).
        if (auto *lv = qobject_cast<QListView *>(w); lv && lv->parentWidget()
            && lv->parentWidget()->inherits("ContextMenuSettingsPage")) {
            auto *sort = qobject_cast<QSortFilterProxyModel *>(lv->model());
            if (sort && sort->sourceModel() && !dynamic_cast<ContextMenuRowFilter *>(sort->sourceModel())) {
                auto *filter = new ContextMenuRowFilter(sort);
                filter->setSourceModel(sort->sourceModel());
                sort->setSourceModel(filter);
            }
        }
        // Mục 36. Danh sách trang được polish ngay lúc KPageView tạo nó (trang
        // đầu tiên vừa thêm, model còn rỗng — đo container 2026-10-07) → làm
        // sau một nhịp, khi hộp thoại đã thêm đủ trang.
        // Mục 77: chữ trang đang chọn là HighlightedText (KDE tô — dạng icon
        // trong KPageListViewDelegate, dạng danh sách ở drawControl dưới).
        // Bản sáng HighlightedText = chữ trên ô cyan (trắng) → trên ô
        // PAGE_ITEM_SELECTED xám mất chữ (container 2026-10-20) → chữ thường.
        // Đặt lại mỗi lần polish (đổi chế độ = style mới), lấy từ bảng màu
        // của theme Kvantum đang nạp.
        if (w->inherits("KDEPrivate::KPageListView") || w->inherits("KDEPrivate::KPageTreeView")) {
            const QPalette theme = standardPalette();
            QPalette pal = w->palette();
            for (auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled})
                pal.setColor(group, QPalette::HighlightedText,
                             theme.color(group, m_light ? QPalette::Text : QPalette::HighlightedText));
            w->setPalette(pal);
        }
        if (w->inherits("KateColorTreeWidget"))  // mục 86 (delegate đặt sẵn trong constructor)
            HoverUnderRow::attach(static_cast<QAbstractItemView *>(w));
        if (w->inherits("KDEPrivate::KPageListView")) {
            PageListRow::attach(static_cast<QAbstractItemView *>(w));
            QTimer::singleShot(0, w, [w] {
                PageListRow::attach(static_cast<QAbstractItemView *>(w));
                // Mục 36 (Dolphin) và 56 (Konsole): tên gốc → tên riêng.
                static const QHash<QString, QString> dolphin = {
                    {QStringLiteral("org.kde.dolphin"), QStringLiteral("samhie-settings-interface")}};
                static const QHash<QString, QString> konsole = {
                    {QStringLiteral("utilities-terminal"), QStringLiteral("samhie-konsole-general")},
                    {QStringLiteral("folder-temp"), QStringLiteral("samhie-konsole-tempfiles")},
                    {QStringLiteral("image-jpeg"), QStringLiteral("samhie-konsole-thumbnails")}};
                static const QHash<QString, QString> profile = {
                    {QStringLiteral("utilities-terminal"), QStringLiteral("samhie-konsole-general")},
                    {QStringLiteral("kcolorchooser"), QStringLiteral("preferences-desktop-theme")},
                    {QStringLiteral("input-keyboard"), QStringLiteral("preferences-desktop-keyboard")},
                    {QStringLiteral("input-mouse"), QStringLiteral("samhie-konsole-mouse")}};
                const QWidget *win = w->window();
                const QHash<QString, QString> *icons = win->inherits("DolphinSettingsDialog")  ? &dolphin
                    : win->inherits("Konsole::ConfigurationDialog")                            ? &konsole
                    : win->inherits("Konsole::EditProfileDialog")                              ? &profile
                                                                                               : nullptr;
                if (!icons)
                    return;
                const QWidget *a = w->parentWidget();
                while (a && !qobject_cast<const KPageView *>(a))
                    a = a->parentWidget();
                auto *model = a ? qobject_cast<KPageWidgetModel *>(static_cast<const KPageView *>(a)->model()) : nullptr;
                for (int r = 0; model && r < model->rowCount(); ++r) {
                    KPageWidgetItem *item = model->item(model->index(r, 0));
                    if (item && icons->contains(item->icon().name()))
                        item->setIcon(QIcon::fromTheme(icons->value(item->icon().name())));
                }
            });
        }
        if (w->inherits("TabConnectedWidget"))
            w->installEventFilter(&m_noPaint);  // mục 30
        if (qobject_cast<QLineEdit *>(w) && w->parentWidget() && w->parentWidget()->inherits("KCommandBar"))
            w->installEventFilter(&m_keepCommandBarFocus);  // mục 57
        if (w->inherits("KCommandBar") || w->inherits("HUDDialog"))
            setupCommandBar(static_cast<QFrame *>(w));  // mục 75, 79

        if (w->inherits("KPropertiesDialog"))  // mục 53
            w->installEventFilter(&m_propsSetup);
        if (w->inherits("Baloo::FileMetaDataWidget"))
            w->installEventFilter(&m_detailsAlign);
        if (qobject_cast<QPlainTextEdit *>(w) && w->parentWidget() && w->parentWidget()->inherits("KCommentWidget"))
            w->installEventFilter(&m_commentHeight);
        if (qobject_cast<QLabel *>(w))
            w->installEventFilter(&m_monoSpans);  // mục 74
        if (CrumbIndent::watches(w)) {  // mục 51
            w->installEventFilter(&m_crumbIndent);
            CrumbIndent::schedule(w->parentWidget());
        }
        if (isUrlNavigatorButton(w)) {
            if (w->inherits("KDEPrivate::KUrlNavigatorToggleButton")) {
                w->setMinimumWidth(TOGGLE_SIDE);  // mục 20: đủ chỗ cho ô vuông
                w->installEventFilter(&m_noPencil);  // mục 24
            } else
                w->installEventFilter(&m_urlHover);
        }
        // Mục 24: không có panel Information (F11, user 2026-10-06): icon to
        // bằng cả bề ngang panel đẩy thông tin xuống phải cuộn, mà Properties
        // → Details đã có đủ. Ẩn hành động (khỏi menu Show Panels, F11 tắt
        // theo) và giữ dock ẩn (HideDock) — objectName "infoDock"
        // (DolphinMainWindow::setupDockWidgets 26.08.1).
        if (auto *dock = qobject_cast<QDockWidget *>(w);
            dock && dock->objectName() == QLatin1String("infoDock") && dock->window()->inherits("DolphinMainWindow")) {
            dock->toggleViewAction()->setVisible(false);
            dock->installEventFilter(&m_hideDock);
            if (!dock->isHidden())
                QMetaObject::invokeMethod(dock, &QWidget::hide, Qt::QueuedConnection);
        }
        // Mục 27. Góc đặt theo cửa sổ (dock được polish khi chưa nằm trong cửa
        // sổ), và SAU một nhịp: ngay sau style, KMainWindow nạp lại trạng thái
        // đã lưu (QMainWindow::restoreState, gồm cả góc) — đặt ngay thì bị đè
        // về mặc định (đo container 2026-10-07).
        if (auto *win = qobject_cast<QMainWindow *>(w); win && win->inherits("DolphinMainWindow")) {
            QTimer::singleShot(0, win, [win] {
                win->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
                win->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
            });
        }
        // TerminalPanel đặt lề bố cục 0 lúc dựng (terminalpanel.cpp 26.08.1),
        // không đặt lại.
        if (auto *dock = qobject_cast<QDockWidget *>(w);
            dock && dock->objectName() == QLatin1String("terminalDock")) {
            if (dock->widget() && dock->widget()->layout())
                dock->widget()->layout()->setContentsMargins(TERMINAL_PAD, TERMINAL_GAP + TERMINAL_PAD, 0, 0);
            dock->installEventFilter(&m_pane);
        }
        if (qobject_cast<QToolBar *>(w) && w->window()->inherits("DolphinMainWindow"))
            w->setContextMenuPolicy(Qt::PreventContextMenu);  // mục 18
        // Mục 44: hộp thoại chọn file của KDE (KFileWidget — vd Browse… của hộp
        // chọn icon; cũng là hộp Open/Save của mọi app KDE). Thanh dựng xong
        // trong hàm tạo KFileWidget, polish lúc hiện. Nhận theo tên icon
        // (kdiroperator.cpp / kfilewidget.cpp KIO 6.30: Compact =
        // view-list-details, Details = view-list-tree). Thanh nằm trong
        // m_opsWidget, trong bộ chia (sidebar | file) của KFileWidget (đo
        // container 2026-10-08). Gỡ khỏi thanh/menu, hành động vẫn gắn trên
        // hộp thoại → phím tắt (F5, Ctrl+lăn chuột, Ctrl+−/+, F9, F11) vẫn chạy.
        if (auto *bar = qobject_cast<QToolBar *>(w); bar && insideClass(w, "KFileWidget")) {
            static const QSet<QString> dropped = {
                QStringLiteral("view-list-details"),  // Compact View (user 2026-10-07)
                QStringLiteral("bookmarks"),
                // user 2026-10-08: thanh zoom (Dolphin cũng bỏ dòng Zoom),
                // ↻ Reload (hộp thoại tự cập nhật), 🖼 Show Preview
                QStringLiteral("zoom-out"), QStringLiteral("zoom-in"),
                QStringLiteral("view-refresh"), QStringLiteral("view-preview"),
            };
            const QList<QAction *> actions = bar->actions();
            for (int i = 0; i < actions.size(); ++i) {
                QAction *a = actions[i];
                const bool slider = qobject_cast<QSlider *>(bar->widgetForAction(a));
                // Vạch ngăn giữa thanh zoom và New Folder thành vạch lẻ loi.
                const bool orphan = a->isSeparator() && i > 0 && actions[i - 1]->icon().name() == QLatin1String("zoom-in");
                if (slider || orphan || dropped.contains(a->icon().name()))
                    bar->removeAction(a);
                // Menu ⚙ Options (dựng một lần): bỏ Allow Expansion in Details
                // View (Dolphin Samhie đã tắt mở dạng cây), Show Places Panel
                // (F9 — tắt nhầm là mất sidebar), Show Preview Panel (F11 —
                // như panel Information Dolphin); giữ Show Hidden Files, Show
                // Quick Filter (user 2026-10-08). Allow Expansion là dòng tích
                // duy nhất không có phím tắt.
                if (a->icon().name() == QLatin1String("configure") && a->menu()) {
                    for (QAction *m : a->menu()->actions()) {
                        const QKeySequence key = m->shortcut();
                        if (key == QKeySequence(Qt::Key_F9) || key == QKeySequence(Qt::Key_F11)
                            || (m->isCheckable() && key.isEmpty()) || m->isSeparator())
                            a->menu()->removeAction(m);
                    }
                }
            }
        }
        // Mục 44: menu chuột phải của hộp chọn file (KDirOperator, hành động
        // PopupMenu — menu dựng lại khi app gọi setupMenu, nên dọn mỗi lần
        // mở): bỏ Sorting + View Mode (trùng nút ⇅ và nút chế độ xem), Reload
        // (F5), Open Containing Folder (user 2026-10-08). Dòng mờ (bấm chỗ
        // trống: Rename, Move to Trash, Properties) ẩn trong lúc menu mở —
        // ẩn lâu thì phím tắt của chúng (F2, Del…) chết theo.
        if (w->inherits("KDirOperator")) {
            for (QAction *popup : w->actions()) {
                QMenu *menu = popup->menu();
                if (!menu || menu->property("_samhie_dirop").toBool()
                    || std::none_of(menu->actions().cbegin(), menu->actions().cend(), [](const QAction *a) {
                           return a->icon().name() == QLatin1String("document-properties");
                       }))
                    continue;
                menu->setProperty("_samhie_dirop", true);
                QObject::connect(menu, &QMenu::aboutToShow, menu, [menu] {
                    static const QSet<QString> dropped = {
                        QStringLiteral("view-sort"), QStringLiteral("view-list-tree"),
                        QStringLiteral("view-refresh"), QStringLiteral("system-file-manager"),
                    };
                    QList<QPointer<QAction>> hidden;
                    for (QAction *a : menu->actions()) {
                        if (dropped.contains(a->icon().name()))
                            menu->removeAction(a);
                        else if (!a->isSeparator() && !a->isEnabled() && a->isVisible()) {
                            a->setVisible(false);
                            hidden.append(a);
                        }
                    }
                    QObject::connect(menu, &QMenu::aboutToHide, menu, [hidden] {
                        for (const QPointer<QAction> &a : hidden) {
                            if (a)
                                a->setVisible(true);
                        }
                    }, Qt::SingleShotConnection);
                });
            }
        }
        if (w->inherits("DolphinStatusBar")) {
            QPalette pal = w->palette();
            pal.setColor(QPalette::Window, QColor(samhie::PANE));
            w->setPalette(pal);
        }
    }
    using QProxyStyle::polish;

    void unpolish(QWidget *w) override
    {
        w->removeEventFilter(&m_pane);
        if (w->property("samhieCard").toBool())
            m_card.detach(w);
        w->removeEventFilter(&m_editorAlpha);
        w->removeEventFilter(&m_popup);
        if (isSearchContents(w))
            w->parentWidget()->removeEventFilter(&m_popup);
        w->removeEventFilter(&m_menuForwarder);
        w->removeEventFilter(&m_urlHover);
        w->removeEventFilter(&m_noPencil);
        w->removeEventFilter(&m_hideDock);
        w->removeEventFilter(&m_popupAnchor);
        w->removeEventFilter(&m_noPaint);
        w->removeEventFilter(&m_followAppPalette);
        w->removeEventFilter(&m_sidebarTab);
        w->removeEventFilter(&m_keepCommandBarFocus);
        w->removeEventFilter(&m_crumbIndent);
        w->removeEventFilter(&m_monoSpans);
        w->removeEventFilter(&m_propsSetup);
        w->removeEventFilter(&m_detailsAlign);
        w->removeEventFilter(&m_commentHeight);
        QProxyStyle::unpolish(w);
    }
    using QProxyStyle::unpolish;

    int pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *w) const override
    {
        if (pm == PM_TextCursorWidth && w && w->property("samhieShortCaret").toBool())
            return 0;  // mục 22: ShortCaret vẽ thay
        // Mục 60: editor Kate (KTextEditor) không chừa khung — KTextEditor
        // đặt lề = độ dày khung quanh vùng code (kateview.cpp 6.30, setupLayout)
        // mà không vẽ khung; 8px của Kvantum để lộ một vòng nền thẻ quanh
        // trái/trên/dưới (đo container: KateViewInternal ở 74,8).
        if (pm == PM_DefaultFrameWidth && w && w->inherits("KTextEditor::ViewPrivate"))
            return 0;
        // Mục 14: lề tiêu đề cột Dolphin (chỉ lời hỏi không kèm gì của Dolphin).
        if (pm == PM_HeaderMargin && !opt && !w
            && QCoreApplication::applicationName() == QLatin1String("dolphin")) {
            const QFontMetricsF fm(QApplication::font());
            // Ngày giờ dài nhất: số 8 rộng nhất trong Dongle; LC_TIME en_GB
            // (samhie.lua). Cộng columnPadding của Dolphin (padding 2 × 6).
            const qreal date = qMax(fm.horizontalAdvance(QStringLiteral("88/88/8888 at 88:88")),
                                    fm.horizontalAdvance(QStringLiteral("Yesterday at 88:88"))) + 12;
            const qreal header = fm.horizontalAdvance(QStringLiteral("Modified"))
                + QProxyStyle::pixelMetric(PM_HeaderGripMargin, opt, w);
            return qMax(QProxyStyle::pixelMetric(pm, opt, w), qCeil((date - header) / 2));
        }
        return QProxyStyle::pixelMetric(pm, opt, w);
    }

    int styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *ret) const override
    {
        if (hint == SH_Menu_SubMenuSloppyCloseTimeout)
            return SUBMENU_SLOPPY_MS;
        if (hint == SH_Menu_SupportsSections)
            return 1;  // mục 78
        return QProxyStyle::styleHint(hint, opt, w, ret);
    }

    // Mục 23. Vạch ở giữa khoảng trống giữa ô icon của hai nút hai bên (nút
    // rộng hẹp khác nhau — vd nút có menu — thì giữa vạch ngăn chưa chắc là
    // giữa hai icon). Không tìm được nút hai bên thì đặt giữa vạch ngăn.
    static void drawToolBarSeparator(const QRect &rect, QPainter *p, const QWidget *w)
    {
        qreal x = rect.center().x();
        if (const QWidget *bar = w->parentWidget()) {
            const QRect me = w->geometry();
            int left = INT_MIN, right = INT_MAX;
            for (const auto *tb : bar->findChildren<QToolButton *>(Qt::FindDirectChildrenOnly)) {
                if (!tb->isVisible())
                    continue;
                const QRect g = tb->geometry();
                const int half = tb->iconSize().width() / 2;
                if (g.right() < me.left())
                    left = qMax(left, g.center().x() + 1 + half);
                else if (g.left() > me.right())
                    right = qMin(right, g.center().x() + 1 - half);
            }
            if (left != INT_MIN && right != INT_MAX)
                x = w->mapFromParent(QPoint((left + right) / 2, 0)).x();
        }
        const int h = qMin(TOOLBAR_SEPARATOR_HEIGHT, rect.height());
        p->fillRect(QRect(int(x), rect.top() + (rect.height() - h) / 2, 1, h), QColor(samhie::FRAME_LINE));
    }

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        if (pe == PE_IndicatorTabTearLeft || pe == PE_IndicatorTabTearRight)
            return;  // mục 63
        // Mục 81: × trên tab đang mở của Konsole (nền terminal, tối cả hai chế
        // độ) — vẽ như phần tử tab-close của Kvantum bản tối (tools/kvantum.py:
        // nét 1,2 trong hộp 16, thường text-muted, trỏ/bấm text).
        if (pe == PE_IndicatorTabClose && w && darkPane(w->parentWidget())) {
            auto *bar = qobject_cast<const QTabBar *>(w->parentWidget());
            const int cur = bar ? bar->currentIndex() : -1;
            if (cur >= 0 && (bar->tabButton(cur, QTabBar::RightSide) == w || bar->tabButton(cur, QTabBar::LeftSide) == w)) {
                const bool hot = opt->state & (State_MouseOver | State_Sunken);
                const QPointF c = QRectF(opt->rect).center();
                p->save();
                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(QPen(QColor(hot ? samhie::TERMINAL_TEXT : samhie::TERMINAL_TEXT_MUTED), 1.2, Qt::SolidLine, Qt::RoundCap));
                p->drawLine(c + QPointF(-3.5, -3.5), c + QPointF(3.5, 3.5));
                p->drawLine(c + QPointF(3.5, -3.5), c + QPointF(-3.5, 3.5));
                p->restore();
                return;
            }
        }
        // Mục 61: đường nhánh cây Projects của Kate. opt->rect là một cột
        // thụt lề (QTreeView::drawBranches, toạ độ vùng hiển thị).
        // Mục 70: nền dòng chọn/trỏ của cây Projects, hết bề ngang. Qt vẽ nền
        // phần thụt lề (PE_PanelItemViewRow, cờ IsDecorationForRootColumn) →
        // đường nhánh → nội dung dòng (QTreeView::drawRow, Qt 6.11.2): tô ở
        // bước đầu thì đường nhánh nằm trên nền; nội dung dòng không tô nữa.
        if (pe == PE_PanelItemViewRow && w && w->property("samhieTreeLines").toBool()) {
            auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
            if (vopt && (vopt->features & QStyleOptionViewItem::IsDecorationForRootColumn)
                && (vopt->state & (State_Selected | State_MouseOver))) {
                const int right = static_cast<const QAbstractScrollArea *>(w)->viewport()->width();
                // Mục 72: cây Git / Symbols — ô hết bề ngang thẻ, vuông
                const bool flush = w->property("samhieRowFlush").toBool();
                const int inset = flush ? 0 : TREE_ROW_INSET;
                const int radius = flush ? 0 : TREE_ROW_RADIUS;
                const QRectF row(inset, vopt->rect.top(), right - 2 * inset, vopt->rect.height());
                p->save();
                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(Qt::NoPen);
                p->setBrush(QColor(vopt->state & State_Selected ? samhie::TREE_ROW_SELECTED : samhie::ROW_HOVER));
                p->drawRoundedRect(row, radius, radius);
                p->restore();
            }
            return;
        }
        if (pe == PE_IndicatorBranch && w && w->property("samhieTreeLines").toBool()) {
            auto *tree = qobject_cast<const QTreeView *>(w);
            const QRect r = opt->rect;
            const int indent = tree ? tree->indentation() : 0;
            const int column = indent > 0 ? (r.left() - tree->columnViewportPosition(0)) / indent : 0;
            if (tree && !(column == 0 && tree->rootIsDecorated())) {
                const int x = r.left() + r.width() / 2;
                const int y = r.top() + r.height() / 2;
                p->save();
                p->setRenderHint(QPainter::Antialiasing, false);
                p->setPen(QColor((opt->state & State_Selected) ? samhie::TREE_LINE_SELECTED : samhie::FRAME_LINE));  // mục 70
                if (opt->state & State_Item) {
                    p->drawLine(x, r.top(), x, (opt->state & State_Sibling) ? r.bottom() : y);  // ├ hoặc └
                    p->drawLine(x, y, r.right(), y);
                } else if (opt->state & State_Sibling) {
                    p->drawLine(x, r.top(), x, r.bottom());  // │ của cấp trên còn mục bên dưới
                }
                p->restore();
            }
            return;
        }
        if (pe == PE_IndicatorToolBarSeparator && w && (opt->state & State_Horizontal)) {
            drawToolBarSeparator(opt->rect, p, w);  // mục 23
            return;
        }
        // Mục 49: khung + nền ô đổi tên = ô nhập thường ôm sát dòng chữ.
        if (isRenameEditor(w) && (pe == PE_Frame || pe == PE_FrameLineEdit || pe == PE_PanelLineEdit)) {
            QStyleOptionFrame f;
            f.QStyleOption::operator=(*opt);
            f.rect = renameBox(w);
            f.state &= ~State_HasFocus;
            f.lineWidth = 1;
            baseStyle()->drawPrimitive(PE_PanelLineEdit, &f, p, nullptr);
            // Vùng chọn: Qt tô cao bằng cả hộp dòng (Highlight của ô đã đặt
            // trong suốt lúc polish) → tô lại ở lớp nền này, cao bằng dải chữ.
            auto *te = static_cast<const QTextEdit *>(w);
            const QTextCursor c = te->textCursor();
            if (c.hasSelection()) {
                const QFontMetricsF fm(te->font());
                const QColor hl = w->property("_samhie_highlight").value<QColor>();
                for (const RenameLine &l : renameLines(te, c.selectionStart(), c.selectionEnd())) {
                    const qreal top = qRound(l.baseline - RENAME_UP * fm.capHeight()) - 2;
                    const qreal bottom = qRound(l.baseline + RENAME_DOWN * fm.capHeight()) + 2;
                    p->fillRect(QRectF(l.x1, top, l.x2 - l.x1, bottom - top), hl);
                }
            }
            return;
        }
        // Mục 42: Kvantum chỉ xét widget để nhận ra "ô sửa tên trong danh
        // sách" → vẽ không kèm widget là ra ô nhập thường.
        if (pe == PE_PanelLineEdit && isFakeItemEditor(w)) {
            QProxyStyle::drawPrimitive(pe, opt, p, nullptr);
            return;
        }
        // Mục 20: tên thư mục / mũi tên không có ô hover (chữ đổi màu thay);
        // nút ✓ có ô vuông ở giữa.
        if (pe == PE_PanelButtonCommand && isUrlNavigatorButton(w)) {
            if (!w->inherits("KDEPrivate::KUrlNavigatorToggleButton"))
                return;
            QStyleOption o(*opt);
            o.rect = centeredSquare(opt->rect, TOGGLE_SIDE);
            QProxyStyle::drawPrimitive(pe, &o, p, w);
            return;
        }
        // Mục 20: ô tìm / ô sửa đường dẫn không viền focus cyan.
        if ((pe == PE_PanelLineEdit || pe == PE_FrameLineEdit) && (opt->state & State_HasFocus)
            && inDolphinLocationOrSearch(w)) {
            QStyleOption o(*opt);
            if (auto *fopt = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
                QStyleOptionFrame f(*fopt);
                f.state &= ~State_HasFocus;
                drawPrimitive(pe, &f, p, w);
            } else {
                o.state &= ~State_HasFocus;
                drawPrimitive(pe, &o, p, w);
            }
            return;
        }
        if (pe == PE_FrameLineEdit && w && w->inherits("KUrlNavigator")) {
            baseStyle()->drawPrimitive(PE_PanelLineEdit, opt, p, w);
            return;
        }
        auto *item = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
        if (pe == PE_PanelItemViewItem && item && isDolphinView(w) && item->rect.width() > w->width() * 0.6) {
            const bool selected = item->state & State_Selected;  // mục 14
            if (!selected && !(item->state & State_MouseOver))
                return;
            // Đổi mép viewport sang toạ độ ô (KItemListWidget) — hiện ô nằm ở
            // x = 0 nên trùng nhau, phòng khi cuộn ngang.
            const QTransform toItem = p->transform().inverted();
            const qreal left = toItem.map(QPointF(0, 0)).x();
            const qreal right = toItem.map(QPointF(w->width(), 0)).x();
            const QRectF box(left, item->rect.top(), right - left, item->rect.height());
            p->fillRect(box, QColor(selected ? samhie::ROW_SELECTED : samhie::ROW_HOVER));
            return;
        }
        if (pe == PE_PanelItemViewItem) {
            // KIO gọi không kèm widget — lấy từ option.
            // Delegate gọi KHÔNG kèm widget; KFilePlacesView::paintEvent gọi KÈM
            // widget để vẽ dấu thả khi kéo file vào sidebar — cái đó giữ nguyên.
            auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
            if (vopt && !w && isPlacesView(vopt->widget)) {
                // Bản sao cục bộ (không const) của KIO — xem chú thích đầu file.
                // Chữ: đang chọn + cửa sổ active → HighlightedText, còn lại →
                // Text. Icon: KIconLoader tô theo palette này (setCustomPalette),
                // chế độ Selected → HighlightedText, chế độ thường → WINDOWTEXT
                // (KIconColors(QPalette), kiconthemes 6.30) — nên đổi cả hai.
                // Tiêu đề nhóm ("Places") cũng lấy WindowText nhưng KIO vẽ nó
                // TRƯỚC lệnh này, bằng chính bản sao này → không bị ảnh hưởng.
                QPalette &pal = const_cast<QStyleOptionViewItem *>(vopt)->palette;
                pal.setColor(QPalette::HighlightedText, QColor(samhie::ACTIVE_TEXT));
                QColor fg;
                if (vopt->state & State_Selected)
                    fg = QColor(samhie::ACTIVE_TEXT);
                else if (vopt->state & State_MouseOver)
                    fg = QColor(samhie::HOVER_TEXT);
                if (fg.isValid()) {
                    pal.setColor(QPalette::Text, fg);
                    pal.setColor(QPalette::WindowText, fg);
                }
                return;  // không tô nền
            }
        }
        // Không vẽ khung focus cho ô file/mục danh sách. Khung "focus" của
        // Kvantum bo 2px đè lên ô bo ITEM_RADIUS thành góc vuông; ô đang chọn
        // đã có viền cyan; còn ô CHƯA chọn thì Dolphin đặt ô đầu tiên làm ô
        // hiện tại mỗi lần vào thư mục → viền lơ lửng ở thư mục đầu (user bỏ,
        // máy ảo 2026-10-04). Mũi tên trong Dolphin chọn luôn ô nên vẫn thấy.
        if (pe == PE_FrameFocusRect && isItemView(w))
            return;
        // Mục 32: ô danh sách trang cách mép phải bằng khe bên trái (setSpacing
        // chỉ chừa trái/trên/dưới — mục dài hết khung bên phải).
        if (pe == PE_PanelItemViewItem && w && w->inherits("KDEPrivate::KPageListView")) {
            if (auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                QStyleOptionViewItem o(*vopt);
                const int right = static_cast<const QAbstractScrollArea *>(w)->viewport()->width() - 1 - PAGE_LIST_GAP;
                o.rect.setRight(qMin(o.rect.right(), right));
                if (o.state & State_Selected) {  // mục 77 (chữ: KDE tự tô HighlightedText)
                    paintPageSelected(p, o.rect);
                    return;
                }
                QProxyStyle::drawPrimitive(pe, &o, p, w);
                return;
            }
        }
        // Mục 30: dải "›" (gọi không kèm widget) không có nền riêng.
        if (pe == PE_PanelItemViewItem && !w && extendableView(opt, w))
            return;
        // Mục 15. Dolphin tô nền ô (drawRoundedRect) rồi gọi PE_Frame — vẽ lại
        // viền trái vùng file ở cột 0 của ô (cùng cột với viền PanePainter).
        // Mục 27: vùng file khép đáy (Terminal mở) thì ô còn đè góc bo dưới-
        // trái + viền đáy → tô lại nền cửa sổ ngoài góc bo, vẽ lại viền vùng
        // file (cùng nét PanePainter, đổi sang toạ độ của ô).
        // Mục 76: popup là QMenu (lịch sử clipboard Kate) — nền thẻ thay nền
        // menu Kvantum (trong như menu, lộ chữ code phía sau), không khung menu
        if (pe == PE_FrameMenu && w && w->property("samhiePopupPanel").toBool())
            return;
        if ((pe == PE_Frame || pe == PE_PanelMenu) && w && w->property("samhiePopupPanel").toBool()) {  // mục 69
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(QPen(QColor(samhie::POPUP_BORDER), 1));
            p->setBrush(QColor(samhie::POPUP));
            p->drawRoundedRect(QRectF(opt->rect).adjusted(0.5, 0.5, -0.5, -0.5), POPUP_RADIUS, POPUP_RADIUS);
            p->restore();
            return;
        }
        if (pe == PE_Frame && w && w->inherits("DolphinStatusBar")) {
            QProxyStyle::drawPrimitive(pe, opt, p, w);
            if (w->layoutDirection() == Qt::LeftToRight) {
                const QWidget *a = w->parentWidget();
                while (a && !a->inherits("DolphinTabWidget"))
                    a = a->parentWidget();
                const auto *tw = static_cast<const QTabWidget *>(a);
                p->save();
                p->setRenderHint(QPainter::Antialiasing);
                if (tw && paneBottomRadius(tw)) {
                    p->translate(-w->mapTo(tw, QPoint(0, 0)));
                    const QRectF all(tw->rect());
                    QPainterPath outside;
                    outside.addRect(all);
                    outside = outside.subtracted(leftOutline(all, PANE_RADIUS, PANE_RADIUS, true));
                    QColor bg(samhie::WINDOW);
                    bg.setAlphaF(samhie::WINDOW_OPACITY);
                    p->setCompositionMode(QPainter::CompositionMode_Source);
                    p->fillPath(outside, bg);
                    p->setCompositionMode(QPainter::CompositionMode_SourceOver);
                    p->setPen(QPen(QColor(samhie::FRAME_LINE), 1));
                    p->setBrush(Qt::NoBrush);
                    p->drawPath(paneBorder(tw));
                } else {
                    p->setRenderHint(QPainter::Antialiasing, false);
                    p->setPen(QPen(QColor(samhie::FRAME_LINE), 1));
                    p->drawLine(QPointF(0.5, 0), QPointF(0.5, w->height()));
                }
                p->restore();
            }
            return;
        }
        // Vạch giữa sidebar và vùng file: viền của vùng file (PanePainter) thay.
        if (pe == PE_IndicatorDockWidgetResizeHandle)
            return;
        QProxyStyle::drawPrimitive(pe, opt, p, w);
    }

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w) const override
    {
        // Mục 21: ô hover vuông ở giữa nút (icon giữ nguyên chỗ: Kvantum đặt
        // icon giữa rect, rect mới cùng tâm).
        auto *tbopt = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
        if (cc == CC_ToolButton && tbopt) {
            QStyleOptionToolButton o(*tbopt);
            // Mục 28: không khung focus cyan; đi bằng Tab → nền hover. Kvantum
            // hạ "hover" về thường khi chuột không nằm trên nút (chống lỗi
            // hover kẹt của Qt, PE_PanelButtonTool) — chỉ khi option có
            // styleObject, nên bỏ nó đi (styleObject chỉ dùng cho hiệu ứng).
            if (o.state & State_HasFocus) {
                o.state &= ~State_HasFocus;
                if (o.state & State_KeyboardFocusChange) {
                    o.state |= State_MouseOver;
                    o.styleObject = nullptr;
                }
            }
            // Mục 72: nút Commit của panel Git — ô hover như tên dự án (mục 59):
            // vuông, hết ô nút. Kvantum chỉ vẽ ô vuông (expansion 0) cho nút
            // KHÔNG chữ — vẽ nền bằng option không chữ, widget là cha (nút
            // có chữ thì Kvantum đọc chữ từ chính nút), rồi vẽ nút không hover.
            if (w && w->property("samhieFlatHover").toBool()) {
                if ((o.state & State_Enabled) && (o.state & (State_MouseOver | State_Sunken | State_On))) {
                    QStyleOptionToolButton panel;
                    panel.QStyleOption::operator=(o);
                    panel.styleObject = nullptr;
                    panel.state = (o.state & (State_Enabled | State_MouseOver | State_Sunken | State_On)) | State_AutoRaise | State_Raised;
                    proxy()->drawPrimitive(PE_PanelButtonTool, &panel, p, w->parentWidget());
                }
                o.state &= ~(State_MouseOver | State_Sunken | State_On);
                QProxyStyle::drawComplexControl(cc, &o, p, w);
                return;
            }
            if (w && tbopt->toolButtonStyle == Qt::ToolButtonIconOnly
                && (isTabCornerButton(w) || w->property("samhieSquareHover").toBool()  // mục 68
                    || (inDolphinLocationOrSearch(w) && !qobject_cast<QLineEdit *>(w->parentWidget()))
                    || (qobject_cast<QToolBar *>(w->parentWidget()) && w->window()->inherits("DolphinMainWindow")))) {
                const int side = qMax(o.iconSize.width(), o.iconSize.height()) + 2 * TOOL_PADDING;
                o.rect = centeredSquare(o.rect, side);
            }
            QProxyStyle::drawComplexControl(cc, &o, p, w);
            return;
        }
        // Mục 83: thanh cuộn trên thẻ tối ở bản sáng mang màu bản tối.
        if (cc == CC_ScrollBar && g_light && darkPane(w)) {
            paintScrollbarDarkened(p, opt->rect, [&](QPainter *ip) { QProxyStyle::drawComplexControl(cc, opt, ip, w); });
            return;
        }
        // Mục 33: thanh trượt không khung focus, không sáng khi trỏ.
        if (cc == CC_Slider) {
            if (auto *sopt = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
                QStyleOptionSlider o(*sopt);
                o.state &= ~(State_HasFocus | State_MouseOver);
                if (!(o.state & State_Sunken))  // đang kéo thì núm vẫn sáng
                    o.activeSubControls = SC_None;
                QProxyStyle::drawComplexControl(cc, &o, p, w);
                return;
            }
        }
        // Mục 59: ô chọn dự án của Kate — tên phẳng, ▾ khi có từ 2 dự án
        if (cc == CC_ComboBox && w && w->property("samhieFlatCombo").toBool()) {
            if (auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                if (cb->state & (State_MouseOver | State_On)) {
                    QStyleOptionToolButton panel;
                    panel.QStyleOption::operator=(*cb);
                    panel.state = (cb->state & (State_Enabled | State_MouseOver | State_On)) | State_AutoRaise | State_Raised;
                    proxy()->drawPrimitive(PE_PanelButtonTool, &panel, p, w);
                }
                auto *combo = qobject_cast<const QComboBox *>(w);
                if (combo && combo->count() >= 2) {
                    QStyleOption arrow(*cb);
                    arrow.rect = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxArrow, w);
                    arrow.state &= ~(State_MouseOver | State_On | State_Sunken);
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrow, p, w);
                }
                return;
            }
        }
        // Mục 20: ô sửa đường dẫn không viền focus cyan. Kvantum (CC_ComboBox,
        // combo_as_lineedit) hỏi thẳng cb->hasFocus() chứ không chỉ đọc state
        // → bỏ cờ focus VÀ không đưa widget (chỉ dùng để hỏi focus / ô nhập
        // trong suốt — ô của KUrlComboBox đục, cùng nhánh khi không có widget).
        if (cc == CC_ComboBox && w && w->parentWidget()
            && (w->parentWidget()->inherits("KUrlNavigator") || inFilterBar(w))) {
            QStyleOptionComboBox o(*qstyleoption_cast<const QStyleOptionComboBox *>(opt));
            o.state &= ~State_HasFocus;
            // Dropdown thanh lọc đang mở danh sách: Kvantum vẽ "đang bấm" bằng
            // viền cyan như focus (máy ảo 2026-10-06) → vẽ như thường.
            if (inFilterBar(w))
                o.state &= ~(State_On | State_Sunken);
            QProxyStyle::drawComplexControl(cc, &o, p, nullptr);
            return;
        }
        QProxyStyle::drawComplexControl(cc, opt, p, w);
    }
    using QProxyStyle::drawComplexControl;

    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        // Mục 78: tiêu đề nhóm trong menu — như dòng thường bị tắt
        if (ce == CE_MenuItem) {
            if (auto *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
                m && m->menuItemType == QStyleOptionMenuItem::Separator && !m->text.isEmpty()) {
                const QStyleOptionMenuItem title = sectionAsItem(m);
                drawControl(ce, &title, p, w);
                return;
            }
        }
        // Mục 60: tay nắm splitter của Kate là khe giữa hai thẻ — không vạch
        if (ce == CE_Splitter && w && w->window()->inherits("KateMainWindow"))
            return;
        // Mục 53: "&" trong chữ ô chọn là chữ thật (QComboBox không có phím
        // tắt trong chữ) — Kvantum vẽ như nhãn nút, "&" thành gạch phím tắt.
        if (ce == CE_ComboBoxLabel && w && w->property("samhieFlatCombo").toBool()) {
            if (auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                // Mục 67: chữ bắt đầu thẳng icon thư mục cấp gốc của cây (plugin
                // Kate mục 13 đo, ghi samhieTextIndent) — tự vẽ vì Kvantum đặt
                // chữ theo lề riêng của nó, không đọc lại được.
                const QVariant indent = w->property("samhieTextIndent");
                if (indent.isValid()) {
                    QRect r = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, w);
                    r.setLeft(cb->rect.left() + indent.toInt());
                    // drawItemText không đổi "&" thành gạch phím tắt (không cờ mnemonic)
                    const QString text = cb->fontMetrics.elidedText(cb->currentText, Qt::ElideRight, r.width());
                    proxy()->drawItemText(p, r, Qt::AlignLeft | Qt::AlignVCenter, cb->palette, cb->state & State_Enabled, text,
                                          QPalette::ButtonText);
                    return;
                }
                QStyleOptionComboBox o(*cb);  // mục 59: không icon
                o.currentIcon = QIcon();
                o.iconSize = QSize();
                o.currentText.replace(QLatin1Char('&'), QLatin1String("&&"));
                QProxyStyle::drawControl(ce, &o, p, w);
                return;
            }
        }
        if (ce == CE_ComboBoxLabel) {
            if (auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt);
                cb && cb->currentText.contains(QLatin1Char('&'))) {
                QStyleOptionComboBox o(*cb);
                o.currentText.replace(QLatin1Char('&'), QLatin1String("&&"));
                QProxyStyle::drawControl(ce, &o, p, w);
                return;
            }
        }
        // Mục 47, 50: icon trong menu có icon (enableMenuIcons), vẽ vào
        // khoảng chữ đã lùi. Chữ bắt đầu ở mép ô + khung 4 + lề chữ 4
        // (tools/kvantum.py MenuItem) + cột ô tích nếu menu có ô tích (Kvantum
        // CE_MenuItem: min(16, PM_IndicatorWidth) + PM_CheckBoxLabelSpacing).
        if (ce == CE_MenuItem && w && w->property("_samhie_menu_icons").toBool()) {
            auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
            QProxyStyle::drawControl(ce, opt, p, w);
            QAction *action = mi && mi->menuItemType == QStyleOptionMenuItem::Normal
                ? static_cast<const QMenu *>(w)->actionAt(mi->rect.center())
                : nullptr;
            if (!action || !action->property("_samhie_indented").toBool())
                return;
            const int check = (mi->menuHasCheckableItems || mi->checkType != QStyleOptionMenuItem::NotCheckable)
                ? qMin(16, baseStyle()->pixelMetric(PM_IndicatorWidth, opt, w))
                    + baseStyle()->pixelMetric(PM_CheckBoxLabelSpacing)
                : 0;
            QRect icon(0, 0, SYMBOL_MENU_ICON, SYMBOL_MENU_ICON);
            icon.moveCenter(mi->rect.center());
            icon.moveLeft(mi->direction == Qt::RightToLeft ? mi->rect.right() - 8 - check - SYMBOL_MENU_ICON + 1
                                                           : mi->rect.left() + 8 + check);
            action->icon().paint(p, icon, Qt::AlignCenter, (mi->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled);
            return;
        }
        // Mục 21: dòng widget trong menu (dòng Zoom của Dolphin tự vẽ nền
        // bằng CE_MenuItem, state lấy từ chính nó) không vẽ trạng thái trỏ.
        // Trỏ vào nút ⊖ ⟲ ⊕ thì dòng vẽ lại ĐÚNG ô dưới nút, đang "trỏ" →
        // nền hover dòng menu; chuột rời đi dòng không vẽ lại → ô #262626
        // vuông nằm lại dưới nút (container 2026-10-06; ảnh user: "như có
        // tận 2 ô hover"). Dòng này vốn không sáng khi trỏ vào chữ Zoom.
        if (ce == CE_MenuItem && w && !qobject_cast<const QMenu *>(w)
            && qobject_cast<const QMenu *>(w->parentWidget())) {
            if (auto *mopt = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
                QStyleOptionMenuItem o(*mopt);
                o.state &= ~(State_MouseOver | State_Selected);
                QProxyStyle::drawControl(ce, &o, p, w);
                return;
            }
        }
        // Mục 30: ô danh sách không khung focus; ô của KExtendableItemDelegate
        // vẽ nền một lần trên cả ô (gồm dải "›"), đứng riêng.
        if (ce == CE_ItemViewItem) {
            if (auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                QStyleOptionViewItem o(*vopt);
                o.state &= ~State_HasFocus;
                if (noCheck(vopt, w))  // mục 71
                    o.features &= ~QStyleOptionViewItem::HasCheckIndicator;
                // Mục 73: breadcrumb Kate không ô hover/chọn — trỏ vào thì chữ
                // cyan như thanh đường dẫn Dolphin (dấu › Kate tự bỏ trạng thái trỏ)
                // Mục 82: thẻ code tối cả hai chế độ → bản sáng chữ + icon bản tối
                // (icon: bản Selected của bộ icon = nét sáng).
                if (w && w->inherits("BreadCrumbView")) {
                    const bool dark = g_light && darkPane(w);
                    if (dark) {
                        o.palette.setColor(QPalette::Text, QColor(samhie::TERMINAL_TEXT));
                        if (!o.icon.isNull())
                            o.icon = QIcon(o.icon.pixmap(o.decorationSize, p->device() ? p->device()->devicePixelRatioF() : 1.0, QIcon::Selected));
                    }
                    if (o.state & State_MouseOver)
                        o.palette.setColor(QPalette::Text, QColor(dark ? samhie::TERMINAL_ACCENT : samhie::HOVER_ACCENT));
                    o.state &= ~(State_Selected | State_MouseOver);
                }
                if (w && w->inherits("KateColorTreeWidget") && !HoverUnderRow::s_panel)  // mục 86: nền đã tô ở HoverUnderRow
                    o.state &= ~(State_Selected | State_MouseOver);
                // Mục 83: dòng kết quả Search Kate (chữ theme code, luôn sáng)
                if (g_light && (o.state & (State_Selected | State_MouseOver)) && darkPane(w)) {
                    paintDarkened(p, o.rect, [&](QPainter *ip) { QProxyStyle::drawPrimitive(PE_PanelItemViewItem, &o, ip, w); });
                    o.state &= ~(State_Selected | State_MouseOver);
                }
                if (w && w->property("samhieTreeLines").toBool()) {  // mục 70: nền ở PE_PanelItemViewRow
                    o.state &= ~(State_Selected | State_MouseOver);
                    QProxyStyle::drawControl(ce, &o, p, w);
                    return;
                }
                // Mục 77: danh sách trang không delegate riêng (Configure Kate:
                // FlatList; dạng cây) — Kvantum vẽ ô trong drawControl của nó
                // (không qua proxy) → tô ô ở đây, đưa xuống như ô thường nhưng
                // giữ chữ sáng của trang đang chọn.
                if (w && (w->inherits("KDEPrivate::KPageListView") || w->inherits("KDEPrivate::KPageTreeView"))
                    && (o.state & State_Selected)) {
                    paintPageSelected(p, o.rect);
                    o.state &= ~(State_Selected | State_MouseOver);
                    o.palette.setColor(QPalette::Text, o.palette.color(QPalette::HighlightedText));
                    QProxyStyle::drawControl(ce, &o, p, w);
                    return;
                }
                // Mục 79: danh sách nằm trong menu (danh sách chế độ — lớp
                // KateModeMenuList không Q_OBJECT; lúc polish chưa vào menu) →
                // nhận lúc vẽ
                if (w && (w->property("samhiePopupList").toBool()
                          || (qobject_cast<const QListView *>(w) && qobject_cast<const QMenu *>(w->window())))) {  // mục 69
                    if (o.state & (State_Selected | State_MouseOver))
                        paintPopupItem(p, o.rect, o.state & State_Selected, o.viewItemPosition);
                    o.state &= ~(State_Selected | State_MouseOver);
                    o.viewItemPosition = QStyleOptionViewItem::OnlyOne;
                    QProxyStyle::drawControl(ce, &o, p, w);
                    return;
                }
                const QAbstractItemView *view = extendableView(opt, w);
                if (!view && isSettingsList(w) && o.index.isValid()) {  // mục 37
                    QStyleOptionViewItem panel(o);
                    panel.viewItemPosition = QStyleOptionViewItem::OnlyOne;
                    panel.rect.adjust(PAGE_LIST_GAP, PAGE_LIST_GAP / 2, -PAGE_LIST_GAP, -PAGE_LIST_GAP / 2);
                    // Danh sách tích chọn: "dòng đang chọn" vô nghĩa → không
                    // tô chọn. Danh sách chọn (thùng rác của từng ổ, trang
                    // Trash) thì tô như danh sách trang (mục 77, không viền).
                    if (o.features & QStyleOptionViewItem::HasCheckIndicator)
                        panel.state &= ~State_Selected;
                    if (panel.state & State_Selected)
                        paintPageSelected(p, panel.rect);
                    else
                        QProxyStyle::drawPrimitive(PE_PanelItemViewItem, &panel, p, w);
                    o.state &= ~(State_Selected | State_MouseOver);
                    o.viewItemPosition = QStyleOptionViewItem::OnlyOne;
                    // Nội dung lùi theo ô: chữ không dính mép trái ô (dòng chỉ
                    // có chữ, Trash — Kvantum để lề chữ ItemView = 0).
                    o.rect.adjust(2 * PAGE_LIST_GAP, 0, -2 * PAGE_LIST_GAP, 0);
                } else if (view && o.index.isValid()) {
                    QStyleOptionViewItem panel(o);
                    panel.viewItemPosition = QStyleOptionViewItem::OnlyOne;
                    panel.rect.setLeft(view->visualRect(o.index).left());
                    // Ô đang chọn vẽ như ô hover, không viền: bấm ô Action thì
                    // ô đó được chọn một khoảnh rồi KDE chuyển sang ô Shortcut
                    // (mở dải sửa) → viền chọn nháy (máy ảo user 2026-10-07).
                    // Dải sửa mở ngay dưới đã cho biết dòng nào. Không kèm
                    // widget: Kvantum hạ "hover" khi chuột ra ngoài danh sách.
                    const bool selected = panel.state & State_Selected;
                    if (selected) {
                        panel.state &= ~State_Selected;
                        panel.state |= State_MouseOver;
                    }
                    QProxyStyle::drawPrimitive(PE_PanelItemViewItem, &panel, p, selected ? nullptr : w);
                    o.state &= ~(State_Selected | State_MouseOver);
                    o.viewItemPosition = QStyleOptionViewItem::OnlyOne;
                }
                QProxyStyle::drawControl(ce, &o, p, w);
                return;
            }
        }
        // Tab đang mở (dải tab kiểu document: Konsole, Dolphin) trong như vùng
        // nội dung bên dưới: card với độ đục cửa sổ, tô ĐÈ (Source) lên nền
        // cửa sổ — Kvantum tô chồng (SourceOver) thì thành ~99% đục (máy ảo
        // 2026-10-04: tab #171717 đục, vùng nội dung thấy hình nền). Cùng dáng
        // phần tử "tab" của Kvantum: khe trên TAB_INSET, bo trên TAB_RADIUS.
        auto *tab = qstyleoption_cast<const QStyleOptionTab *>(opt);
        auto *bar = qobject_cast<const QTabBar *>(w);
        // Mục 28: tab không khung focus (hai sọc cyan quanh chữ tab đang mở).
        if ((ce == CE_TabBarTab || ce == CE_TabBarTabLabel) && tab && (tab->state & State_HasFocus)) {
            QStyleOptionTab o(*tab);
            o.state &= ~State_HasFocus;
            drawControl(ce, &o, p, w);
            return;
        }
        if (ce == CE_TabBarTabShape && tab && bar && bar->documentMode()
            && tab->shape == QTabBar::RoundedNorth && (tab->state & State_Selected)) {
            const QRectF r = QRectF(tab->rect).adjusted(0, samhie::TAB_INSET, 0, 0);
            const qreal rad = samhie::TAB_RADIUS, d = 2 * rad;
            QPainterPath path;
            path.moveTo(r.left(), r.bottom() + 1);
            path.lineTo(r.left(), r.top() + rad);
            path.arcTo(QRectF(r.left(), r.top(), d, d), 180, -90);
            path.lineTo(r.right() + 1 - rad, r.top());
            path.arcTo(QRectF(r.right() + 1 - d, r.top(), d, d), 90, -90);
            path.lineTo(r.right() + 1, r.bottom() + 1);
            path.closeSubpath();
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            p->setCompositionMode(QPainter::CompositionMode_Source);
            p->fillPath(path, paneColor(w));
            p->restore();
            return;
        }
        // Mục 85: tab thường có icon + chữ (Configure Dolphin › View: General,
        // Content Display…) — Kvantum đặt icon sát mép trái tab, chữ canh giữa
        // phần còn lại → tab dàn hết bề ngang thì icon trôi xa chữ, như thuộc
        // tab bên cạnh. Thu khung nhãn về bề rộng nội dung, đặt giữa tab.
        // Chỉ tab bị giãn (dư hơn TAB_LABEL_SLACK) và không có nút × — tab
        // Kate/Konsole vừa khít nội dung, có ×.
        if (ce == CE_TabBarTabLabel && tab && !tab->icon.isNull() && !tab->text.isEmpty()
            && tab->rightButtonSize.isEmpty() && tab->leftButtonSize.isEmpty()
            && (tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::RoundedSouth)) {
            QString text = tab->text;
            text.replace(QLatin1String("&&"), QLatin1String("\x01")).remove(QLatin1Char('&')).replace(QLatin1Char('\x01'), QLatin1Char('&'));
            const int content = tab->iconSize.width() + TAB_LABEL_GAP + tab->fontMetrics.horizontalAdvance(text) + 2 * TAB_LABEL_PAD;
            if (tab->rect.width() > content + TAB_LABEL_SLACK) {
                QStyleOptionTab o(*tab);
                o.rect = QRect(tab->rect.center().x() + 1 - content / 2, tab->rect.y(), content, tab->rect.height());
                drawControl(ce, &o, p, w);
                return;
            }
        }
        // Mục 81: chữ tab đang mở của Konsole nằm trên nền terminal (tối cả
        // hai chế độ) → chữ bản tối. Kvantum lấy màu chữ tab từ kvconfig, bỏ
        // qua palette → Kvantum vẽ nhãn (icon + chữ, đúng bố cục của nó) vào
        // ảnh tạm, rồi tô cả nhãn một màu chữ bản tối. Mục 82: tab Kate có icon
        // file — icon Samhie là nét một màu nên tô chung được.
        if (ce == CE_TabBarTabLabel && tab && (tab->state & State_Selected) && darkPane(w)) {
            const qreal dpr = p->device() ? p->device()->devicePixelRatioF() : 1.0;
            QImage label(tab->rect.size() * dpr, QImage::Format_ARGB32_Premultiplied);
            label.setDevicePixelRatio(dpr);
            label.fill(Qt::transparent);
            {
                QPainter ip(&label);
                ip.setFont(p->font());
                ip.translate(-tab->rect.topLeft());
                QProxyStyle::drawControl(ce, tab, &ip, w);
                ip.resetTransform();
                ip.setCompositionMode(QPainter::CompositionMode_SourceIn);
                ip.fillRect(QRectF(QPointF(0, 0), QSizeF(tab->rect.size())), QColor(samhie::TERMINAL_TEXT));
            }
            p->drawImage(tab->rect.topLeft(), label);
            return;
        }
        auto *button = qstyleoption_cast<const QStyleOptionButton *>(opt);
        // Mục 83: nút trên vùng tối (⤢ tách kết quả Search Kate, ô màu "None
        // set" của Theme Editor) — mặt + viền bản tối; nhãn (chữ + icon nét
        // một màu) Kvantum vẽ vào ảnh tạm rồi tô một màu chữ bản tối — Kvantum
        // lấy màu chữ nút từ kvconfig, bỏ qua palette (đo container: chữ mất).
        // Ô "None set" Kate vẽ không cờ Enabled (bản sáng Kvantum làm mờ chữ:
        // đo 99 so với 189 bản tối) → nhãn vẽ như đang bật; nút THẬT bị tắt thì
        // tô màu chữ mờ. Lót nền thẻ chỉ cho nút thật (⤢ nằm đè lên kết quả).
        if (ce == CE_PushButton && button && g_light && darkPane(w)) {
            QStyleOptionButton o(*button);
            o.state &= ~State_HasFocus;
            const bool real = qobject_cast<const QAbstractButton *>(w);
            paintDarkened(p, o.rect, [&](QPainter *ip) { QProxyStyle::drawControl(CE_PushButtonBevel, &o, ip, w); },
                          real ? paneColor(w) : QColor());
            const bool disabled = real && !(o.state & State_Enabled);
            o.state |= State_Enabled;
            const qreal dpr = p->device() ? p->device()->devicePixelRatioF() : 1.0;
            QImage label(o.rect.size() * dpr, QImage::Format_ARGB32_Premultiplied);
            label.setDevicePixelRatio(dpr);
            label.fill(Qt::transparent);
            {
                QPainter ip(&label);
                ip.setFont(p->font());
                ip.translate(-o.rect.topLeft());
                o.rect = subElementRect(SE_PushButtonContents, button, w);
                QProxyStyle::drawControl(CE_PushButtonLabel, &o, &ip, w);
                ip.resetTransform();
                ip.setCompositionMode(QPainter::CompositionMode_SourceIn);
                ip.fillRect(QRectF(QPointF(0, 0), QSizeF(button->rect.size())),
                            QColor(disabled ? samhie::TERMINAL_TEXT_MUTED : samhie::TERMINAL_TEXT));
            }
            p->drawImage(button->rect.topLeft(), label);
            return;
        }
        if (ce == CE_PushButton && button && (button->state & State_HasFocus)) {  // mục 16, 28
            QStyleOptionButton o(*button);
            o.state &= ~State_HasFocus;
            if (o.state & State_KeyboardFocusChange) {
                o.state |= State_MouseOver;
                o.styleObject = nullptr;  // xem CC_ToolButton: Kvantum hạ hover khi chuột ở ngoài
            }
            QProxyStyle::drawControl(ce, &o, p, w);
            return;
        }
        // Mục 25: radio/checkbox không có khung focus cyan quanh chữ. Bấm chuột
        // thì thôi hẳn; đi bằng Tab (State_KeyboardFocusChange) thì ô đang
        // đứng có nền như ô hover — không thì bấm Space không biết chọn ô nào.
        // Mục 28: radio/checkbox vẽ lùi vào KEY_FOCUS_PAD (Kvantum tự tính
        // vị trí vòng + chữ từ rect, không hỏi lại style này).
        if ((ce == CE_RadioButton || ce == CE_CheckBox) && button
            && ((button->state & State_HasFocus) || isToggle(w))) {
            QStyleOptionButton o(*button);
            if (isToggle(w))
                o.rect.setLeft(o.rect.left() + KEY_FOCUS_PAD);
            o.state &= ~State_HasFocus;
            if ((button->state & State_HasFocus) && (button->state & State_KeyboardFocusChange)) {
                o.state |= State_MouseOver;  // vòng/ô vuông sáng lên như lúc trỏ chuột
                const bool radio = ce == CE_RadioButton;
                const QRect ind = QProxyStyle::subElementRect(radio ? SE_RadioButtonIndicator : SE_CheckBoxIndicator, &o, w);
                const QRect txt = QProxyStyle::subElementRect(radio ? SE_RadioButtonFocusRect : SE_CheckBoxFocusRect, &o, w);
                const QRect box = ind.united(txt).adjusted(-KEY_FOCUS_PAD, -3, KEY_FOCUS_PAD, 3) & button->rect;
                const bool onCard = w && w->window() && w->window()->inherits("QMenu");
                p->save();
                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(Qt::NoPen);
                p->setBrush(QColor(onCard ? samhie::KEY_FOCUS_FILL : samhie::KEY_FOCUS_FILL_RAISED));
                p->drawRoundedRect(QRectF(box), samhie::MENUITEM_RADIUS, samhie::MENUITEM_RADIUS);
                p->restore();
            }
            QProxyStyle::drawControl(ce, &o, p, w);
            return;
        }
        auto *header = qstyleoption_cast<const QStyleOptionHeader *>(opt);
        // Mục 83: tiêu đề cột trên vùng tối (bảng kiểu chữ Theme Editor Kate) —
        // nền ô như ô dòng; chữ + icon (B I U S, nét một màu) Kvantum vẽ vào
        // ảnh tạm rồi tô một màu chữ bản tối (như nhãn tab, mục 81)
        if (ce == CE_Header && header && g_light && darkPane(w)) {
            paintDarkened(p, header->rect, [&](QPainter *ip) { QProxyStyle::drawControl(CE_HeaderSection, header, ip, w); });
            const qreal dpr = p->device() ? p->device()->devicePixelRatioF() : 1.0;
            QImage label(header->rect.size() * dpr, QImage::Format_ARGB32_Premultiplied);
            label.setDevicePixelRatio(dpr);
            label.fill(Qt::transparent);
            {
                QPainter ip(&label);
                ip.setFont(p->font());
                ip.translate(-header->rect.topLeft());
                QStyleOptionHeader o(*header);
                o.rect = subElementRect(SE_HeaderLabel, header, w);
                QProxyStyle::drawControl(CE_HeaderLabel, &o, &ip, w);
                if (header->sortIndicator != QStyleOptionHeader::None) {
                    o.rect = subElementRect(SE_HeaderArrow, header, w);
                    QProxyStyle::drawPrimitive(PE_IndicatorHeaderArrow, &o, &ip, w);
                }
                ip.resetTransform();
                ip.setCompositionMode(QPainter::CompositionMode_SourceIn);
                ip.fillRect(QRectF(QPointF(0, 0), QSizeF(header->rect.size())), QColor(samhie::TERMINAL_TEXT));
            }
            p->drawImage(header->rect.topLeft(), label);
            return;
        }
        if (ce == CE_Header && header && isDolphinView(w)) {  // mục 14
            p->save();
            p->setCompositionMode(QPainter::CompositionMode_Source);
            p->fillRect(header->rect, paneColor());
            p->restore();
            QStyleOptionHeader o(*header);
            o.rect.setBottom(o.rect.bottom() - HEADER_GAP);
            const bool padding = o.text.isEmpty() && o.sortIndicator == QStyleOptionHeader::None;
            if (padding) {
                if (o.position == QStyleOptionHeader::End)
                    m_headerEnd = o.rect;
            } else {
                if (o.state & State_MouseOver) {
                    QStyleOptionHeader section(o);
                    if (o.section == 0 && w->layoutDirection() == Qt::LeftToRight)
                        section.rect.setLeft(qMin(o.rect.left(), 0));
                    if (m_headerEnd.isValid() && qAbs(m_headerEnd.left() - o.rect.right() - 1) <= 1)
                        section.rect.setRight(m_headerEnd.right());
                    if (section.rect != o.rect) {
                        QProxyStyle::drawControl(CE_HeaderSection, &section, p, w);
                        // Không vẽ ô hover/ô đang bấm thứ hai chồng lên phần
                        // cũ (bấm thì phần cũ đậm hơn phần kéo dài — máy ảo).
                        o.state &= ~(State_MouseOver | State_Sunken);
                    }
                }
                if (o.text == QLatin1String("Size") || o.text == QLatin1String("Permissions"))
                    o.textAlignment = Qt::AlignRight | Qt::AlignVCenter;
                m_headerEnd = QRect();
            }
            QProxyStyle::drawControl(ce, &o, p, w);
            return;
        }
        QProxyStyle::drawControl(ce, opt, p, w);
    }

    QRect subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w) const override
    {
        if (se == SE_LineEditContents && isFakeItemEditor(w))  // mục 42: lề chữ như ô nhập thường
            return QProxyStyle::subElementRect(se, opt, nullptr);
        // Mục 28: vùng bấm/vòng/chữ của radio/checkbox theo chỗ đã lùi.
        if (isToggle(w) && qstyleoption_cast<const QStyleOptionButton *>(opt)
            && (se == SE_RadioButtonIndicator || se == SE_RadioButtonContents || se == SE_RadioButtonFocusRect
                || se == SE_RadioButtonClickRect || se == SE_CheckBoxIndicator || se == SE_CheckBoxContents
                || se == SE_CheckBoxFocusRect || se == SE_CheckBoxClickRect)) {
            QStyleOptionButton o(*qstyleoption_cast<const QStyleOptionButton *>(opt));
            o.rect.setLeft(o.rect.left() + KEY_FOCUS_PAD);
            return QProxyStyle::subElementRect(se, &o, w);
        }
        // Mục 37: nội dung ô danh sách cài đặt vẽ lùi 2 × PAGE_LIST_GAP mỗi
        // bên → ô tích / icon / chữ tính theo chỗ đã lùi. Delegate dò cú bấm
        // vào ô tích bằng SE_ItemViewItemCheckIndicator: tính theo chỗ cũ thì
        // vùng bấm lệch trái 8px so với ô vẽ ra — bấm giữa ô không ăn, phải
        // bấm mép trái (trang Context Menu, user 2026-10-07; đo container).
        if ((se == SE_ItemViewItemCheckIndicator || se == SE_ItemViewItemDecoration
             || se == SE_ItemViewItemText || se == SE_ItemViewItemFocusRect)
            && isSettingsList(w) && !extendableView(opt, w)) {
            if (auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt); vopt && vopt->index.isValid()) {
                QStyleOptionViewItem o(*vopt);
                o.rect.adjust(2 * PAGE_LIST_GAP, 0, -2 * PAGE_LIST_GAP, 0);
                return QProxyStyle::subElementRect(se, &o, w);
            }
        }
        if (se == SE_ItemViewItemCheckIndicator || se == SE_ItemViewItemDecoration || se == SE_ItemViewItemText
            || se == SE_ItemViewItemFocusRect) {
            auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
            if (vopt && noCheck(vopt, w)) {  // mục 71
                QStyleOptionViewItem o(*vopt);
                o.features &= ~QStyleOptionViewItem::HasCheckIndicator;
                return QProxyStyle::subElementRect(se, &o, w);
            }
        }
        QRect r = QProxyStyle::subElementRect(se, opt, w);
        // Ô góc (nút + của Konsole) cao bằng dải tab, nút tự căn icon giữa.
        auto *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt);
        auto *tw = qobject_cast<const QTabWidget *>(w);
        if ((se == SE_TabWidgetLeftCorner || se == SE_TabWidgetRightCorner) && twf && tw
            && tw->documentMode() && tw->tabPosition() == QTabWidget::North && r.isValid()) {
            r.setTop(0);
            r.setHeight(qMax(r.height(), twf->tabBarSize.height()));
        }
        // Dolphin: dải tab + nút + lùi vào trong viền khung (mục 9 đầu file).
        if ((se == SE_TabWidgetLeftCorner || se == SE_TabWidgetTabBar) && tw
            && tw->inherits("DolphinTabWidget") && tw->tabPosition() == QTabWidget::North
            && r.isValid()) {
            r.translate(DOLPHIN_TAB_INSET, 0);
            if (se == SE_TabWidgetTabBar)
                r.translate(NEW_TAB_GAP, 0);  // tab đầu không dính ô hover nút +
            if (se == SE_TabWidgetLeftCorner)
                r.setTop(DOLPHIN_TAB_INSET);  // nền khi trỏ ngang đỉnh tab
            r.setRight(qMin(r.right(), tw->width() - 1));
        }
        return r;
    }

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &size, const QWidget *w) const override
    {
        // Mục 78: dòng tiêu đề nhóm cao như dòng thường
        if (ct == CT_MenuItem) {
            if (auto *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
                m && m->menuItemType == QStyleOptionMenuItem::Separator && !m->text.isEmpty()) {
                const QStyleOptionMenuItem title = sectionAsItem(m);
                return sizeFromContents(ct, &title, size, w);
            }
        }
        // Mục 42: cao như ô nhập thường. Không kèm widget thì Kvantum nâng lên
        // cỡ mặc định của Qt (nội dung + khung — cao 56 thay vì 41, đo
        // container) → đưa nội dung cao 0: chiều cao chỉ còn theo font + khung
        // như khi có widget (ô nhập thường không dùng chiều cao nội dung).
        if (ct == CT_LineEdit && isFakeItemEditor(w))
            return QProxyStyle::sizeFromContents(ct, opt, QSize(size.width(), 0), nullptr);
        // Mục 66: KMultiTabBarTab lấy lề quanh icon từ đây (computeMargins:
        // lề = (kết quả − icon)/2 mỗi chiều), rồi nút cột dọc cao = icon + 2·lề
        // dọc, rộng = max(icon, cao dòng chữ RỖNG) + 2·lề ngang
        // (computeSizeHint, KWidgetsAddons 6.30). Dòng Dongle cao hơn icon
        // 22 (đo container: cột rộng 66 thay vì 48) → lề ngang bù phần đó.
        // Nút chỉ-có-icon chỉ nằm ở cột trái/phải (đáy Kate luôn có chữ).
        if (ct == CT_ToolButton && isIconOnlySidebarTab(w)) {
            const int icon = qMax(size.width(), size.height());
            const int line = qMax(icon, w->fontMetrics().size(0, QString()).height());
            const int side = qMax(SIDEBAR_TAB, icon + 2 * TOOL_PADDING);
            return QSize(icon + qMax(0, side - line), side);
        }
        QSize s = QProxyStyle::sizeFromContents(ct, opt, size, w);
        auto *bopt = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
        if (ct == CT_ToolButton && bopt && w && qobject_cast<QToolBar *>(w->parentWidget())
            && bopt->toolButtonStyle == Qt::ToolButtonIconOnly
            && !(bopt->features & QStyleOptionToolButton::MenuButtonPopup)) {
            const int side = qMax(bopt->iconSize.width(), bopt->iconSize.height()) + 2 * TOOL_PADDING;
            s = QSize(side, side);
        }
        // Mục 24: 🔒 đã ẩn của thanh lọc khai cao = con hiện cao nhất + lề
        // dưới → thanh lọc cao thêm đúng lề đó (FilterBar::preferredHeight).
        // Lúc thanh chưa hiện lần nào mọi con đều isHidden() → chỉ bỏ con ẩn
        // tường minh (Aᴮ).
        if (ct == CT_ToolButton && w && w->property("samhieFilterBarHeight").toBool() && w->parentWidget()) {
            int tallest = 0;
            for (QWidget *sib : w->parentWidget()->findChildren<QWidget *>(Qt::FindDirectChildrenOnly)) {
                if (sib != w && !(sib->isHidden() && sib->testAttribute(Qt::WA_WState_ExplicitShowHide)))
                    tallest = qMax(tallest, sib->sizeHint().height());
            }
            s.setHeight(tallest + POPUP_MARGIN);
        }
        // Dòng menu cao bằng nhịp sidebar Places (41px với Dongle): Kvantum lấy
        // max(chữ + khung, chiều cao font + 8 "như qcommonstyle") — với hộp
        // dòng cao của Dongle, khoản +8 thắng → 47px (máy ảo 2026-10-04, user
        // muốn gọn lại bằng sidebar). Bỏ khoản +8: chữ + khung MenuItem trên +
        // dưới (MENUITEM_FRAME, kvconfig frame.top/bottom). Vạch ngăn giữ nguyên.
        // Mục 14: KItemListView::setHeaderVisible hỏi cỡ tiêu đề KHÔNG kèm
        // widget (QHeaderView luôn kèm) — chỉ ở Dolphin.
        if (ct == CT_HeaderSection && !w && size.isEmpty()
            && QCoreApplication::applicationName() == QLatin1String("dolphin"))
            s.setHeight(s.height() + HEADER_GAP);
        // Mục 16: hỏi Kvantum cỡ của chính nút đó nếu có một chữ — lấy chiều
        // cao (Kvantum tự tính từ chữ/icon trong option, không theo size).
        // Mục 31: mọi nút chỉ có icon cũng vậy (nút chọn thư mục của Configure
        // Dolphin cao 18px cạnh ô nhập 40px — máy ảo user 2026-10-07), và
        // vuông nếu không có menu.
        auto *pbopt = qstyleoption_cast<const QStyleOptionButton *>(opt);
        if (ct == CT_PushButton && pbopt && pbopt->text.isEmpty() && (isMenuRowButton(w) || !pbopt->icon.isNull())) {
            QStyleOptionButton o(*pbopt);
            o.text = QStringLiteral("X");
            const QSize withText = QProxyStyle::sizeFromContents(
                ct, &o, size.expandedTo(QSize(0, o.fontMetrics.height())), w);
            s.setHeight(qMax(s.height(), withText.height()));
            if (!isMenuRowButton(w) && !(pbopt->features & QStyleOptionButton::HasMenu))
                s.setWidth(s.height());
        }
        if ((ct == CT_RadioButton || ct == CT_CheckBox) && isToggle(w))
            s.setWidth(s.width() + KEY_FOCUS_PAD);  // mục 28
        if (ct == CT_ItemViewItem && w && w->property("samhieTreeLines").toBool())
            s.setHeight(KATE_TREE_ROW);  // mục 61
        auto *mopt = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        if (ct == CT_MenuItem && mopt && mopt->menuItemType != QStyleOptionMenuItem::Separator)
            s.setHeight(qMin(s.height(), mopt->fontMetrics.height() + 2 * MENUITEM_FRAME));
        return s;
    }

private:
    // Mục 78: dòng tiêu đề nhóm (separator có chữ) → dòng thường bị tắt
    static QStyleOptionMenuItem sectionAsItem(const QStyleOptionMenuItem *m)
    {
        QStyleOptionMenuItem item(*m);
        item.menuItemType = QStyleOptionMenuItem::Normal;
        item.checkType = QStyleOptionMenuItem::NotCheckable;
        item.checked = false;
        item.state &= ~(State_Enabled | State_Selected | State_MouseOver | State_Sunken);
        return item;
    }

    // Mục 71: cây kết quả Search của Kate đang bỏ ô tích
    static bool noCheck(const QStyleOptionViewItem *vopt, const QWidget *w)
    {
        const QWidget *view = w ? w : vopt->widget;
        return (vopt->features & QStyleOptionViewItem::HasCheckIndicator) && view
            && view->property("samhieNoCheck").toBool();
    }

    static void trimTabMenu(QMenu *menu)  // mục 19
    {
        for (QAction *a : menu->actions()) {
            if (!a->isSeparator() && a->icon().name() != QLatin1String("tab-close-other"))
                menu->removeAction(a);
        }
    }

    static void trimPlacesMenu(QMenu *menu)  // mục 19
    {
        const QList<QAction *> actions = menu->actions();
        QAction *lastHint = nullptr;
        bool places = false;
        for (QAction *a : actions) {
            if (a->icon().name() == QLatin1String("hint"))
                lastHint = a;
            if (a->objectName() == QLatin1String("lock_panels"))
                places = true;
        }
        if (!places && !lastHint)
            return;
        for (QAction *a : actions) {
            const QString icon = a->icon().name();
            if (a == lastHint || a->menu() || a->objectName() == QLatin1String("lock_panels")
                || icon == QLatin1String("window-new") || icon == QLatin1String("document-properties"))
                menu->removeAction(a);
        }
    }

    static void hideItemCreateNew(QMenu *menu)  // mục 17
    {
        bool first = true;
        for (QAction *a : menu->actions()) {
            if (!a->isVisible() || a->isSeparator())
                continue;
            if (!first && a->menu() && a->icon().name() == QLatin1String("list-add")) {
                a->setVisible(false);
                return;
            }
            first = false;
        }
    }

    // Menu con "Create New" của các cửa sổ Dolphin (hành động "new_menu").
    static QList<QMenu *> windowCreateNewMenus()
    {
        QList<QMenu *> menus;
        for (QWidget *top : QApplication::topLevelWidgets()) {
            if (!top->inherits("DolphinMainWindow"))
                continue;
            if (auto *a = top->findChild<QAction *>(QStringLiteral("new_menu")); a && a->menu())
                menus << a->menu();
        }
        return menus;
    }

    static void removeHamburgerCreateNew(QMenu *menu)  // mục 17
    {
        const QList<QMenu *> createNew = windowCreateNewMenus();
        for (QAction *a : menu->actions()) {
            if (a->menu() && createNew.contains(a->menu()))
                menu->removeAction(a);
        }
    }

    static void trimKonsoleMenu(QMenu *menu)  // mục 55
    {
        for (QAction *a : menu->actions()) {
            if (a->objectName() == QLatin1String("bookmark") || a->objectName() == QLatin1String("switch-profile")
                || a->objectName() == QLatin1String("edit-current-profile")
                || QString(a->text()).remove(QLatin1Char('&')) == QLatin1String("Built-in"))
                menu->removeAction(a);
        }
        flattenHamburgerConfigure(menu);
        trimHamburgerMoreHelp(menu);
    }

    static void trimHamburgerMoreHelp(QMenu *menu)  // mục 40
    {
        for (QAction *a : menu->actions()) {
            QMenu *sub = a->menu();
            if (!sub)
                continue;
            const QList<QAction *> subActions = sub->actions();
            // More: menu KHamburgerMenu dựng, mở đầu bằng Show Menubar.
            if (std::any_of(subActions.begin(), subActions.end(), [](QAction *s) {
                    return s->objectName() == QLatin1String("options_show_menubar");
                })) {
                menu->removeAction(a);
                continue;
            }
            for (QAction *s : subActions) {
                if (HELP_HIDDEN.contains(s->objectName()))
                    sub->removeAction(s);
            }
        }
    }

    static void flattenHamburgerConfigure(QMenu *menu)  // mục 29
    {
        for (QAction *a : menu->actions()) {
            QMenu *sub = a->menu();
            if (!sub)
                continue;
            QAction *keys = nullptr, *notify = nullptr, *prefs = nullptr;
            bool language = false;
            for (QAction *s : sub->actions()) {
                if (s->objectName() == QLatin1String("options_configure_keybinding"))
                    keys = s;
                else if (s->objectName() == QLatin1String("options_configure_notifications"))
                    notify = s;  // Konsole (mục 55); ☰ Dolphin không có
                else if (s->objectName() == QLatin1String("options_configure"))
                    prefs = s;
                else if (s->objectName() == QLatin1String("switch_application_language"))
                    language = true;
            }
            // Dấu nhận tầng Configure: có Configure Language. Không có dấu này
            // thì "Menu ›" cuối menu chuột phải Konsole (= ☰ ĐÃ phẳng, chứa
            // thẳng Configure Konsole…) bị làm phẳng nhầm vào menu chuột phải
            // (container 2026-10-09).
            if (!prefs || !language)
                continue;
            if (keys)
                menu->insertAction(a, keys);
            if (notify)
                menu->insertAction(a, notify);
            menu->insertAction(a, prefs);
            menu->removeAction(a);
            return;
        }
    }

    static void hideViewportFolderColors(QMenu *menu)  // mục 17
    {
        const QList<QMenu *> createNew = windowCreateNewMenus();
        const QList<QAction *> actions = menu->actions();
        if (std::none_of(actions.begin(), actions.end(),
                         [&](QAction *a) { return a->menu() && createNew.contains(a->menu()); }))
            return;
        for (QAction *a : actions) {
            auto *wa = qobject_cast<QWidgetAction *>(a);
            if (!wa)
                continue;
            for (QWidget *row : static_cast<WidgetActionAccess *>(wa)->createdWidgets()) {
                const auto buttons = row->findChildren<QPushButton *>();
                if (std::any_of(buttons.begin(), buttons.end(), [](QPushButton *b) { return b->menu(); })) {
                    // Gỡ hẳn: ẩn (setVisible) thì QMenu bỏ qua hành động
                    // nhưng widget dòng vẫn hiện, đè lên dòng đầu (container).
                    menu->removeAction(a);
                    return;
                }
            }
        }
    }

    // Mục 52. Hành động không có objectName; con của FileViewGitPlugin
    // (dolphin-plugins 26.08.1: m_cloneAction = new QAction(this), chữ
    // "<application>Git</application> Clone..."). Hành động khác của plugin
    // (Commit, Push… trong repo) cũng là con của nó → lọc thêm theo chữ.
    static void hideGitClone(QMenu *menu)
    {
        for (QAction *a : menu->actions()) {
            if (a->parent() && a->parent()->inherits("FileViewGitPlugin")
                && a->text().contains(QLatin1String("Clone")))
                a->setVisible(false);
        }
    }

    PanePainter m_pane;
    bool m_light;
    PopupPainter m_popup;
    CardPainter m_card;
    EditorAlpha m_editorAlpha;
    MenuButtonForwarder m_menuForwarder;
    UrlButtonHover m_urlHover;
    NoEditPencil m_noPencil;
    HideDock m_hideDock;
    PopupAnchor m_popupAnchor;
    NoPaint m_noPaint;
    FollowAppPalette m_followAppPalette;
    SidebarTabPaint m_sidebarTab;
    KeepCommandBarFocus m_keepCommandBarFocus;
    CrumbIndent m_crumbIndent;
    MonoSpans m_monoSpans;
    PropertiesDialogSetup m_propsSetup;
    DetailsAlign m_detailsAlign;
    CommentHeight m_commentHeight;
    // Mục 14: ô đệm phải của tiêu đề cột, Dolphin vẽ ngay trước cột cuối.
    mutable QRect m_headerEnd;
};

}  // namespace

// Mục 47: sửa .directory của thư mục vừa được hàng màu đổi icon.
// Lưu riêng màu và ký hiệu (X-Samhie-FolderColor / X-Samhie-FolderSymbol)
// cạnh Icon: plugin ghi đè Icon mỗi lần bấm, nên trạng thái trước đó phải nằm
// ở khoá riêng. Ký hiệu "tự nhiên" của thư mục XDG (Music → music…) là cái KIO
// tự gán khi .directory không có Icon (KFileItem, KIO 6.30).
class FolderChangeWatcher : public QObject
{
    Q_OBJECT

public:
    static FolderChangeWatcher *instance()
    {
        static auto *watcher = [] {
            auto *w = new FolderChangeWatcher(qApp);
            QDBusConnection::sessionBus().connect(QString(), QString(), QStringLiteral("org.kde.KDirNotify"),
                                                  QStringLiteral("FilesChanged"), w,
                                                  SLOT(filesChanged(QStringList)));
            w->m_timeout.setSingleShot(true);
            w->m_timeout.setInterval(3000);  // plugin báo gần như ngay — quá lâu thì bỏ ý định
            return w;
        }();
        return watcher;
    }

    void remember(const QString &dir, const QString &symbol, const QString &color)
    {
        m_prevDir = dir.isEmpty() ? QString() : QFileInfo(dir).canonicalFilePath();
        m_prevSymbol = symbol;
        m_prevColor = color;
    }

    void arm(FolderChange kind, const QString &value)
    {
        m_kind = kind;
        m_value = value;
        m_armed = true;
        m_timeout.start();
    }

public Q_SLOTS:
    void filesChanged(const QStringList &urls)
    {
        if (!m_armed || !m_timeout.isActive())
            return;
        for (const QString &u : urls) {
            const QUrl url(u);
            if (!url.isLocalFile() || !QFileInfo(url.toLocalFile()).isDir())
                continue;
            m_armed = false;
            apply(url.toLocalFile());
            // Báo lại để Dolphin đọc .directory mới (thông báo của plugin có
            // thể đã tới Dolphin trước khi ta sửa).
            QDBusMessage msg = QDBusMessage::createSignal(QStringLiteral("/"), QStringLiteral("org.kde.KDirNotify"),
                                                          QStringLiteral("FilesChanged"));
            msg << QStringList{u};
            QDBusConnection::sessionBus().send(msg);
            return;
        }
    }

private:
    using QObject::QObject;

    void apply(const QString &dir)
    {
        const QString group = QStringLiteral("[Desktop Entry]");
        const QString iconKey = QStringLiteral("Icon"), symbolKey = FOLDER_SYMBOL_KEY, colorKey = FOLDER_COLOR_KEY;
        QFile file(dir + QStringLiteral("/.directory"));
        QStringList lines;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
        file.close();
        while (!lines.isEmpty() && lines.last().isEmpty())
            lines.removeLast();

        // Đọc khoá trong [Desktop Entry]; gỡ ba khoá của ta để ghi lại.
        QHash<QString, QString> keys;
        int start = lines.indexOf(group), end = lines.size();
        if (start >= 0) {
            for (int i = start + 1; i < lines.size(); ++i) {
                if (lines[i].startsWith(QLatin1Char('['))) {
                    end = i;
                    break;
                }
            }
            for (int i = end - 1; i > start; --i) {
                const int eq = lines[i].indexOf(QLatin1Char('='));
                const QString key = lines[i].left(eq).trimmed();
                if (eq > 0 && (key == iconKey || key == symbolKey || key == colorKey)) {
                    keys.insert(key, lines[i].mid(eq + 1).trimmed());
                    lines.removeAt(i);
                    --end;
                }
            }
        }

        const QString natural = naturalFolderSymbol(dir);
        QString symbol = keys.contains(symbolKey) ? keys.value(symbolKey) : natural;
        if (symbol == QLatin1String("none"))
            symbol.clear();
        QString color = keys.value(colorKey);
        if (!m_prevDir.isEmpty() && QFileInfo(dir).canonicalFilePath() == m_prevDir) {
            symbol = m_prevSymbol;  // đọc lúc menu mở, trước khi plugin ghi đè
            color = m_prevColor;
        }
        if (m_kind == FolderChange::Color)
            color = m_value;
        else
            symbol = m_value;

        QString icon = symbol.isEmpty() ? QStringLiteral("folder") : QStringLiteral("folder-") + symbol;
        if (!color.isEmpty())
            icon += QLatin1Char('-') + color;
        const QString naturalIcon = natural.isEmpty() ? QStringLiteral("folder") : QStringLiteral("folder-") + natural;

        QStringList add;
        if (icon != naturalIcon)
            add << iconKey + QLatin1Char('=') + icon;
        if (symbol != natural)
            add << symbolKey + QLatin1Char('=') + (symbol.isEmpty() ? QStringLiteral("none") : symbol);
        if (!color.isEmpty())
            add << colorKey + QLatin1Char('=') + color;
        if (start < 0 && !add.isEmpty()) {
            lines << group;
            start = lines.size() - 1;
            end = lines.size();
        }
        for (const QString &line : add)
            lines.insert(end++, line);
        // Nhóm rỗng (chỉ còn tiêu đề) và không còn gì khác → xoá file như plugin.
        if (start >= 0 && end == start + 1)
            lines.removeAt(start);
        bool empty = std::all_of(lines.begin(), lines.end(), [](const QString &l) { return l.trimmed().isEmpty(); });
        if (empty) {
            QFile::remove(file.fileName());
            return;
        }
        QSaveFile out(file.fileName());
        if (out.open(QIODevice::WriteOnly | QIODevice::Text)) {
            out.write((lines.join(QLatin1Char('\n')) + QLatin1Char('\n')).toUtf8());
            out.commit();
        }
    }

    QString m_prevDir, m_prevSymbol, m_prevColor;
    bool m_armed = false;
    FolderChange m_kind = FolderChange::Color;
    QString m_value;
    QTimer m_timeout;
};

void armFolderChange(FolderChange kind, const QString &value)
{
    FolderChangeWatcher::instance()->arm(kind, value);
}

void watchFolderChanges(const QString &dir, const QString &symbol, const QString &color)
{
    FolderChangeWatcher::instance()->remember(dir, symbol, color);
}

// Mục 80.
// Đọc tay một khoá: QSettings (IniFormat) đọc kdeglobals của KDE ra rỗng
// (container 2026-10-20: file có ColorScheme=SamhieLight vẫn ra tối).
// Kate giữ bản tối cho tới khi có bản sáng riêng: vùng code tối (bảng màu
// riêng). Konsole: mục 81.
bool lightMode()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
               + QStringLiteral("/kdeglobals"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    bool general = false, light = false;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        if (line.startsWith('['))
            general = line == "[General]";
        else if (general && line.startsWith("ColorScheme="))
            light = line == "ColorScheme=SamhieLight";
    }
    return light;
}

// Mục 81: lớp bọc thay MenuStyle của Konsole khi nạp lại style.
class AltKeyStyle : public QProxyStyle
{
public:
    AltKeyStyle(QStyle *base, int altKey)
        : QProxyStyle(base)
        , m_altKey(altKey)
    {
    }

    int styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *ret) const override
    {
        return hint == SH_MenuBar_AltKeyNavigation ? m_altKey : QProxyStyle::styleHint(hint, opt, w, ret);
    }

private:
    int m_altKey;
};

class ColorModeWatcher : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    bool light = false;

public Q_SLOTS:
    void notifyChange(int type, int)
    {
        if (type != 0)  // 0 = PaletteChanged (KGlobalSettings)
            return;
        const bool now = lightMode();
        if (now == light)
            return;
        light = now;
        // Sau khi tín hiệu này xử lý xong: setStyle xoá style cũ — có thể là
        // style đang gọi tới đây.
        QTimer::singleShot(0, qApp, [] {
            QStyle *style = qApp->style();
            if (dynamic_cast<SamhieStyle *>(style)) {
                QApplication::setStyle(QStringLiteral("samhie"));
                return;
            }
            // Mục 81: Konsole bọc style app trong MenuStyle (main.cpp: một
            // QProxyStyle chỉ đổi SH_MenuBar_AltKeyNavigation) — style()->name()
            // RỖNG (container 2026-10-20). Thay bằng lớp bọc giữ đúng gợi ý đó.
            auto *outer = qobject_cast<QProxyStyle *>(style);
            if (outer && dynamic_cast<SamhieStyle *>(outer->baseStyle()))
                QApplication::setStyle(new AltKeyStyle(QStyleFactory::create(QStringLiteral("samhie")),
                                                       style->styleHint(QStyle::SH_MenuBar_AltKeyNavigation)));
        });
    }
};

void watchColorMode(bool light)
{
    static auto *watcher = [] {
        auto *w = new ColorModeWatcher(qApp);
        QDBusConnection::sessionBus().connect(QString(), QStringLiteral("/KGlobalSettings"),
                                              QStringLiteral("org.kde.KGlobalSettings"),
                                              QStringLiteral("notifyChange"), w, SLOT(notifyChange(int, int)));
        return w;
    }();
    watcher->light = light;
}

class SamhieStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "samhie-style.json")

public:
    QStyle *create(const QString &key) override
    {
        return key.compare(QLatin1String("samhie"), Qt::CaseInsensitive) == 0 ? new SamhieStyle : nullptr;
    }
};

#include "samhie-style.moc"
