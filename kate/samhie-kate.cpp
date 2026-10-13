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
//    - Đầu khung Projects: ô chọn dự án vẽ phẳng thành tên (style samhie mục
//      59 — plugin chỉ đánh dấu), bỏ nút ✕ (đóng dự án), thêm « thu khung. Đóng
//      dự án (hiếm dùng) còn ở Ctrl+Alt+I › "Close Project" — menu Projects
//      đã ẩn ở mục 2.
//    - Cây Projects và vùng code (breadcrumb + editor) là hai thẻ bo góc trên
//      nền cửa sổ: style samhie vẽ (mục 60), plugin đánh dấu widget
//      (samhieCard) và chừa khe CARD_GAP quanh thẻ. Cây có đường nhánh, không
//      mũi tên (mục 61, samhieTreeLines). Đỉnh thẻ cây thẳng hàng đỉnh thẻ
//      code: bỏ vạch ngăn dưới hàng tên dự án, hàng đó cao bằng hàng tab
//      (đo container: ô chọn 42 + vạch 3 so với hàng tab 40 → lệch 5px; máy
//      ảo user 2026-10-11 cũng 5px). Nhật ký trong khung Output
//      (KateOutputEdit) cũng là thẻ, ô Search + nút nằm trên nền cửa sổ.
//    - Icon cây Projects và mọi nút icon ở hàng trên (⟳ « đầu khung Projects;
//      ‹ › ⌘ ⧉ ☰ của view space) cùng cỡ ICON_SIZE (user 2026-10-12: to hơn
//      cho đồng đều — trước đều 16, thử 17, chốt 18). Ô hover của các nút
//      đó vuông icon + 6px mỗi phía (user 2026-10-13: trước cao bằng icon,
//      28×18 — "lùn"); plugin đánh dấu samhieSquareHover, style vẽ (mục 68).
//
// 5. Menu chuột phải trên nút panel (Output, Diagnostics, cột icon) không có
//    "Hide Button" (user 2026-10-11): với Output nó là mục ĐẦU menu — nhiều khả năng lỡ
//    bấm nên nút Output biến mất khỏi đáy cửa sổ (file phiên máy ảo:
//    Show-Button-In-Sidebar=false). Ẩn nút có chủ ý vẫn làm được bằng
//    Ctrl+Alt+I › "Show … Button".
//
// 6. Bấm chỗ TRỐNG của cột icon trái / thanh đáy (Diagnostics, Output) không
//    làm gì (user 2026-10-11: "nút nào thì thực hiện đúng nhiệm vụ của nút
//    đó", lỡ bấm ra ngoài mà thẻ tự đóng thì hoang mang). Kate mặc định thu/
//    mở panel dùng gần nhất (Sidebar::eventFilter, MouseButtonPress trên
//    MultiTabBar có thuộc tính "is-multi-tabbar", katemdi.cpp 26.08.1).
//
// 7. Dải tab không kẹt ở độ cuộn cũ. Mở nhiều file lúc khởi động, có lúc dải
//    tab cuộn tới mức chỉ còn tab cuối, bên phải trống trơn (container: nút
//    hàng trên 18px thì lần nào cũng vậy, 17px thì không — do thứ tự xếp bố
//    cục; dải tab rộng 1133 mà độ cuộn 1400). Qt 6.11 chỉ tính lại độ cuộn
//    khi tab đổi hoặc dải tab đổi cỡ (QTabBar::resizeEvent → makeVisible,
//    nhánh "lấp chỗ trống bên phải") → sau mỗi lần xếp lại bố cục và mỗi
//    lần đổi file, gửi lại sự kiện đổi cỡ cho dải tab (refreshTabScroll).
//
// 8. Cột số dòng rộng ít nhất bằng GUTTER_DIGITS chữ số (user 2026-10-12:
//    file 3 chữ số cũng rộng như file 4 chữ số). Kate tính bề rộng theo đúng
//    số chữ số của file (KateIconBorder::lineNumberWidth, KTextEditor 6.30:
//    (chữ số + 1) × bề rộng chữ số lớn nhất), số căn phải → thêm khoảng trống
//    BÊN TRÁI cột: ô đệm trái của lưới trong view (m_leftSpacer, ô 2,0 —
//    kateview.cpp setupLayout; style samhie để nó 0). Khe giữa số và code
//    rộng thêm: style samhie mục 64.
//
// 9. Chuột phải file/thư mục trong cây Projects › "Open Internal Terminal
//    Here" mở TAB terminal của plugin tại thư mục đó (user 2026-10-12), thay vì
//    khung Terminal dưới đáy của plugin Projects (trùng mục 1). Mục đó là một
//    lambda giữ đường dẫn theo tham chiếu (kateprojecttreeviewcontextmenu.cpp
//    26.08.1) — plugin tự tính lại đường dẫn từ sự kiện chuột phải, cùng công
//    thức với KateProjectViewTree::contextMenuEvent, rồi nối mục sang tab.
//
// 10. Menu chuột phải trên tab bỏ Open…, Detach Document (mở file ở cửa sổ
//    mới), Properties, Compare with Active Document (+ "… Using") — rà Kate
//    đợt 1, user 2026-10-12. Menu dựng mới mỗi lần trong mã
//    (KateViewSpace::buildContextMenu) nên ẩn mục lúc menu polish, trước khi
//    tính cỡ. Menu chuột phải trong vùng code: kate-ui.sh.
//
// 11. Khung Projects khi chưa mở thư mục nào: thẻ "You have not yet opened a
//    folder." + nút Open Folder như VS Code (user 2026-10-12 — Kate để trống
//    trơn; phòng khi tắt welcome page). Nút gọi openDirectoryOrProject() của
//    plugin Projects (hành động "Open Folder…" có sẵn). Thẻ nằm CẠNH ngăn xếp
//    cây dự án chứ không trong nó: Kate ép kiểu currentWidget() của ngăn xếp
//    thành KateProjectView (kateprojectpluginview.cpp). ToolView của Kate tự
//    thêm mọi con vào bố cục và chuyển focus cho con mới nhất
//    (ToolView::childEvent) — trả focus về ngăn xếp như lúc Kate dựng xong.
//
// 12. Rà Kate đợt 2 (2026-10-12):
//    - Quick Open (Ctrl+Alt+O) rộng 60% cửa sổ thay vì 1/2,4 (đường dẫn dài
//      bị cắt cụt — Kate tự vẽ chữ, không có "…"; KateQuickOpen::
//      updateViewGeometry, Kate 26.08.1). Kate đặt lại cỡ mỗi lần mở và mỗi
//      khi cửa sổ đổi cỡ (setFixedSize rồi move) → nới lại ở Show/Move, giữ tâm.
//    - Breadcrumb bỏ nút "…" đầu dải (bấm vào thì nở thành cả đường dẫn thật
//      /var/home/… — VS Code chỉ hiện từ thư mục dự án). Kate hiện lại nút mỗi
//      lần đổi file (UrlbarContainer, kateurlbar.cpp) → ép cỡ 0 thay vì ẩn,
//      dải lùi vào 12px thay chỗ nút.
//    - Panel Search ở cột trái (~190px) chật: "Replace"/"Replace Checked" cắt
//      thành "R…". Thành nút icon cạnh ô Replace như VS Code, hàng dưới chỉ
//      còn nút tuỳ chọn (icon Hugeicons replace / replace-all — bộ icon
//      Samhie). Kate xếp lưới lại khi panel đổi giữa dọc/ngang
//      (KatePluginSearchView::onResize, trong bộ lọc Resize của nó — chạy SAU
//      bộ lọc ứng dụng của plugin) → xếp lại sau một nhịp.
//    - Nút Git ở cột icon trái: icon Kate nhúng trong mã (gitIcon(),
//      sc-apps-git.svg — không qua bộ icon) → gắn "samhie-kate-git" (Hugeicons
//      git-branch) lên nút. Nút là KMultiTabBarTab mang tên panel ở thuộc tính
//      kate_original_text (Sidebar::updateButtonStyle, katemdi.cpp).
//
// 13. Hàng tên dự án và cột icon trái (user 2026-10-13):
//    - Ô sáng nút panel (style samhie mục 66, ô 48 như VS Code) cao bằng ô
//      hover tên dự án = chiều cao ô chọn dự án (co giãn theo mục 4 cho đỉnh
//      thẻ cây thẳng đỉnh thẻ code — đo: 41 máy ảo, 40 container). Phần còn
//      lại của nhịp 48 thành khe giữa các nút (spacing của bố cục
//      KMultiTabBarInternal) → icon vẫn cách nhau 48, ô sáng Projects khớp
//      ô hover cạnh nó.
//    - Chữ tên dự án bắt đầu đúng chỗ icon thư mục cấp gốc trong cây (Kvantum
//      đặt chữ sát mép ô, lệch icon ~18–20px). Vị trí icon = hộp decoration
//      style trả về cho dòng cấp gốc (khớp điểm ảnh, đo container) → ghi vào
//      thuộc tính samhieTextIndent của ô chọn, style samhie tự vẽ chữ ở đó.

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

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QLabel>
#include <QDir>
#include <QFrame>
#include <QFileInfo>
#include <QFontMetricsF>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPointer>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QStyleOptionViewItem>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <algorithm>
#include <cmath>

