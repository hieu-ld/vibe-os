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
//      59 — plugin chỉ đánh dấu), thêm « thu khung. Nút ✕ (đóng dự án) của
//      Kate: 2026-10-10 bỏ; user 2026-10-13 muốn "Close Folder" như VS Code —
//      hiện lại (icon × trơn Hugeicons dialog-close như × thanh tìm Dolphin —
//      window-close-symbolic là × trong vòng tròn của nút cửa sổ; tên
//      "window-close" Kate gọi thì bộ icon Samhie chưa có → icon Breeze) và
//      thêm ☰ › File › "Close Folder" (bấm chính nút ✕, mờ theo nút — Kate
//      tắt nút khi chưa mở thư mục). Đóng: Kate hỏi đóng luôn file của dự án.
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
//    - Quick Open: xem mục 14 (trước: chỉ nới rộng 60%).
//    - Breadcrumb bỏ nút "…" đầu dải (bấm vào thì nở thành cả đường dẫn thật
//      /var/home/… — VS Code chỉ hiện từ thư mục dự án). Kate hiện lại nút mỗi
//      lần đổi file (UrlbarContainer, kateurlbar.cpp) → ép cỡ 0 thay vì ẩn,
//      dải lùi vào 12px thay chỗ nút.
//    - Panel Search ở cột trái (~190px) chật: "Replace"/"Replace Checked" cắt
//      thành "R…". Thành nút icon cạnh ô Replace như VS Code (icon Hugeicons
//      replace / replace-all — bộ icon Samhie). Cả panel: xem mục 15.
//    - Nút Git ở cột icon trái: icon Kate nhúng trong mã (gitIcon(),
//      sc-apps-git.svg — không qua bộ icon) → gắn "samhie-kate-git" (Hugeicons
//      git-branch) lên nút. Nút là KMultiTabBarTab mang tên panel ở thuộc tính
//      kate_original_text (Sidebar::updateButtonStyle, katemdi.cpp).
//
// 14. Quick Open (Ctrl+Alt+O) theo bản phác của user (2026-10-13): MỘT thẻ có
//    lề trong, ô tìm là ô nhập thường (nền tối hơn thẻ) ở trên, kết quả ngay
//    trên nền thẻ, mép trái thẳng mép ô tìm. Thẻ + ô đang chọn do style samhie
//    vẽ (mục 69; plugin đánh dấu samhiePopupPanel/samhiePopupList).
//    - Căn GIỮA cửa sổ, rộng như VS Code: min(62% cửa sổ, 600px)
//      (QuickInputController.MAX_WIDTH); cao giữ của Kate (nửa cửa sổ). Kate
//      đặt lại cỡ + chỗ mỗi lần mở và mỗi khi cửa sổ đổi cỡ (setFixedSize rồi
//      move, KateQuickOpen::updateViewGeometry, Kate 26.08.1) → đặt lại ở
//      Show/Resize/Move.
//    - Dòng kết quả cắt "…" khi dài: delegate của Kate vẽ một QTextLine rộng
//      bằng ô nhưng dời phải 25px — chữ dài bị ngắt ở chỗ xuống dòng được,
//      phần sau mất hẳn, không có "…" (Utils::paintItemViewText). Plugin thay
//      delegate: tên file + đường dẫn xám (nhỏ hơn 1pt như Kate) cắt "…" ở
//      cuối; chữ trong tên khớp chuỗi gõ (đoạn liền, không có thì lần lượt
//      từng ký tự; không phân biệt hoa thường — Kate dùng kfts, chọn chỗ khớp
//      khác đôi chút) in đậm
//      màu link như Kate. Kate còn in đậm chỗ khớp trong đường dẫn — bỏ.
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
//      Chưa mở thư mục (thẻ chào, mục 11): thẳng dòng chữ của thẻ — Kvantum
//      cắt cụt "No Folder Openec" không "…"; chữ đổi thành "No Folder".
//
// 15. Panel Search gọn như VS Code (user 2026-10-13: "nhiều thứ, không đủ
//    chỗ"; chưa dùng chức năng nào → Claude sắp xếp). Bố cục dọc:
//        [›] [Find              ▾]
//            [Replace    ] [⇄] [⇄⇄]   ← chỉ hiện khi bấm › (như VS Code)
//            [Project ▾   ] [Aa] [.*] [⋯] [■]
//    - Ẩn: nút Search (Enter trong ô Find làm việc đó; gõ thì Kate tự tìm
//      trong file đang mở), Next (F6 / Shift+F6), "+" tab tìm mới (lệnh
//      "Search in Files (in new tab)" ở Ctrl+Alt+I), ≣ Expand results (bật
//      sẵn — mọi kết quả mở hết như VS Code), phễu lọc kết quả, ⟳ "dùng thư
//      mục file đang mở" (chọn "In Folder" là Kate tự làm), nút T+ chèn ký tự
//      đặc biệt trong ô Replace.
//    - ■ Stop chỉ hiện lúc Kate đang tìm (stopAndNext lật sang trang Stop).
//    - Aa .* ⋯ cỡ icon + ô hover như nút hàng trên (mục 4: ICON_SIZE,
//      samhieSquareHover) — Kate dựng chúng cao bằng icon (máy ảo user
//      2026-10-13: ô hover Aa 30×16 cạnh ô Project cao 40, "lùn").
//    - ⋯ (Kate: ⚙ "Show more search options") mở trang Folder/Filter/Exclude
//      thay chỗ kết quả. Có dự án thì chỗ tìm mặc định "Project" (VS Code
//      tìm cả thư mục đang mở; Kate mặc định "In Open Files") và chọn dự án
//      thì KHÔNG tự mở trang ⋯ (Kate mở — chỉ cần cho Folder). Ô chỗ tìm
//      bỏ icon, chữ ngắn (Current File / Open Files / Folder / Project /
//      All Projects) — ô chật, "In Open Files" còn "In".
//    - Kết quả là một thẻ như cây Projects (user 2026-10-13: bỏ viền, bo góc
//      nền, kết quả cách mép): thẻ = widget Results (results.ui: cây +
//      ô lọc ẩn), lề trong CARD_GAP, cây không khung + nền trong.
//    - Trang ⋯ xếp dọc như VS Code (user 2026-10-13 — Kate xếp ngang: ở
//      ~250px bốn ô tích mất chữ, "128 MB" đè "Size limit"): nhãn trên ô
//      (Folder / Files to include / Files to exclude), mỗi ô tích một hàng,
//      bỏ vạch ngăn đầu trang.
//    - Kate xếp lưới lại khi panel đổi giữa dọc/ngang
//      (KatePluginSearchView::onResize, trong bộ lọc Resize của nó — chạy SAU
//      bộ lọc ứng dụng của plugin) → xếp lại sau một nhịp. Bố cục ngang
//      (panel kéo xuống đáy): giữ của Kate, chỉ bỏ › và hiện lại ô Replace.

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
#include <QHeaderView>
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
#include <QPainter>
#include <QTextLayout>
#include <QStyledItemDelegate>
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
// Mục 14: Quick Open — bề rộng như VS Code (62% cửa sổ, tối đa 600), lề
// trong của thẻ, khe giữa ô tìm và danh sách
constexpr double QUICK_OPEN_SHARE = 0.62;
constexpr int QUICK_OPEN_MAX_WIDTH = 600;
constexpr int QUICK_OPEN_PAD = 12;
constexpr int QUICK_OPEN_GAP = 8;

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