namespace
{
const QString kTitle = QStringLiteral("Terminal");
// Khe giữa thẻ và mép cửa sổ / thanh trạng thái (bản phác: ~5px)
constexpr int CARD_GAP = 6;
// Icon cây Projects + nút hàng trên (đo máy ảo: đều 16; user thử 17 rồi chọn
// 18 "cho chẵn")
constexpr int ICON_SIZE = 18;

// Mục 8: cột số dòng tối thiểu bấy nhiêu chữ số
constexpr int GUTTER_DIGITS = 4;

void padGutter(KTextEditor::View *view)
{
    auto *grid = qobject_cast<QGridLayout *>(view->layout());
    QLayoutItem *item = grid ? grid->itemAtPosition(2, 0) : nullptr;
    QSpacerItem *spacer = item ? item->spacerItem() : nullptr;
    if (!spacer) {
        return;
    }
    // Cùng công thức với Kate: số chữ số = ceil(log10(số dòng + 1))
    const int digits = int(std::ceil(std::log10(double(view->document()->lines() + 1))));
    const QFontMetricsF fm(view->configValue(QStringLiteral("font")).value<QFont>());
    qreal digitWidth = 0;
    for (char c = '0'; c <= '9'; ++c) {
        digitWidth = std::max(digitWidth, std::ceil(fm.horizontalAdvance(QLatin1Char(c))));
    }
    const int pad = std::max(0, GUTTER_DIGITS - digits) * int(digitWidth);
    if (spacer->sizeHint().width() != pad) {
        spacer->changeSize(pad, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        grid->invalidate();
    }
}

// Mục 7: gửi lại QResizeEvent (cùng cỡ) để QTabBar tính lại độ cuộn
void refreshTabScroll(QWidget *viewSpace)
{
    QTimer::singleShot(0, viewSpace, [viewSpace] {
        for (auto *bar : viewSpace->findChildren<QTabBar *>()) {
            if (bar->inherits("KateTabBar")) {
                QResizeEvent ev(bar->size(), bar->size());
                QCoreApplication::sendEvent(bar, &ev);
            }
        }
    });
}

// Mục 13: nút chỉ-có-icon ở cột trái/phải (Kate xoá chữ ở đó)
bool isIconOnlySidebarTab(const QObject *obj)
{
    auto *b = qobject_cast<const QAbstractButton *>(obj);
    return b && obj->inherits("KMultiTabBarTab") && b->text().isEmpty();
}

// Mục 13: mép trái hộp icon của dòng cấp gốc trong cây, toạ độ của `to`
int rootIconLeft(QTreeView *tree, QWidget *to)
{
    QStyleOptionViewItem opt;
    opt.initFrom(tree->viewport());
    opt.rect = QRect(tree->rootIsDecorated() ? tree->indentation() : 0, 0, tree->viewport()->width(), std::max(tree->sizeHintForRow(0), tree->iconSize().height()));
    opt.features = QStyleOptionViewItem::HasDecoration | QStyleOptionViewItem::HasDisplay;
    opt.decorationSize = tree->iconSize();
    opt.decorationPosition = QStyleOptionViewItem::Left;
    opt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    opt.font = tree->font();
    const QRect deco = tree->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, tree);
    return tree->viewport()->mapTo(to, deco.topLeft()).x();
}

void growIcons(QWidget *parent)
{
    for (auto *button : parent->findChildren<QToolButton *>(Qt::FindDirectChildrenOnly)) {
        button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        // Ô hover vuông như nút icon khác của Samhie (style samhie mục 68) —
        // style đọc thuộc tính lúc polish → polish lại
        if (!button->property("samhieSquareHover").toBool()) {
            button->setProperty("samhieSquareHover", true);
            button->style()->polish(button);
        }
    }
}

// Style samhie đọc thuộc tính lúc polish — polish lại sau khi đánh dấu
void markCard(QWidget *w)
{
    if (!w->property("samhieCard").toBool()) {
        w->setProperty("samhieCard", true);
        w->style()->polish(w);
    }
}

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

// Mục "Hide Button" trong menu chuột phải nút panel: Sidebar::eventFilter dựng
// QMenu cha là Sidebar, mục mang data = Sidebar::HideButtonAction (11,
// katemdi.h 26.08.1). Tiêu đề mục (section) không có data — xét isValid.
bool isHideButtonAction(const QAction *a)
{
    return a->data().isValid() && a->data().toInt() == 11;
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

// Mục 10: nhận mục theo tên icon (đặt trong mã Kate, không đổi theo ngôn ngữ):
// Open… (KStandardAction::open), Detach Document, Properties (chép icon của
// hành động file_properties), Compare + menu con "… Using" (menuAction mang
// icon của menu con)
bool isTrimmedTabAction(const QAction *a)
{
    static const QStringList icons{QStringLiteral("document-open"),
                                   QStringLiteral("tab-detach"),
                                   QStringLiteral("dialog-object-properties"),
                                   QStringLiteral("vcs-diff")};
    return icons.contains(a->icon().name());
}

// Mục 12: KateQuickOpen không có Q_OBJECT — nhận qua ô nhập (focus proxy)
bool isQuickOpen(const QObject *obj)
{
    auto *w = qobject_cast<const QWidget *>(obj);
    return w && w->focusProxy() && w->focusProxy()->inherits("QuickOpenLineEdit");
}

void widenQuickOpen(QWidget *w)
{
    const int target = std::max(w->width(), int(w->parentWidget()->width() * 0.6));
    if (w->width() == target) {
        return;
    }
    const int center = w->geometry().center().x();
    w->setFixedWidth(target);
    w->move(center - target / 2, w->y());
}

// Mục 12: nút "…" của breadcrumb — QToolButton trong UrlbarContainer (không
// Q_OBJECT), container nằm trong QStackedWidget của KateUrlBar
bool isCrumbEllipsis(const QObject *obj)
{
    auto *b = qobject_cast<const QToolButton *>(obj);
    if (!b || b->text() != QStringLiteral("…")) {
        return false;
    }
    for (const QWidget *w = b->parentWidget(); w; w = w->parentWidget()) {
        if (w->inherits("KateUrlBar")) {
            return true;
        }
    }
    return false;
}

// Mục 12: bỏ "…" — cỡ 0 (Kate gọi show() lại mỗi lần đổi file), dải
// breadcrumb lùi vào 12px thay chỗ nút cho chữ không dính góc bo của thẻ.
// Lề của layout container Kate chỉ đặt một lần lúc dựng (setContentsMargins({})).
void hideCrumbEllipsis(QWidget *button)
{
    button->setFixedSize(0, 0);
    if (QLayout *row = button->parentWidget()->layout()) {
        row->setContentsMargins(12, 0, 0, 0);
    }
}

// Mục 12: lưới panel Search (search.ui: widget "SearchDialog", lưới
// "gridLayout"). Chỉ xếp khi Kate đang để bố cục dọc (ô Replace ở hàng 2).
void arrangeSearchPanel(QWidget *dialog)
{
    auto *grid = dialog->findChild<QGridLayout *>(QStringLiteral("gridLayout"));
    const auto child = [dialog](const char *name) {
        return dialog->findChild<QWidget *>(QLatin1String(name));
    };
    QWidget *replaceCombo = child("replaceCombo");
    auto *replace = qobject_cast<QPushButton *>(child("replaceButton"));
    auto *replaceChecked = qobject_cast<QPushButton *>(child("replaceCheckedBtn"));
    QWidget *options = child("searchOptionsLayoutW");
    QWidget *newTab = child("newTabButton");
    QWidget *more = child("displayOptions");
    if (!grid || !replaceCombo || !replace || !replaceChecked || !options || !newTab || !more) {
        return;
    }
    int row, col, rowSpan, colSpan;
    grid->getItemPosition(grid->indexOf(replaceCombo), &row, &col, &rowSpan, &colSpan);
    if (row != 2 || colSpan != 5) { // bố cục ngang, hoặc đã xếp rồi
        return;
    }
    // Chữ → icon (một lần; giữ cả khi panel chuyển sang ngang)
    if (!replace->text().isEmpty()) {
        replace->setToolTip(replace->text());
        replace->setText(QString());
        replace->setIcon(QIcon::fromTheme(QStringLiteral("edit-find-replace")));
        replaceChecked->setToolTip(replaceChecked->text());
        replaceChecked->setText(QString());
        replaceChecked->setIcon(QIcon::fromTheme(QStringLiteral("samhie-replace-all")));
    }
    grid->addWidget(replaceCombo, 2, 0, 1, 3);
    grid->addWidget(replace, 2, 3);
    grid->addWidget(replaceChecked, 2, 4);
    grid->addWidget(options, 3, 0, 1, 3);
    grid->addWidget(newTab, 3, 3);
    grid->addWidget(more, 3, 4);
}

// Mục 12: nút panel Git ở cột icon trái
bool isGitSidebarButton(const QObject *obj)
{
    return obj->inherits("KMultiTabBarTab") && obj->property("kate_original_text").toString() == QLatin1String("Git");
}

// Mục 9: cây Projects mà plugin đã đánh dấu (samhieTreeLines) — mỗi dự án một cây
QTreeView *projectTreeOf(QObject *obj)
{
    auto *tree = qobject_cast<QTreeView *>(obj);
    if (!tree && obj->parent()) {
        tree = qobject_cast<QTreeView *>(obj->parent());
        if (tree && tree->viewport() != obj) {
            tree = nullptr;
        }
    }
    return tree && tree->property("samhieTreeLines").toBool() ? tree : nullptr;
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

        // Mục 8: cột số dòng — view mới, số dòng đổi, đổi font/phóng to chữ
        const auto watchGutter = [this](KTextEditor::View *view) {
            padGutter(view);
            connect(view->document(), &KTextEditor::Document::textChanged, view, [view] {
                padGutter(view);
            });
            connect(view, &KTextEditor::View::configChanged, view, [view] {
                padGutter(view);
            });
        };
        for (KTextEditor::View *view : m_mw->views()) {
            watchGutter(view);
        }
        connect(m_mw, &KTextEditor::MainWindow::viewCreated, this, watchGutter);

        // Mục 7: đổi file (mở file mới cũng là đổi) → dải tab tính lại độ cuộn
        connect(m_mw, &KTextEditor::MainWindow::viewChanged, this, [this] {
            for (QWidget *vs : m_mw->window()->findChildren<QWidget *>()) {
                if (isViewSpace(vs)) {
                    refreshTabScroll(vs);
                }
            }
        });

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

        // Mục 12: breadcrumb của view space đã dựng trước khi plugin nạp (cũng
        // nút Git dưới)
        for (auto *button : m_mw->window()->findChildren<QToolButton *>()) {
            if (isCrumbEllipsis(button)) {
                hideCrumbEllipsis(button);
            }
        }
        // Mục 12: nút Git — panel Git dựng trước plugin này
        for (auto *button : m_mw->window()->findChildren<QAbstractButton *>()) {
            if (isGitSidebarButton(button)) {
                button->setIcon(QIcon::fromTheme(QStringLiteral("samhie-kate-git")));
            }
        }
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            // Mục 6: bấm chỗ trống cột icon / thanh đáy không thu-mở panel.
            // Bộ lọc của ứng dụng chạy trước bộ lọc Sidebar::eventFilter.
            if (obj->property("is-multi-tabbar").toBool() && static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
                return true;
            }
            break;
        case QEvent::ContextMenu:
            // Mục 9: nhớ thư mục của mục bị chuột phải (sự kiện tới viewport
            // trước khi Kate dựng menu)
            if (QTreeView *tree = projectTreeOf(obj)) {
                const QModelIndex index = tree->indexAt(static_cast<QContextMenuEvent *>(event)->pos());
                QString path = index.isValid() ? index.data(Qt::UserRole).toString() : projectBaseDir();
                if (QFileInfo(path).isFile()) {
                    path = QFileInfo(path).absolutePath();
                }
                m_contextDir = path;
            }
            break;
        case QEvent::Polish:
            if (isCrumbEllipsis(obj)) {
                hideCrumbEllipsis(static_cast<QWidget *>(obj)); // mục 12 (view space mới)
            }
            if (isGitSidebarButton(obj)) {
                static_cast<QAbstractButton *>(obj)->setIcon(QIcon::fromTheme(QStringLiteral("samhie-kate-git"))); // mục 12
            }
            if (isIconOnlySidebarTab(obj)) {
                schedulePlaceMenuButton(); // mục 13: nút panel nạp sau
            }
            if (auto *menu = qobject_cast<QMenu *>(obj); menu && menu->parentWidget()) {
                if (isViewSpace(menu->parentWidget())) {
                    for (QAction *a : menu->actions()) {
                        if (isTrimmedTabAction(a)) {
                            a->setVisible(false);
                        }
                    }
                } else if (QTreeView *tree = projectTreeOf(menu->parentWidget()); tree && !m_contextDir.isEmpty()) {
                    for (QAction *a : menu->actions()) {
                        // Kate đặt icon "terminal", nhưng bộ icon không có tên đó thì
                        // icon rỗng (đo container) → nhận theo chữ. Giao diện Samhie
                        // tiếng Anh; ngôn ngữ khác thì mục giữ hành vi gốc của Kate.
                        if (a->text() == QLatin1String("Open Internal Terminal Here")) {
                            a->disconnect(tree);
                            connect(a, &QAction::triggered, this, [this, dir = m_contextDir] {
                                openTerminalIn(dir);
                            });
                        }
                    }
                }
            }
            break;
        case QEvent::ActionAdded:
        case QEvent::ActionChanged:
            if (obj == m_menuBar) {
                trimMenuBar(m_menuBar);
            }
            break;
        case QEvent::Show:
            if (isQuickOpen(obj)) {
                widenQuickOpen(static_cast<QWidget *>(obj)); // mục 12
            }
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
            if (auto *menu = qobject_cast<QMenu *>(obj); menu && menu->parentWidget() && menu->parentWidget()->inherits("KateMDI::Sidebar")) {
                for (QAction *a : menu->actions()) {
                    if (isHideButtonAction(a)) {
                        a->setVisible(false);
                    }
                }
            }
            if (auto *edit = qobject_cast<QLineEdit *>(obj); edit && isProjectFilter(edit)) {
                edit->hide();
                QWidget *view = edit->parentWidget();
                // trái: cách cột icon; phải: khe do vùng code chừa (dưới)
                view->layout()->setContentsMargins(CARD_GAP, 0, 0, CARD_GAP);
                QWidget *tree = view->layout()->itemAt(0)->widget();
                m_projectTree = tree;
                schedulePlaceMenuButton();  // căn đỉnh thẻ (alignProjectCard)
                tree->setProperty("samhieTreeLines", true);  // style samhie mục 61
                markCard(tree);
                if (auto *view = qobject_cast<QTreeView *>(tree)) {
                    view->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
                }
            } else if (auto *combo = qobject_cast<QComboBox *>(obj)) {
                if (QHBoxLayout *row = projectHeader(combo)) {
                    fixProjectHeader(row, combo->parentWidget());
                }
            }
            // Khung Output (KateOutputView: [hàng Search, nhật ký]): thẻ thẳng
            // hàng với thẻ code ở trên (trái = khe vùng code, phải/dưới = khe)
            if (obj->inherits("KateOutputEdit")) {
                auto *edit = static_cast<QWidget *>(obj);
                if (!edit->property("samhieCard").toBool() && edit->parentWidget() && edit->parentWidget()->layout()) {
                    edit->parentWidget()->layout()->setContentsMargins(CARD_GAP, 0, CARD_GAP, CARD_GAP);
                    markCard(edit);
                }
            }
            if (isViewSpace(obj)) {
                auto *vs = static_cast<QWidget *>(obj);
                if (!vs->property("samhieCard").toBool()) {
                    vs->layout()->setContentsMargins(0, 0, CARD_GAP, CARD_GAP);
                    markCard(vs);
                }
                // Khe trái của cả vùng code (KateViewManager, QSplitter ngoài
                // cùng): cách thẻ Projects, hoặc cách cột icon khi thu khung lại
                for (QWidget *w = vs->parentWidget(); w; w = w->parentWidget()) {
                    if (w->inherits("KateViewManager")) {
                        w->setContentsMargins(CARD_GAP, 0, 0, 0);
                        break;
                    }
                }
                schedulePlaceMenuButton();
            }
            break;
        case QEvent::Hide:
        case QEvent::Resize:
            if (isViewSpace(obj)) {
                schedulePlaceMenuButton();
            }
            if (event->type() == QEvent::Resize && obj->objectName() == QLatin1String("SearchDialog")) {
                auto *dialog = static_cast<QWidget *>(obj);
                QTimer::singleShot(0, dialog, [dialog] {
                    arrangeSearchPanel(dialog); // mục 12
                });
            }
            break;
        case QEvent::Move:
            if (isQuickOpen(obj)) {
                widenQuickOpen(static_cast<QWidget *>(obj)); // mục 12
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
        // Vạch ngăn dưới hàng (QFrame HLine, kateprojectpluginview.cpp)
        for (int i = 0; i < toolView->layout()->count(); ++i) {
            auto *line = qobject_cast<QFrame *>(toolView->layout()->itemAt(i)->widget());
            if (line && line->frameShape() == QFrame::HLine) {
                line->hide();
            }
        }
        m_projectCombo = qobject_cast<QComboBox *>(row->itemAt(0)->widget());
        // Ô chọn dự án vẽ phẳng thành tên (style samhie, mục 59)
        row->itemAt(0)->widget()->setProperty("samhieFlatCombo", true);
        auto *collapse = new QToolButton(toolView);
        collapse->setAutoRaise(true);
        collapse->setIcon(QIcon::fromTheme(QStringLiteral("sidebar-collapse-left")));
        collapse->setToolTip(QStringLiteral("Hide Sidebar"));
        connect(collapse, &QToolButton::clicked, this, [this, toolView] {
            m_mw->hideToolView(toolView);
        });
        row->addWidget(collapse);
        growIcons(toolView);
        addWelcome(toolView, row->itemAt(1)->widget());
    }

    // Mục 11
    // reload: nút ⟳ — chưa có dự án thì Kate để nó mờ; ẩn đi cho tiêu đề đủ chỗ
    void addWelcome(QWidget *toolView, QWidget *reload)
    {
        auto *stack = toolView->findChild<QStackedWidget *>(QString(), Qt::FindDirectChildrenOnly);
        if (!stack) {
            return;
        }
        m_welcome = new QWidget(toolView);
        auto *outer = new QVBoxLayout(m_welcome);
        outer->setContentsMargins(CARD_GAP, 0, 0, CARD_GAP); // như cây dự án
        auto *card = new QWidget(m_welcome);
        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(12);
        auto *text = new QLabel(QStringLiteral("You have not yet opened a folder."), card);
        text->setWordWrap(true);
        layout->addWidget(text);
        auto *open = new QPushButton(QStringLiteral("Open Folder"), card);
        connect(open, &QPushButton::clicked, this, [this] {
            if (QObject *projects = m_mw->pluginView(QStringLiteral("kateprojectplugin"))) {
                QMetaObject::invokeMethod(projects, "openDirectoryOrProject");
            }
        });
        layout->addWidget(open);
        layout->addStretch();
        outer->addWidget(card);
        markCard(card);
        m_welcomeCard = card;
        toolView->setFocusProxy(stack);

        // Tiêu đề khi chưa có dự án (Kate để "Current Project", bị cắt chữ)
        if (m_projectCombo) {
            m_projectCombo->setPlaceholderText(QStringLiteral("No Folder Opened"));
        }
        const auto update = [this, stack, reload = QPointer<QWidget>(reload)] {
            const bool empty = stack->count() == 0;
            stack->setVisible(!empty);
            m_welcome->setVisible(empty);
            if (reload) {
                reload->setVisible(!empty);
            }
            schedulePlaceMenuButton(); // căn đỉnh thẻ
        };
        update();
        // widgetRemoved phát khi đang gỡ — xét sau một nhịp
        connect(stack, &QStackedWidget::widgetAdded, this, update);
        connect(stack, &QStackedWidget::widgetRemoved, this, [this, update] {
            QTimer::singleShot(0, this, update);
        });
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

    // Đỉnh thẻ cây Projects (hay thẻ chào, mục 11) = đỉnh thẻ code của view space trên cùng: đổi
    // chiều cao ô chọn dự án (hàng tên dự án cao theo nó) đúng phần lệch.
    void alignProjectCard(QWidget *topViewSpace)
    {
        QWidget *sideCard = m_projectTree && m_projectTree->isVisible() ? m_projectTree.data()
            : m_welcomeCard && m_welcomeCard->isVisible()           ? m_welcomeCard.data()
                                                                    : nullptr;
        if (!m_projectCombo || !sideCard || !topViewSpace) {
            return;
        }
        int cardTop = -1; // breadcrumb hoặc editor — con thấp nhất dưới hàng tab
        for (QObject *o : topViewSpace->children()) {
            auto *c = qobject_cast<QWidget *>(o);
            if (c && !c->isHidden() && (c->inherits("KateUrlBar") || qobject_cast<QStackedWidget *>(c))) {
                cardTop = cardTop < 0 ? c->y() : std::min(cardTop, c->y());
            }
        }
        if (cardTop < 0) {
            return;
        }
        QWidget *win = m_mw->window();
        const int delta = topViewSpace->mapTo(win, QPoint(0, cardTop)).y() - sideCard->mapTo(win, QPoint(0, 0)).y();
        if (delta != 0 && m_projectCombo->height() + delta > 0) {
            m_projectCombo->setFixedHeight(m_projectCombo->height() + delta);
        }
        if (auto *tree = qobject_cast<QTreeView *>(sideCard)) {
            const int indent = rootIconLeft(tree, win) - m_projectCombo->mapTo(win, QPoint(0, 0)).x();
            if (m_projectCombo->property("samhieTextIndent").toInt() != indent) {
                m_projectCombo->setProperty("samhieTextIndent", indent);
                m_projectCombo->update();
            }
        }
    }

    // Mục 13: nút cột icon cao bằng ô chọn dự án, khe bù cho đủ nhịp
    void fitSidebarTabs()
    {
        if (!m_projectCombo) {
            return;
        }
        // Vừa đổi setFixedHeight (alignProjectCard) thì height() chưa kịp đổi
        const int fixed = m_projectCombo->maximumHeight();
        const int height = fixed < QWIDGETSIZE_MAX ? fixed : m_projectCombo->height();
        for (auto *tab : m_mw->window()->findChildren<QAbstractButton *>()) {
            if (!isIconOnlySidebarTab(tab)) {
                continue;
            }
            const int pitch = tab->sizeHint().height(); // ô của style samhie (mục 66)
            const int h = std::min(height, pitch);
            if (tab->minimumHeight() != h || tab->maximumHeight() != h) {
                tab->setFixedHeight(h);
            }
            if (QLayout *bar = tab->parentWidget() ? tab->parentWidget()->layout() : nullptr) {
                if (bar->spacing() != pitch - h) {
                    bar->setSpacing(pitch - h);
                }
            }
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
        alignProjectCard(target);
        fitSidebarTabs();
        for (QWidget *vs : win->findChildren<QWidget *>()) {
            if (isViewSpace(vs)) {
                growIcons(vs);
                refreshTabScroll(vs);
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
        growIcons(target);
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

    QString projectBaseDir() const
    {
        QObject *projects = m_mw->pluginView(QStringLiteral("kateprojectplugin"));
        return projects ? projects->property("projectBaseDir").toString() : QString();
    }

    // Thư mục mở terminal: gốc dự án đang mở → thư mục của file đang xem → home
    QString startDirectory()
    {
        if (const QString dir = projectBaseDir(); !dir.isEmpty() && QFileInfo(dir).isDir()) {
            return dir;
        }
        if (KTextEditor::View *view = m_mw->activeView()) {
            const QUrl url = view->document()->url();
            if (url.isLocalFile()) {
                return QFileInfo(url.toLocalFile()).absolutePath();
            }
        }
        return QDir::homePath();
    }

    TerminalTab *createTerminal(const QString &dir)
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

        qobject_cast<TerminalInterface *>(part)->showShellInDir(dir);
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
        openTerminalIn(startDirectory());
    }

    void openTerminalIn(const QString &dir)
    {
        rememberEditor();
        if (TerminalTab *tab = createTerminal(dir)) {
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
            tab = createTerminal(startDirectory());
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
    QPointer<QComboBox> m_projectCombo;
    QPointer<QWidget> m_projectTree;
    QPointer<QWidget> m_welcome; // mục 11: khung ngoài (lề) và thẻ bên trong
    QPointer<QWidget> m_welcomeCard;
    bool m_placePending = false;
    QPointer<TerminalTab> m_lastTerminal;
    QPointer<QWidget> m_lastEditor;
    QString m_contextDir; // mục 9
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