class NoPaint : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *, QEvent *event) override
    {
        return event->type() == QEvent::Paint;
    }
};

// Dòng đầu cây Projects cách mép trên thẻ CARD_GAP (user 2026-10-13: trước
// dính sát — nét icon cách mép trên 7px, mép trái 18px). Không đặt được bằng
// lề: QTreeView::updateGeometries đặt lại lề trên vùng hiển thị = chiều cao
// header (header ẩn → 0, Qt 6.11.2), còn setContentsMargins của cây đọc lại
// ra 0 ngay (đo container). → hiện header của cây (Kate ẩn nó,
// kateprojectviewtree.cpp) nhưng cao CARD_GAP, không vẽ, không nhận chuột:
// Qt tự chừa đúng khe đó; nền thẻ (style samhie mục 60) lộ qua.
void padTreeTop(QTreeView *tree)
{
    static NoPaint noPaint;
    QHeaderView *header = tree->header();
    header->setFixedHeight(CARD_GAP);
    header->setSectionsClickable(false);
    header->setAttribute(Qt::WA_TransparentForMouseEvents);
    header->setAutoFillBackground(false);
    header->viewport()->setAutoFillBackground(false);
    header->installEventFilter(&noPaint);
    header->viewport()->installEventFilter(&noPaint);
    tree->setHeaderHidden(false);
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

void growIcon(QToolButton *button)
{
    button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    // Ô hover vuông như nút icon khác của Samhie (style samhie mục 68) —
    // style đọc thuộc tính lúc polish → polish lại
    if (!button->property("samhieSquareHover").toBool()) {
        button->setProperty("samhieSquareHover", true);
        button->style()->polish(button);
    }
}

void growIcons(QWidget *parent)
{
    for (auto *button : parent->findChildren<QToolButton *>(Qt::FindDirectChildrenOnly)) {
        growIcon(button);
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

// Mục 14: dòng kết quả Quick Open — tên + đường dẫn cắt "…"
class QuickOpenDelegate : public QStyledItemDelegate
{
public:
    QuickOpenDelegate(QLineEdit *input, QObject *parent)
        : QStyledItemDelegate(parent)
        , m_input(input)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QWidget *view = opt.widget;
        QStyle *style = view ? view->style() : QApplication::style();
        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, view);
        opt.text.clear();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, view); // nền + icon

        // KateQuickOpenModel::FileName / FilePath (Qt::UserRole + 1, + 2)
        const QString name = index.data(Qt::UserRole + 1).toString();
        QString path = index.data(Qt::UserRole + 2).toString();
        const QString suffix = QLatin1Char('/') + name;
        if (path.endsWith(suffix)) {
            path.chop(suffix.size());
        }

        QFont small = opt.font;
        small.setPointSizeF(opt.font.pointSizeF() - 1);
        const QFontMetrics fm(opt.font);
        const QFontMetrics fmSmall(small);
        textRect.setRight(textRect.right() - TEXT_END_PAD);
        const QString shownName = fm.elidedText(name, Qt::ElideRight, textRect.width());
        const int pathRoom = textRect.width() - fm.horizontalAdvance(shownName + QLatin1Char(' '));
        QString shownPath = shownName == name ? fmSmall.elidedText(path, Qt::ElideRight, pathRoom) : QString();
        if (shownPath == QStringLiteral("…")) {
            shownPath.clear();
        }

        QTextCharFormat gray;
        gray.setForeground(opt.palette.color(QPalette::PlaceholderText));
        gray.setFont(small);
        QList<QTextLayout::FormatRange> formats;
        if (!shownPath.isEmpty()) {
            formats.append({.start = int(shownName.size()) + 1, .length = int(shownPath.size()), .format = gray});
        }
        QTextCharFormat match;
        match.setForeground(opt.palette.link());
        match.setFontWeight(QFont::Bold);
        for (int i : matchPositions(shownName)) {
            formats.append({.start = i, .length = 1, .format = match});
        }

        QTextLayout layout(shownPath.isEmpty() ? shownName : shownName + QLatin1Char(' ') + shownPath, opt.font);
        layout.setFormats(formats);
        QTextOption textOption;
        textOption.setWrapMode(QTextOption::NoWrap);
        layout.setTextOption(textOption);
        layout.beginLayout();
        QTextLine line = layout.createLine();
        layout.endLayout();
        if (!line.isValid()) {
            return;
        }
        painter->save();
        painter->setPen(opt.palette.color(QPalette::Text));
        const qreal y = textRect.top() + (textRect.height() - line.height()) / 2;
        layout.draw(painter, QPointF(textRect.left(), y));
        painter->restore();
    }

private:
    // Ký tự trong tên khớp chuỗi gõ (phần sau "/" cuối như Kate): đoạn liền
    // nếu có ("svg" trong "save.svg" → đuôi .svg), không thì lần lượt từng ký
    // tự — khớp không đủ thì không đánh dấu gì
    QList<int> matchPositions(const QString &name) const
    {
        QString pattern = m_input ? m_input->text() : QString();
        pattern = pattern.mid(pattern.lastIndexOf(QLatin1Char('/')) + 1);
        QList<int> positions;
        if (const int at = pattern.isEmpty() ? -1 : name.indexOf(pattern, 0, Qt::CaseInsensitive); at >= 0) {
            for (int i = 0; i < pattern.size(); ++i) {
                positions.append(at + i);
            }
            return positions;
        }
        int from = 0;
        for (const QChar c : std::as_const(pattern)) {
            const int at = name.indexOf(c, from, Qt::CaseInsensitive);
            if (at < 0) {
                return {};
            }
            positions.append(at);
            from = at + 1;
        }
        return positions;
    }

    static constexpr int TEXT_END_PAD = 8; // "…" không dính mép phải ô chọn
    QPointer<QLineEdit> m_input;
};

// Mục 14: chỗ + cỡ (gọi lại sau mỗi lần Kate đặt) và bố cục thẻ (một lần)
void placeQuickOpen(QWidget *w)
{
    QWidget *win = w->parentWidget();
    const int width = std::min(int(win->width() * QUICK_OPEN_SHARE), QUICK_OPEN_MAX_WIDTH);
    if (w->width() != width || w->maximumWidth() != width) {
        w->setFixedWidth(width);
    }
    const QPoint pos = win->rect().center() - QPoint(width / 2, w->height() / 2);
    if (w->pos() != pos) {
        w->move(pos);
    }
    if (w->property("samhiePopupPanel").toBool()) {
        return;
    }
    auto *input = qobject_cast<QLineEdit *>(w->focusProxy());
    auto *list = w->findChild<QTreeView *>();
    if (!input || !list || !w->layout()) {
        return;
    }
    w->layout()->setContentsMargins(QUICK_OPEN_PAD, QUICK_OPEN_PAD, QUICK_OPEN_PAD, QUICK_OPEN_PAD);
    w->layout()->setSpacing(QUICK_OPEN_GAP);
    input->setFrame(true); // ô nhập thường (Kate: không khung)
    list->setFrameShape(QFrame::NoFrame);
    list->viewport()->setAutoFillBackground(false);
    list->setItemDelegate(new QuickOpenDelegate(input, list));
    for (QWidget *marked : {w, static_cast<QWidget *>(list)}) {
        marked->setProperty(marked == w ? "samhiePopupPanel" : "samhiePopupList", true);
        marked->style()->polish(marked);
    }
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

// Mục 15: thẻ kết quả — Results (cha của ResultsTreeView) là thẻ, cây nằm
// trong cách mép CARD_GAP (vùng hiển thị của cây không giữ lề: padTreeTop).
void cardResults(QTreeView *tree)
{
    QWidget *results = tree->parentWidget();
    if (!results || !results->layout() || results->property("samhieCard").toBool()) {
        return;
    }
    results->layout()->setContentsMargins(CARD_GAP, CARD_GAP, CARD_GAP, CARD_GAP);
    tree->setFrameShape(QFrame::NoFrame);
    tree->viewport()->setAutoFillBackground(false);
    markCard(results);
}

// Mục 15: kết quả không ô tích khi ô Replace đang đóng (style samhie mục 71
// đọc samhieNoCheck). Cây kết quả: ResultsTreeView, một cây mỗi tab tìm.
void syncResultChecks(QWidget *dialog)
{
    QWidget *replaceCombo = dialog->findChild<QWidget *>(QStringLiteral("replaceCombo"));
    const bool noCheck = replaceCombo && !replaceCombo->isVisibleTo(dialog);
    for (auto *tree : dialog->findChildren<QTreeView *>()) {
        // Cây của tab đầu được polish trước khi plugin nạp → làm thẻ ở đây
        if (tree->inherits("ResultsTreeView")) {
            cardResults(tree);
        }
        if (tree->inherits("ResultsTreeView") && tree->property("samhieNoCheck").toBool() != noCheck) {
            tree->setProperty("samhieNoCheck", noCheck);
            tree->viewport()->update();
        }
    }
}

// Mục 15: một hàng của panel Search — dải ngang riêng, không chung cột lưới
// với hàng khác (chung cột thì nút Replace rộng bóp ô chỗ tìm còn "⌄").
QWidget *searchRow(QWidget *dialog, const char *name, int spacing, const QList<QWidget *> &widgets)
{
    auto *row = dialog->findChild<QWidget *>(QLatin1String(name), Qt::FindDirectChildrenOnly);
    if (!row) {
        row = new QWidget(dialog);
        row->setObjectName(QLatin1String(name));
        auto *box = new QHBoxLayout(row);
        box->setContentsMargins(0, 0, 0, 0);
        box->setSpacing(spacing);
    }
    auto *box = static_cast<QHBoxLayout *>(row->layout());
    for (QWidget *w : widgets) {
        box->addWidget(w, w == widgets.first() ? 1 : 0); // ô đầu giãn
    }
    return row;
}

// Mục 15: trang ⋯ (search.ui: folderOptions, lưới gridLayout_2 — Kate không
// giữ con trỏ tới các layout, chỉ tới widget) xếp lại dọc.
void arrangeSearchOptions(QWidget *dialog)
{
    QWidget *box = dialog->findChild<QWidget *>(QStringLiteral("folderOptions"));
    if (!box || box->property("samhieArranged").toBool()) {
        return;
    }
    const auto child = [box](const char *name) {
        return box->findChild<QWidget *>(QLatin1String(name));
    };
    auto *folderLabel = qobject_cast<QLabel *>(child("folderLabel"));
    auto *filterLabel = qobject_cast<QLabel *>(child("filterLabel"));
    auto *excludeLabel = qobject_cast<QLabel *>(child("excludeLabel"));
    auto *sizeLabel = qobject_cast<QLabel *>(child("label"));
    QWidget *folder = child("folderRequester");
    QWidget *folderUp = child("folderUpButton");
    QWidget *filter = child("filterCombo");
    QWidget *exclude = child("excludeCombo");
    QWidget *sizeLimit = child("sizeLimitSpinBox");
    if (!folderLabel || !filterLabel || !excludeLabel || !sizeLabel || !folder || !folderUp || !filter || !exclude || !sizeLimit) {
        return;
    }
    box->setProperty("samhieArranged", true);
    if (QWidget *line = child("line")) {
        line->hide();
    }
    const int spacing = box->layout() ? box->layout()->spacing() : 6;
    delete box->layout(); // xoá cả layout con + spacer, widget còn nguyên

    auto *column = new QVBoxLayout(box);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(spacing);
    // Nhãn trên ô, sát ô hơn khe giữa các nhóm
    const auto group = [column, spacing](QLabel *label, const QString &text, QWidget *field) {
        label->setText(text);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        auto *pair = new QVBoxLayout;
        pair->setSpacing(spacing / 2);
        pair->addWidget(label);
        pair->addWidget(field);
        column->addLayout(pair);
    };
    auto *folderRow = new QWidget(box);
    auto *folderBox = new QHBoxLayout(folderRow);
    folderBox->setContentsMargins(0, 0, 0, 0);
    folderBox->setSpacing(spacing);
    folderBox->addWidget(folder, 1);
    folderBox->addWidget(folderUp);
    group(folderLabel, QStringLiteral("Folder"), folderRow);
    group(filterLabel, QStringLiteral("Files to include"), filter);
    group(excludeLabel, QStringLiteral("Files to exclude"), exclude);
    for (const char *name : {"recursiveCheckBox", "hiddenCheckBox", "symLinkCheckBox", "binaryCheckBox"}) {
        if (QWidget *check = child(name)) {
            column->addWidget(check);
        }
    }
    auto *sizeRow = new QHBoxLayout;
    sizeRow->setSpacing(spacing);
    sizeRow->addWidget(sizeLabel, 1);
    sizeRow->addWidget(sizeLimit);
    column->addLayout(sizeRow);
}

// Mục 15: lưới panel Search (search.ui: widget "SearchDialog", lưới
// "gridLayout", 6 cột, cột 0 giãn). Kate đặt ô Replace ở (2, 0, span 5) khi
// vừa xếp dọc, ở (1, 0) khi xếp ngang; bản của plugin: trong dải
// samhieReplaceRow (không nằm thẳng trong lưới).
void arrangeSearchPanel(QWidget *dialog)
{
    auto *grid = dialog->findChild<QGridLayout *>(QStringLiteral("gridLayout"));
    const auto child = [dialog](const char *name) {
        return dialog->findChild<QWidget *>(QLatin1String(name));
    };
    QWidget *searchCombo = child("searchCombo");
    QWidget *replaceCombo = child("replaceCombo");
    auto *replace = qobject_cast<QPushButton *>(child("replaceButton"));
    auto *replaceChecked = qobject_cast<QPushButton *>(child("replaceCheckedBtn"));
    QWidget *options = child("searchOptionsLayoutW");
    QWidget *place = child("searchPlaceLayoutW");
    auto *placeCombo = qobject_cast<QComboBox *>(child("searchPlaceCombo"));
    auto *more = qobject_cast<QToolButton *>(child("displayOptions"));
    auto *stopAndNext = qobject_cast<QStackedWidget *>(child("stopAndNext"));
    auto *stop = qobject_cast<QPushButton *>(child("stopButton"));
    auto *matchCase = qobject_cast<QToolButton *>(child("matchCase"));
    auto *regExp = qobject_cast<QToolButton *>(child("useRegExp"));
    auto *expand = qobject_cast<QToolButton *>(child("expandResults"));
    if (!grid || !searchCombo || !replaceCombo || !replace || !replaceChecked || !options || !place || !placeCombo || !more || !stopAndNext || !stop
        || !matchCase || !regExp || !expand) {
        return;
    }

    // Một lần: icon, nút ẩn, nút › và các nối tín hiệu
    auto *toggle = dialog->findChild<QToolButton *>(QStringLiteral("samhieReplaceToggle"));
    if (!toggle) {
        const auto iconOnly = [](QAbstractButton *b, const char *icon) {
            if (!b->text().isEmpty()) {
                b->setToolTip(b->text());
                b->setText(QString());
            }
            b->setIcon(QIcon::fromTheme(QLatin1String(icon)));
        };
        iconOnly(replace, "edit-find-replace");
        iconOnly(replaceChecked, "samhie-replace-all");
        iconOnly(stop, "process-stop");
        matchCase->setIcon(QIcon::fromTheme(QStringLiteral("samhie-match-case")));
        regExp->setIcon(QIcon::fromTheme(QStringLiteral("samhie-regex")));
        more->setIcon(QIcon::fromTheme(QStringLiteral("samhie-search-details")));
        for (QToolButton *b : {matchCase, regExp, more}) {
            growIcon(b);
        }
        arrangeSearchOptions(dialog);
        for (const char *name : {"searchButton", "newTabButton", "expandResults", "filterBtn", "currentFolderButton"}) {
            if (QWidget *w = child(name)) {
                w->hide();
            }
        }
        // Mọi kết quả mở hết (Kate lưu ExpandSearchResults theo phiên)
        expand->setChecked(true);
        // Nút T+ (chèn \n, \t… vào ô Replace) — action icon "insert-text"
        if (auto *combo = qobject_cast<QComboBox *>(replaceCombo); combo && combo->lineEdit()) {
            for (QAction *a : combo->lineEdit()->actions()) {
                if (a->icon().name() == QLatin1String("insert-text")) {
                    a->setVisible(false);
                }
            }
        }

        // ■ chỉ hiện lúc đang tìm
        stopAndNext->setVisible(stopAndNext->currentWidget() == stop);
        QObject::connect(stopAndNext, &QStackedWidget::currentChanged, stopAndNext, [stopAndNext, stop] {
            stopAndNext->setVisible(stopAndNext->currentWidget() == stop);
        });

        // Chỗ tìm: chữ ngắn, bỏ icon (ô chật: "In Open Files" còn "In") —
        // Kate thêm/bớt hai mục dự án khi mở/đóng dự án nên làm lại mỗi lần
        // thêm. Dự án (index 3 = MatchModel::Project) mặc định khi có, và
        // chọn dự án thì không tự mở trang ⋯: nối SAU Kate → chạy sau
        // searchPlaceChanged (nó bật ⋯ khi đổi bằng combo).
        constexpr int PROJECT = 3;
        const auto tidyPlaces = [placeCombo] {
            static const QStringList names{QStringLiteral("Current File"),
                                           QStringLiteral("Open Files"),
                                           QStringLiteral("Folder"),
                                           QStringLiteral("Project"),
                                           QStringLiteral("All Projects")};
            for (int i = 0; i < placeCombo->count() && i < names.size(); ++i) {
                placeCombo->setItemText(i, names[i]);
                placeCombo->setItemIcon(i, QIcon());
            }
            if (placeCombo->count() > PROJECT && placeCombo->currentIndex() < 2) { // Current File / Open Files
                placeCombo->setCurrentIndex(PROJECT);
            }
        };
        QObject::connect(placeCombo, &QComboBox::currentIndexChanged, more, [more](int index) {
            if (index >= PROJECT) {
                more->setChecked(false);
            }
        });
        QObject::connect(placeCombo->model(), &QAbstractItemModel::rowsInserted, placeCombo, tidyPlaces, Qt::QueuedConnection);
        tidyPlaces();

        // › / ⌄ mở ô Replace. Không dùng nút checkable: nút đang "bật" có
        // nền như nút được nhấn — VS Code chỉ đổi mũi tên.
        toggle = new QToolButton(dialog);
        toggle->setObjectName(QStringLiteral("samhieReplaceToggle"));
        toggle->setAutoRaise(true);
        toggle->setToolTip(QStringLiteral("Toggle Replace"));
        toggle->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
        toggle->setFixedWidth(20);
        toggle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred); // cao theo hàng, không đòi thêm
        QObject::connect(toggle, &QToolButton::clicked, dialog, [dialog, grid, toggle] {
            QWidget *row = dialog->findChild<QWidget *>(QStringLiteral("samhieReplaceRow"), Qt::FindDirectChildrenOnly);
            if (!row) {
                return;
            }
            const bool on = row->isHidden();
            row->setVisible(on);
            toggle->setIcon(QIcon::fromTheme(on ? QStringLiteral("arrow-down") : QStringLiteral("arrow-right")));
            // Mở thì › cao hết hai ô như VS Code; đóng thì chỉ hàng Find
            // (› nằm ở hàng 1 thì hàng đó không còn trống → vẫn chừa khe)
            grid->addWidget(toggle, 0, 0, on ? 2 : 1, 1);
            if (on) {
                dialog->findChild<QWidget *>(QStringLiteral("replaceCombo"))->setFocus();
            }
            syncResultChecks(dialog);
        });
    }

    const int index = grid->indexOf(replaceCombo);
    if (index < 0) { // đã xếp rồi (ô Replace nằm trong dải của plugin)
        return;
    }
    int row, col, rowSpan, colSpan;
    grid->getItemPosition(index, &row, &col, &rowSpan, &colSpan);
    if (row == 1 && col == 0) { // Kate vừa xếp ngang (kéo các ô ra khỏi dải): giữ của Kate
        toggle->hide();
        for (const char *name : {"samhieReplaceRow", "samhiePlaceRow"}) {
            if (QWidget *w = dialog->findChild<QWidget *>(QLatin1String(name), Qt::FindDirectChildrenOnly)) {
                w->hide();
            }
        }
        grid->setColumnStretch(0, 1);
        grid->setColumnStretch(1, 0);
        syncResultChecks(dialog);
        return;
    }
    // Dọc: cột 0 ›, cột 1 giãn — mỗi hàng dưới là một dải
    const int spacing = grid->horizontalSpacing();
    QWidget *replaceRow = searchRow(dialog, "samhieReplaceRow", spacing, {replaceCombo, replace, replaceChecked});
    QWidget *placeRow = searchRow(dialog, "samhiePlaceRow", spacing, {place, options, more, stopAndNext});
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    grid->addWidget(searchCombo, 0, 1, 1, 5);
    grid->addWidget(replaceRow, 1, 1, 1, 5);
    grid->addWidget(placeRow, 2, 1, 1, 5);
    const bool on = toggle->icon().name() == QLatin1String("arrow-down");
    replaceRow->setVisible(on);
    placeRow->show();
    toggle->show();
    grid->addWidget(toggle, 0, 0, on ? 2 : 1, 1);
    syncResultChecks(dialog);
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

        // Mục 4: ☰ › File › Close Folder = bấm nút ✕ đầu khung Projects
        m_closeFolder = a = actionCollection()->addAction(QStringLiteral("samhie_close_folder"));
        a->setText(QStringLiteral("Close Folder"));
        a->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
        a->setEnabled(false);
        connect(a, &QAction::triggered, this, [this] {
            if (m_closeFolderButton) {
                m_closeFolderButton->click();
            }
        });

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
            if (obj->inherits("ResultsTreeView")) { // mục 15: tab tìm mới
                cardResults(static_cast<QTreeView *>(obj));
                for (QWidget *w = static_cast<QWidget *>(obj)->parentWidget(); w; w = w->parentWidget()) {
                    if (w->objectName() == QLatin1String("SearchDialog")) {
                        syncResultChecks(w);
                        break;
                    }
                }
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
                placeQuickOpen(static_cast<QWidget *>(obj)); // mục 14
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
                // Khung focus cyan cả dòng khi cây giữ focus (bấm vào cây, bấm
                // chỗ trống): QTreeView::drawRow vẽ PE_FrameFocusRect KHÔNG kèm
                // widget khi allColumnsShowFocus (Kate bật,
                // kateprojectviewtree.cpp) → style không chặn được. Tắt thì
                // focus về lại ô (State_HasFocus) — style samhie bỏ (mục 70).
                if (auto *view = qobject_cast<QTreeView *>(tree)) {
                    view->setAllColumnsShowFocus(false);
                }
                markCard(tree);
                if (auto *view = qobject_cast<QTreeView *>(tree)) {
                    padTreeTop(view);
                }
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
            if (event->type() == QEvent::Resize && isQuickOpen(obj)) {
                placeQuickOpen(static_cast<QWidget *>(obj)); // mục 14
            }
            if (event->type() == QEvent::Resize && obj->objectName() == QLatin1String("SearchDialog")) {
                auto *dialog = static_cast<QWidget *>(obj);
                QTimer::singleShot(0, dialog, [dialog] {
                    arrangeSearchPanel(dialog); // mục 15
                });
            }
            break;
        case QEvent::EnabledChange:
            if (obj == m_closeFolderButton) {
                m_closeFolder->setEnabled(m_closeFolderButton->isEnabled()); // mục 4
            }
            break;
        case QEvent::Move:
            if (isQuickOpen(obj)) {
                placeQuickOpen(static_cast<QWidget *>(obj)); // mục 14
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
    // ✕ đóng dự án giữ lại (mục 4), thêm « thu khung (hideToolView nhận đúng widget mà createToolView
    // trả về — ô chọn dự án là con trực tiếp của nó)
    void fixProjectHeader(QHBoxLayout *row, QWidget *toolView)
    {
        // Nút ✕ đóng dự án (mục 4)
        if (auto *close = qobject_cast<QToolButton *>(row->itemAt(2)->widget())) {
            close->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
            close->setToolTip(QStringLiteral("Close Folder"));
            m_closeFolderButton = close;
            m_closeFolder->setEnabled(close->isEnabled());
        }
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
    // (✕ đóng dự án cũng ẩn — đặt m_closeFolderButton TRƯỚC khi gọi hàm này)
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
            m_projectCombo->setPlaceholderText(QStringLiteral("No Folder")); // ngắn cho vừa ô (thẻ chào nói rõ)
        }
        const auto update = [this, stack, reload = QPointer<QWidget>(reload)] {
            const bool empty = stack->count() == 0;
            stack->setVisible(!empty);
            m_welcome->setVisible(empty);
            if (reload) {
                reload->setVisible(!empty);
            }
            if (m_closeFolderButton) {
                m_closeFolderButton->setVisible(!empty); // ✕ (mục 4) cũng vậy
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
        // Chữ tiêu đề: thẳng icon thư mục cấp gốc (mục 13) — chưa mở thư mục
        // thì thẳng dòng chữ của thẻ chào (mục 11)
        int textLeft = -1;
        if (auto *tree = qobject_cast<QTreeView *>(sideCard)) {
            textLeft = rootIconLeft(tree, win);
        } else if (auto *label = sideCard->findChild<QLabel *>()) {
            textLeft = label->mapTo(win, label->contentsRect().topLeft()).x();
        }
        if (textLeft >= 0) {
            const int indent = textLeft - m_projectCombo->mapTo(win, QPoint(0, 0)).x();
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
    QPointer<QToolButton> m_closeFolderButton; // mục 4: ✕ của Kate
    QAction *m_closeFolder = nullptr;
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
