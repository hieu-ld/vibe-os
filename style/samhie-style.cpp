// Widget style "samhie": bọc theme Kvantum (QProxyStyle) để vẽ thêm vài chỗ
// mà theme Kvantum KHÔNG vẽ được bằng SVG/kvconfig — đã đọc mã Kvantum 1.1.6,
// xem docs/PLAN.md giai đoạn 4. Mọi thứ còn lại chuyển nguyên cho Kvantum.
//
//   1. Khung thanh đường dẫn Dolphin. KUrlNavigator vẽ khung bằng
//      PE_FrameLineEdit, Kvantum bỏ qua lệnh này (chỉ vẽ cho LibreOffice).
//      → vẽ bằng PE_PanelLineEdit của Kvantum: y hệt ô nhập chữ.
//   2. Sidebar Places (KFilePlacesView): trỏ chuột / đang chọn CHỈ đổi màu chữ,
//      không tô nền; lùi vào thêm SIDEBAR_INDENT px (lề vùng hiển thị).
//      Màu chữ: KFilePlacesViewDelegate::paint (KIO 6.30) chép option ra một
//      bản CỤC BỘ, gọi style vẽ nền mục (PE_PanelItemViewItem) bằng bản đó,
//      RỒI mới lấy màu chữ/icon từ chính bản đó — nên lúc vẽ nền ta sửa màu
//      trong bản sao (icon: KIconLoader đọc WindowText/HighlightedText). KHÔNG bọc delegate: KIO tìm delegate bằng
//      qobject_cast<KFilePlacesViewDelegate*>(itemDelegate()) và có chỗ không
//      kiểm rỗng → Dolphin crash khi bấm sidebar (máy ảo 2026-10-03, gdb:
//      KFilePlacesViewDelegate::pointIsHeaderArea).
//   3. Vùng nội dung của khung tab kiểu "document" (vùng file Dolphin, vùng
//      chữ Konsole) nền card, trên nền cửa sổ secondary — bản vẽ user. Nền
//      cửa sổ do Kvantum tô (window.color); vùng nội dung tô ở đây, ĐÈ hẳn
//      (CompositionMode_Source) để giữ đúng độ trong thay vì chồng hai lớp.
//      Kvantum chừa khe 2px giữa dải tab và nội dung (SE_TabWidgetTabContents)
//      — khe nằm trong vùng tô nên cùng màu nội dung.
//   4. Dolphin: vùng file có viền trên + trái, bo góc trên-trái (bản vẽ: 12px
//      như góc cửa sổ) — thay vạch tay nắm dock của Kvantum. Thanh trạng thái
//      Dolphin tự tô ô nền bằng màu cửa sổ → đổi sang màu vùng file.
//   5. Nút ở góc dải tab (+ của Konsole) cao bằng dải tab — thấp hơn thì Qt
//      dồn nó xuống đáy (đo container 2026-10-03).
//   6. Nút chỉ-có-icon trên thanh công cụ VUÔNG, icon giữa (Kvantum cho ô
//      hover 32×22 lệch tâm — máy ảo 2026-10-03).
//   7. Thanh tìm Dolphin (Search::Bar) trông như một popup: thẻ bo góc có
//      viền, cách mép vùng file (bản vẽ user). Nó vẫn nằm TRONG bố cục, trượt
//      xuống và đẩy file xuống như cũ — Dolphin dựng nó vậy, style không đổi
//      được chỗ đặt.
//   9. Dolphin: nút + (tab mới) ở góc trái dải tab, như Konsole — Dolphin không
//      có nút này. Tab mới (nút +, Ctrl+T, menu) mở thư mục Home thay vì thư
//      mục đang xem (user 2026-10-04); "Open in New Tab" vẫn mở thư mục đó. Dải tab + nút lùi vào DOLPHIN_TAB_INSET px: viền trái vùng
//      file nằm ở cột 0 và góc khung bo 12, tab bo 8 — không lùi thì tab đè
//      lên viền, góc tab lòi ra ngoài đường cong khung (máy ảo 2026-10-04).
//      Tab Dolphin không icon (bản vẽ, như Konsole).
//  11. Tab đang mở trong như vùng nội dung (tô đè, không chồng lớp).
//  10. Dòng menu cao bằng dòng sidebar (bỏ khoản "font + 8" của Kvantum);
//      dòng widget trong menu (Zoom của Dolphin) cũng cao bằng.
//   8. Ô file Dolphin / mục danh sách không có khung focus (khung của Kvantum
//      bo 2px vẽ đè thành góc vuông; ô đầu thư mục luôn có viền lơ lửng).
//  12. Nút trong dòng widget của menu (nút "Other" màu thư mục, ô màu, Zoom)
//      bấm được cả khi menu con còn mở. Lúc đó menu con giữ chuột; nó chuyển
//      cú bấm cho MENU CHA (QMenuPrivate::mouseEventTaken) chứ không cho nút,
//      menu cha coi như chọn cả dòng (QWidgetAction) → đóng hết. User đi từ
//      "Compress" lên "Other" rồi bấm ngay (máy ảo 2026-10-04). → bộ lọc trên
//      QMenu: cú bấm/thả tới chính menu mà dưới chuột là một nút thì đóng menu
//      con, chuyển cho nút (MenuButtonForwarder). Bình thường cú bấm đi thẳng
//      vào nút, menu không thấy — bộ lọc chỉ chạy ở trường hợp chuyển tiếp.
//      Thêm: menu con đóng sau SUBMENU_SLOPPY_MS thay vì 1000ms của Kvantum.
//  13. Dolphin chế độ Details không kẻ sọc. KItemListWidget::paint tự tô dòng
//      xen kẽ bằng fillRect(AlternateBase) — không qua style — mà vùng file
//      trong (Kvantum transparent_dolphin_view) → dòng sọc đục #171717 giữa
//      các dòng thấy hình nền (container 2026-10-05). AlternateBase trong
//      suốt trên QGraphicsView + scene của vùng file: KItemListView lấy
//      palette từ view lúc vào scene, sau đó từ scene (PaletteChange).
//  14. Tiêu đề cột Dolphin (Details) — KItemListHeaderWidget vẽ từng cột +
//      phần đệm hai bên bằng CE_Header, widget = viewport (máy ảo 2026-10-05):
//      - nền đục như vùng file: hàng cuộn xuống nằm dưới tiêu đề (z 10)
//        không còn lộ chữ qua tiêu đề trong suốt;
//      - ô hover cột đầu kéo tới mép trái vùng file, cột cuối tới mép phải
//        (Dolphin vẽ phần đệm thành ô riêng, ngay TRƯỚC cột kề nó). Chỉ ô
//        nền kéo dài (CE_HeaderSection); chữ + mũi tên sắp xếp vẽ ở chỗ cũ;
//      - chữ "Size"/"Permissions" căn phải như số liệu bên dưới (Dolphin
//        căn phải hai cột này — KFileItemListWidget::isRoleRightAligned —
//        còn tiêu đề thì căn trái). Nhận ra theo chữ: giao diện tiếng Anh;
//      - cao thêm HEADER_GAP ở dưới, vẽ trong phần trên: dòng đầu cách ô
//        hover đúng bằng khe từ mép trên vùng file tới ô hover (đo 10px).
//      Ô hover/chọn của dòng file (Details) cũng rộng bằng, và VUÔNG (user
//      2026-10-05; chế độ Icons giữ ô bo của Kvantum): Dolphin đưa
//      PE_PanelItemViewItem khung từ lề trái tới hết các cột → style tự tô
//      cả bề ngang viewport (= ô hover tiêu đề: viewport tự cách viền vùng
//      file 7px — đo container 2026-10-05, cả hai 630…1528), cùng màu phần tử
//      itemview của Kvantum (trỏ = nền hover, chọn = nền hover + viền
//      accent). Nhận ra dòng Details theo bề ngang (> 60% vùng file; ô chế
//      độ Icons hẹp hơn nhiều).
//      Cột Size/Modified rộng cố định (user 2026-10-05: đổi thư mục là cột
//      xê dịch): Dolphin cho mỗi cột rộng tối thiểu = chữ tiêu đề + grip +
//      2 × PM_HeaderMargin (KItemListView::preferredColumnWidths, hỏi KHÔNG
//      kèm option/widget), còn lại theo chữ dài nhất trong thư mục ("Yesterday
//      at 20:37" < "29/09/2026 at 20:38" → lệch 7px). Trả lề đủ để "Modified"
//      tối thiểu bằng ngày giờ dài nhất có thể → cột không đổi theo thư mục.
//  15. Dòng trạng thái nhỏ của Dolphin (nổi ở góc dưới-trái vùng file) tô
//      nền đè lên viền trái vùng file → vẽ lại đoạn viền đó.
//  16. Nút bấm (QPushButton) trong dòng widget của menu — hàng màu thư mục
//      của Dolphin (setfoldericonitemaction: 5 ô màu chỉ icon + "Other"):
//      - ô chỉ icon cao bằng nút có chữ: Kvantum tính theo icon 16px → ô
//        lùn hơn hẳn "Other" (chữ Dongle cao; ảnh user 2026-10-05);
//      - không có khung focus: bấm Other thì nút nhận focus, khung focus
//        của Kvantum (accent@0.7) bị viền nút che trên/dưới, chỉ còn hai sọc
//        dọc xanh (đo ảnh user: #0d93af ở hai cột). Focus BÀN PHÍM (đi bằng
//        Tab/mũi tên trong hàng) hiện bằng nền hover, như dòng menu.
//  17. Menu chuột phải Dolphin khi bấm VÀO một thư mục không có "Create New"
//      (user 2026-10-05: tạo file BÊN TRONG thư mục đó, dễ nhầm với bấm chỗ
//      trống). Dolphin không có tuỳ chọn (các mục khác tắt bằng /etc/xdg/
//      dolphinrc [ContextMenu]). Menu con có icon list-add: menu chỗ trống
//      đặt nó ĐẦU TIÊN (DolphinContextMenu::addViewportContextMenu, 26.08),
//      menu thư mục đặt sau Open in…/Open With → cái không đứng đầu thì ẩn.
//      Làm lúc polish: QMenu::popup gọi ensurePolished trước khi tính cỡ.
//      Menu ☰ cũng không có "Create New" (trùng chuột phải chỗ trống). Menu
//      con đó là MỘT với menu chỗ trống (hành động "new_menu" của cửa sổ,
//      DolphinMainWindow::m_newFileMenu) → gỡ khỏi menu ☰ (removeAction),
//      không ẩn hành động. Menu ☰ hiện ra là bản CHÉP KHamburgerMenu dựng
//      (QMenu không cha, khamburgermenu.cpp newMenu) — dựng mới khi Dolphin
//      đổi danh sách → gỡ lúc polish + mỗi aboutToShow. Mọi QMenu không cha
//      của Dolphin (trừ menu chuột phải) đều gỡ — kể cả File trong More.
//      Menu chuột phải CHỖ TRỐNG không có hàng màu thư mục: ở đó nó đổi màu
//      THƯ MỤC ĐANG MỞ — dễ nhầm, đã có ở menu của thư mục và hộp thoại tạo
//      thư mục (user 2026-10-05). Menu chỗ trống = menu có Create New của cửa
//      sổ ("new_menu"); hàng màu = QWidgetAction có nút kèm menu (Other).
//      QMenu tạo widget cho QWidgetAction ngay lúc thêm hành động → lúc
//      polish đã có. Gỡ khỏi menu (removeAction) chứ không ẩn.
//  18. Thanh công cụ Dolphin không có menu chuột phải (Text Position, Icon
//      Size, Configure Toolbars, Lock — bấm nhầm là hỏng thanh theo bản vẽ;
//      user 2026-10-05). Ô đường dẫn trong thanh vẫn có menu riêng.
//  19. Bớt menu chuột phải tab và sidebar Dolphin (user 2026-10-05; Dolphin
//      26.08 / KIO 6.30 không có tuỳ chọn). Nhận mục theo TÊN ICON — chữ thì
//      đổi theo ngôn ngữ. Gỡ khỏi menu (removeAction), hành động vẫn còn.
//      - Tab (DolphinTabBar::contextMenuEvent, QMenu cha = dải tab): chỉ giữ
//        Close Other Tabs (tab-close-other). New/Close Tab trùng nút + / ×.
//      - Sidebar (KFilePlacesView::contextMenuEvent): QMenu KHÔNG cha, polish
//        ngay lúc tạo khi còn trống → bớt lúc aboutToShow (đủ mục, cả mục
//        Dolphin thêm). Nhận ra: có mục icon "hint" (Hide / Hide Section) hoặc
//        hành động lock_panels (menu chỗ trống). Bỏ Open in New Window
//        (window-new, menu thư mục cũng đã tắt), Properties
//        (document-properties), Hide Section (mục "hint" CUỐI: mục sidebar
//        có Hide rồi Hide Section, tiêu đề nhóm chỉ có Hide Section — sidebar
//        chỉ một nhóm, ẩn là trống trơn), Icon Size (menu con duy nhất; cỡ 20
//        đặt ở /etc/xdg/dolphinrc), Unlock Panels (lock_panels — mở khoá là
//        hiện thanh tiêu đề panel, phá bố cục).
//  20. Thanh đường dẫn Dolphin (KUrlNavigator, user 2026-10-05):
//      - trỏ vào tên thư mục / mũi tên › : KHÔNG tô ô, chữ + mũi tên đổi sang
//        HOVER_ACCENT. KIO vẽ chữ bằng palette Text của chính nút, ép độ đục
//        (thư mục cha 50% khi trỏ, thư mục đang mở 100% — foregroundColor,
//        KIO 6.30) → đổi Text của nút lúc chuột vào/ra (UrlButtonHover).
//      - ô sửa đường dẫn (bấm vào thanh) không có viền focus cyan.
//      - nút ✓ (thoát chế độ sửa): ô hover VUÔNG như nút hàng xóm.
//      - ô tìm của Dolphin (Search::Bar) cũng không viền focus cyan.
//  22. Dấu nháy ô nhập chữ ngắn lại (user 2026-10-05). Qt vẽ dấu nháy cao
//      bằng CẢ hộp dòng của font — Dongle khai ascent+descent 1,448 em mà chữ
//      hoa chỉ cao 0,405 em → nháy 29px cạnh chữ ~8px. Qt không cho đổi chiều
//      cao (QTextLayout::drawCursor) → PM_TextCursorWidth = 0 (Qt vẫn chạy
//      nháy nhưng không vẽ gì) và một lớp con trong suốt (ShortCaret) vẽ dấu
//      nháy theo chiều cao chữ: CARET_UP × cap trên chân chữ, CARET_DOWN ×
//      cap dưới. Chỉ ô nhập dùng Dongle (font giao diện).
//  21. Ô hover vuông, icon + TOOL_PADDING mỗi phía, ở giữa nút: nút + của dải
//      tab Dolphin (cao bằng dải tab — mục 5) và nút icon trên thanh công cụ
//      Dolphin (☰ do KHamburgerMenu dựng cao bằng thanh, 34×42 — container),
//      nút × của thanh tìm (Dolphin xếp nó lùn hơn icon: ô hover 26×14 —
//      container) — nút đó được nới tối thiểu bằng ô vuông.
//
// Kvantum chủ động hỗ trợ bị bọc trong QProxyStyle (Kvantum.cpp
// setSurfaceFormat: "qt5ct uses QProxyStyle").

#include <QAbstractButton>
#include <QApplication>
#include <QFontMetricsF>
#include <QtMath>
#include <QAbstractItemView>
#include <QAction>
#include <QDir>
#include <QAbstractScrollArea>
#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QPainterPath>
#include <QLayout>
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
#include <QLineEdit>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QWidgetAction>

#include <algorithm>

#include "colors.h"  // sinh từ theme/palette.toml (tools/gen-theme.py)

namespace {

constexpr int SIDEBAR_INDENT = 8;  // bản vẽ: icon sidebar cách mép ~16px (KIO tự chừa 4)
constexpr int PANE_RADIUS = 12;    // góc viền vùng file Dolphin (bản vẽ ~12) = góc cửa sổ
constexpr int TOOL_PADDING = 6;    // nút icon trên thanh công cụ: icon + 6px mỗi phía
constexpr int POPUP_MARGIN = 8;    // thẻ tìm cách mép vùng file
constexpr int POPUP_PADDING = 10;  // nội dung cách mép thẻ
constexpr int POPUP_RADIUS = 12;
// = PANE_RADIUS − góc tab (8, tools/kvantum.py) → góc tab/nút + đồng tâm với
// góc khung; cũng bằng khe trên của tab (TAB_INSET) nên lùi đều hai phía.
constexpr int DOLPHIN_TAB_INSET = 4;
constexpr int MENUITEM_FRAME = 1;  // khung trên/dưới của MenuItem (tools/kvantum.py frame top=1)
// Mục 12. Kvantum: SH_Menu_SubMenuSloppyCloseTimeout 1000 (đo bằng chương
// trình nhỏ trong image), mở menu con sau 250. Vẫn đủ để rê chéo vào menu con.
constexpr int SUBMENU_SLOPPY_MS = 300;
// Mục 14: khe trên ô hover tiêu đề cột (Kvantum) đo trong máy ảo 2026-10-05.
constexpr int HEADER_GAP = 10;
// Mục 20: nút ✓ của thanh đường dẫn — icon 16 (KIconLoader::SizeSmall) + đệm.
constexpr int TOGGLE_SIDE = 16 + 2 * TOOL_PADDING;

// setViewportMargins là protected — gọi qua lớp con chỉ để mở quyền truy cập.
class ScrollAreaAccess : public QAbstractScrollArea
{
public:
    using QAbstractScrollArea::setViewportMargins;
};

// createdWidgets là protected — cùng cách trên (mục 17).
class WidgetActionAccess : public QWidgetAction
{
public:
    using QWidgetAction::createdWidgets;
};

// Nút trên thanh đường dẫn: tên thư mục, mũi tên đầu thanh, nút ✎/✓ (KIO).
bool isUrlNavigatorButton(const QWidget *w)
{
    return w && w->inherits("KDEPrivate::KUrlNavigatorButtonBase");
}

// Trong thanh đường dẫn hoặc thanh tìm của Dolphin (mục 20).
bool inDolphinLocationOrSearch(const QWidget *w)
{
    for (; w; w = w->parentWidget()) {
        if (w->inherits("KUrlNavigator") || w->inherits("Search::Bar"))
            return true;
    }
    return false;
}

// Ô vuông cạnh `side` ở giữa `r` (không lớn hơn r).
QRect centeredSquare(const QRect &r, int side)
{
    side = qMin(side, qMin(r.width(), r.height()));
    return QRect(r.x() + (r.width() - side) / 2, r.y() + (r.height() - side) / 2, side, side);
}

bool isPlacesView(const QWidget *w)
{
    return w && w->inherits("KFilePlacesView");
}

// Vùng file của Dolphin: KItemListWidget vẽ với widget = viewport của
// KItemListContainerViewport (QGraphicsView) nằm trong KItemListContainer.
// Danh sách Qt thường: delegate đưa chính QAbstractItemView.
bool isItemView(const QWidget *w)
{
    for (int i = 0; w && i < 3; ++i, w = w->parentWidget()) {
        if (qobject_cast<const QAbstractItemView *>(w) || w->inherits("KItemListContainer"))
            return true;
    }
    return false;
}

// Vùng file Dolphin (không tính QHeaderView của danh sách Qt thường).
bool isDolphinView(const QWidget *w)
{
    for (int i = 0; w && i < 3; ++i, w = w->parentWidget()) {
        if (w->inherits("KItemListContainer"))
            return true;
    }
    return false;
}

// Mục 16: nút trong dòng widget của menu (nút → widget dòng → QMenu).
bool isMenuRowButton(const QWidget *w)
{
    return qobject_cast<const QPushButton *>(w) && w->parentWidget()
        && qobject_cast<const QMenu *>(w->parentWidget()->parentWidget());
}

// Khung nội dung của thanh tìm Dolphin: Search::Bar (AnimatedHeightWidget)
// → QScrollArea → viewport → khung nội dung (widget của QScrollArea).
bool isSearchContents(const QWidget *w)
{
    const QWidget *vp = w ? w->parentWidget() : nullptr;
    auto *area = vp ? qobject_cast<const QScrollArea *>(vp->parentWidget()) : nullptr;
    return area && area->widget() == w && area->parentWidget()
        && area->parentWidget()->inherits("Search::Bar");
}

// Tab mới ở thư mục Home, cuối dải (như trình duyệt và như Ctrl+T gốc
// của Dolphin), focus vào vùng file của tab mới. Gọi slot công khai
// DolphinTabWidget::openNewActivatedTab(QUrl, QUrl) qua meta-object.
void openHomeTab(QTabWidget *tw)
{
    const QUrl home = QUrl::fromLocalFile(QDir::homePath());
    if (!QMetaObject::invokeMethod(tw, "openNewActivatedTab", Q_ARG(QUrl, home), Q_ARG(QUrl, QUrl())))
        return;
    tw->tabBar()->moveTab(tw->currentIndex(), tw->count() - 1);
    const auto views = tw->currentWidget()->findChildren<QWidget *>();
    for (QWidget *v : views) {
        if (v->inherits("KItemListContainer")) {
            v->setFocus();
            break;
        }
    }
}

// Hành động "new_tab" của cửa sổ Dolphin (KActionCollection đặt
// objectName = tên hành động). Nó KHÔNG là con QObject của cửa sổ
// (findChild không thấy — container) mà được gắn vào cửa sổ
// (addAssociatedWidget → QWidget::actions()) để phím tắt chạy.
QAction *newTabAction(QWidget *window)
{
    const auto actions = window->actions();
    for (QAction *action : actions) {
        if (action->objectName() == QLatin1String("new_tab"))
            return action;
    }
    return nullptr;
}

// Lần đầu tab widget Dolphin hiện (cửa sổ đã dựng xong, hành động chắc chắn
// có): bỏ kết nối new_tab.triggered → cửa sổ (DolphinMainWindow::
// openNewActivatedTab = mở thư mục đang xem), nối sang openHomeTab. Tự huỷ.
class NewTabRewire : public QObject
{
public:
    explicit NewTabRewire(QTabWidget *tw) : QObject(tw) {}

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() != QEvent::Show)
            return false;
        auto *tw = static_cast<QTabWidget *>(obj);
        QWidget *window = tw->window();
        if (QAction *action = newTabAction(window)) {
            QObject::disconnect(action, &QAction::triggered, window, nullptr);
            QObject::connect(action, &QAction::triggered, tw, [tw] { openHomeTab(tw); });
        }
        obj->removeEventFilter(this);
        deleteLater();
        return false;
    }
};

// Mục 13. Chỉ khoá AlternateBase — vai trò khác vẫn theo palette app (đổi
// Dark mode).
template<typename T>
void clearAlternateBase(T *target, const QPalette &from)
{
    QPalette pal = from;
    for (auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled})
        pal.setColor(group, QPalette::AlternateBase, Qt::transparent);
    target->setPalette(pal);
}

QColor paneColor()
{
    QColor c(samhie::PANE);
    c.setAlphaF(samhie::WINDOW_OPACITY);
    return c;
}

// Tô vùng nội dung của QTabWidget kiểu document (+ viền ở Dolphin) TRƯỚC khi
// QTabWidget và con của nó tự vẽ — con có nền trong suốt nên thấy lớp này.
class PanePainter : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Paint)
            paint(static_cast<QTabWidget *>(obj));
        return false;
    }

private:
    static void paint(QTabWidget *tw)
    {
        const bool dolphin = tw->inherits("DolphinTabWidget");
        const QRect all = tw->rect();
        // Vùng tô: dưới dải tab (nếu dải đang hiện) tới đáy.
        QRect pane = all;
        const QTabBar *bar = tw->tabBar();
        if (bar && bar->isVisible() && tw->tabPosition() == QTabWidget::North)
            pane.setTop(bar->geometry().bottom() + 1);

        QPainter p(tw);
        p.setRenderHint(QPainter::Antialiasing);
        // Dolphin: góc bo ở ĐỈNH khung. Có dải tab thì dải nằm trong khung,
        // giữ nền cửa sổ — vùng file khi đó vuông góc.
        const QPainterPath fill = dolphin && pane.top() == all.top()
            ? topLeftRounded(QRectF(pane), true)
            : rectPath(QRectF(pane));
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillPath(fill, paneColor());
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        if (dolphin) {
            // Viền trên + trái, nét 1px rơi đúng giữa hàng điểm ảnh. Không
            // khép đáy/phải: đó là mép cửa sổ.
            p.setPen(QPen(QColor(samhie::FRAME_LINE), 1));
            p.setBrush(Qt::NoBrush);
            p.drawPath(topLeftRounded(QRectF(all).adjusted(0.5, 0.5, 0, 0), false));
        }
    }

    static QPainterPath rectPath(const QRectF &r)
    {
        QPainterPath path;
        path.addRect(r);
        return path;
    }

    // Cạnh trái + góc bo + cạnh trên; closed = khép thành vùng tô.
    static QPainterPath topLeftRounded(const QRectF &r, bool closed)
    {
        const qreal rad = PANE_RADIUS, d = 2 * rad;
        QPainterPath path;
        path.moveTo(r.left(), r.bottom() + 1);
        path.lineTo(r.left(), r.top() + rad);
        path.arcTo(QRectF(r.left(), r.top(), d, d), 180, -90);
        path.lineTo(r.right() + 1, r.top());
        if (closed) {
            path.lineTo(r.right() + 1, r.bottom() + 1);
            path.closeSubpath();
        }
        return path;
    }
};

// Thẻ popup sau nội dung thanh tìm (vẽ trước các con của khung nội dung).
class PopupPainter : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        auto *w = static_cast<QWidget *>(obj);
        if (event->type() == QEvent::Paint) {
            QPainter p(w);
            p.setRenderHint(QPainter::Antialiasing);
            // Mép dưới thẻ = mép dưới thanh: Dolphin tính chiều cao thanh với
            // đúng 4px dưới dòng nút thứ hai (Bar::preferredHeight), nên
            // không nới lề dưới được.
            const QRectF card = QRectF(w->rect()).adjusted(POPUP_MARGIN + 0.5, POPUP_MARGIN + 0.5,
                                                           -POPUP_MARGIN - 0.5, -0.5);
            p.setPen(QPen(QColor(samhie::POPUP_BORDER), 1));
            p.setBrush(QColor(samhie::POPUP));
            p.drawRoundedRect(card, POPUP_RADIUS, POPUP_RADIUS);
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

// Mục 22. Vị trí lấy từ ImCursorRectangle (toạ độ ô nhập): đỉnh = đỉnh hộp
// dòng; Qt nới ô đó 5px mỗi bên quanh dấu nháy (rectForPos) → lấy giữa ô. Nhấp nháy theo nhịp hệ thống, hiện lại ngay khi
// gõ / dời con trỏ như Qt.
class ShortCaret : public QWidget
{
public:
    explicit ShortCaret(QLineEdit *edit)
        : QWidget(edit)
        , m_edit(edit)
    {
        setObjectName(QStringLiteral("samhie-caret"));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        const int flash = QApplication::cursorFlashTime();
        m_timer.setInterval(flash > 0 ? flash / 2 : 0);
        QObject::connect(&m_timer, &QTimer::timeout, this, [this] {
            m_on = !m_on;
            update();
        });
        QObject::connect(edit, &QLineEdit::cursorPositionChanged, this, [this] { restart(); });
        QObject::connect(edit, &QLineEdit::textChanged, this, [this] { restart(); });
        edit->installEventFilter(this);
        edit->setProperty("samhieShortCaret", true);
        setGeometry(edit->rect());
        raise();
        if (edit->hasFocus())
            restart();
    }

protected:
    bool eventFilter(QObject *, QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::Resize:
            setGeometry(m_edit->rect());
            break;
        case QEvent::FocusIn:
            restart();
            break;
        case QEvent::FocusOut:
            m_timer.stop();
            update();
            break;
        default:
            break;
        }
        return false;
    }

    void paintEvent(QPaintEvent *) override
    {
        if (!m_on || !m_edit->hasFocus() || m_edit->isReadOnly())
            return;
        const QRect line = m_edit->inputMethodQuery(Qt::ImCursorRectangle).toRect();
        const QFontMetricsF fm(m_edit->font());
        const qreal base = line.top() + fm.ascent();
        const qreal top = qRound(base - CARET_UP * fm.capHeight());
        const qreal bottom = qRound(base + CARET_DOWN * fm.capHeight());
        QPainter p(this);
        p.fillRect(QRectF(line.left() + line.width() / 2, top, 1, bottom - top),
                   m_edit->palette().color(QPalette::Text));
    }

private:
    static constexpr qreal CARET_UP = 1.3;    // hơi trên đỉnh chữ hoa
    static constexpr qreal CARET_DOWN = 0.4;  // tới chân chữ g, y

    void restart()
    {
        m_on = true;
        if (m_edit->hasFocus() && m_timer.interval() > 0)
            m_timer.start();
        update();
    }

    QLineEdit *m_edit;
    QTimer m_timer;
    bool m_on = true;
};

// Mục 12. Gửi bằng postEvent (hàng đợi giữ thứ tự bấm → thả): nút có thể mở
// danh sách bằng exec() — không chạy vòng lặp lồng trong lúc menu con còn
// đang xử lý cú bấm của chính nó.
class MenuButtonForwarder : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease)
            return false;
        auto *menu = static_cast<QMenu *>(obj);
        auto *me = static_cast<QMouseEvent *>(event);
        auto *button = qobject_cast<QAbstractButton *>(menu->childAt(me->position().toPoint()));
        if (!button || !button->isEnabled())
            return false;
        if (event->type() == QEvent::MouseButtonPress) {
            const auto actions = menu->actions();
            for (QAction *action : actions) {
                if (action->menu() && action->menu()->isVisible())
                    action->menu()->hide();
            }
        }
        const QPointF local = button->mapFrom(menu, me->position());
        QCoreApplication::postEvent(button, new QMouseEvent(event->type(), local, me->scenePosition(),
                                                            me->globalPosition(), me->button(), me->buttons(),
                                                            me->modifiers(), me->pointingDevice()));
        return true;
    }
};

class SamhieStyle : public QProxyStyle
{
public:
    SamhieStyle() : QProxyStyle(QStyleFactory::create(QStringLiteral("kvantum"))) {}

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
        if (qobject_cast<QToolButton *>(w) && w->parentWidget()
            && qobject_cast<QMenu *>(w->parentWidget()->parentWidget())) {
            QWidget *row = w->parentWidget();
            row->setMinimumHeight(row->fontMetrics().height() + 2 * MENUITEM_FRAME);
        }
        // DolphinStatusBar::paintEvent: drawRoundedRect(palette().window()).
        if (qobject_cast<QMenu *>(w))
            w->installEventFilter(&m_menuForwarder);  // mục 12
        if (w->inherits("DolphinContextMenu")) {
            hideItemCreateNew(static_cast<QMenu *>(w));
            hideViewportFolderColors(static_cast<QMenu *>(w));
        }
        if (qobject_cast<QMenu *>(w) && !w->parentWidget() && !w->inherits("DolphinContextMenu")
            && QCoreApplication::applicationName() == QLatin1String("dolphin")) {
            auto *menu = static_cast<QMenu *>(w);  // mục 17: menu ☰
            removeHamburgerCreateNew(menu);
            QObject::connect(menu, &QMenu::aboutToShow, menu, [menu] {
                removeHamburgerCreateNew(menu);
                trimPlacesMenu(menu);  // mục 19
            });
        }
        if (qobject_cast<QMenu *>(w) && w->parentWidget() && w->parentWidget()->inherits("DolphinTabBar"))
            trimTabMenu(static_cast<QMenu *>(w));  // mục 19
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
        if (isUrlNavigatorButton(w)) {
            if (w->inherits("KDEPrivate::KUrlNavigatorToggleButton"))
                w->setMinimumWidth(TOGGLE_SIDE);  // mục 20: đủ chỗ cho ô vuông
            else
                w->installEventFilter(&m_urlHover);
        }
        if (qobject_cast<QToolBar *>(w) && w->window()->inherits("DolphinMainWindow"))
            w->setContextMenuPolicy(Qt::PreventContextMenu);  // mục 18
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
        w->removeEventFilter(&m_popup);
        w->removeEventFilter(&m_menuForwarder);
        w->removeEventFilter(&m_urlHover);
        QProxyStyle::unpolish(w);
    }
    using QProxyStyle::unpolish;

    int pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *w) const override
    {
        if (pm == PM_TextCursorWidth && w && w->property("samhieShortCaret").toBool())
            return 0;  // mục 22: ShortCaret vẽ thay
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
        return QProxyStyle::styleHint(hint, opt, w, ret);
    }

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
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
            p->fillRect(box, QColor(samhie::ROW_HOVER));
            if (selected) {
                p->save();
                p->setPen(QPen(QColor(samhie::ROW_SELECTED_BORDER), 1));
                p->setBrush(Qt::NoBrush);
                p->drawRect(box.adjusted(0.5, 0.5, -0.5, -0.5));
                p->restore();
            }
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
        // Mục 15. Dolphin tô nền ô (drawRoundedRect) rồi gọi PE_Frame — vẽ lại
        // viền trái vùng file ở cột 0 của ô (cùng cột với viền PanePainter).
        if (pe == PE_Frame && w && w->inherits("DolphinStatusBar")) {
            QProxyStyle::drawPrimitive(pe, opt, p, w);
            if (w->layoutDirection() == Qt::LeftToRight) {
                p->save();
                p->setPen(QPen(QColor(samhie::FRAME_LINE), 1));
                p->drawLine(QPointF(0.5, 0), QPointF(0.5, w->height()));
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
        if (cc == CC_ToolButton && tbopt && w && tbopt->toolButtonStyle == Qt::ToolButtonIconOnly
            && (w->objectName() == QLatin1String("samhie-new-tab")
                || (inDolphinLocationOrSearch(w) && !qobject_cast<QLineEdit *>(w->parentWidget()))
                || (qobject_cast<QToolBar *>(w->parentWidget()) && w->window()->inherits("DolphinMainWindow")))) {
            QStyleOptionToolButton o(*tbopt);
            const int side = qMax(o.iconSize.width(), o.iconSize.height()) + 2 * TOOL_PADDING;
            o.rect = centeredSquare(o.rect, side);
            QProxyStyle::drawComplexControl(cc, &o, p, w);
            return;
        }
        // Mục 20: ô sửa đường dẫn không viền focus cyan. Kvantum (CC_ComboBox,
        // combo_as_lineedit) hỏi thẳng cb->hasFocus() chứ không chỉ đọc state
        // → bỏ cờ focus VÀ không đưa widget (chỉ dùng để hỏi focus / ô nhập
        // trong suốt — ô của KUrlComboBox đục, cùng nhánh khi không có widget).
        if (cc == CC_ComboBox && w && w->parentWidget() && w->parentWidget()->inherits("KUrlNavigator")) {
            QStyleOptionComboBox o(*qstyleoption_cast<const QStyleOptionComboBox *>(opt));
            o.state &= ~State_HasFocus;
            QProxyStyle::drawComplexControl(cc, &o, p, nullptr);
            return;
        }
        QProxyStyle::drawComplexControl(cc, opt, p, w);
    }
    using QProxyStyle::drawComplexControl;

    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        // Tab đang mở (dải tab kiểu document: Konsole, Dolphin) trong như vùng
        // nội dung bên dưới: card với độ đục cửa sổ, tô ĐÈ (Source) lên nền
        // cửa sổ — Kvantum tô chồng (SourceOver) thì thành ~99% đục (máy ảo
        // 2026-10-04: tab #171717 đục, vùng nội dung thấy hình nền). Cùng dáng
        // phần tử "tab" của Kvantum: khe trên TAB_INSET, bo trên TAB_RADIUS.
        auto *tab = qstyleoption_cast<const QStyleOptionTab *>(opt);
        auto *bar = qobject_cast<const QTabBar *>(w);
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
            p->fillPath(path, paneColor());
            p->restore();
            return;
        }
        auto *button = qstyleoption_cast<const QStyleOptionButton *>(opt);
        if (ce == CE_PushButton && button && isMenuRowButton(w)
            && (button->state & State_HasFocus)) {  // mục 16
            QStyleOptionButton o(*button);
            o.state &= ~State_HasFocus;
            if (o.state & State_KeyboardFocusChange)
                o.state |= State_MouseOver;
            QProxyStyle::drawControl(ce, &o, p, w);
            return;
        }
        auto *header = qstyleoption_cast<const QStyleOptionHeader *>(opt);
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
            if (se == SE_TabWidgetLeftCorner)
                r.setTop(DOLPHIN_TAB_INSET);  // nền khi trỏ ngang đỉnh tab
            r.setRight(qMin(r.right(), tw->width() - 1));
        }
        return r;
    }

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &size, const QWidget *w) const override
    {
        QSize s = QProxyStyle::sizeFromContents(ct, opt, size, w);
        auto *bopt = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
        if (ct == CT_ToolButton && bopt && w && qobject_cast<QToolBar *>(w->parentWidget())
            && bopt->toolButtonStyle == Qt::ToolButtonIconOnly
            && !(bopt->features & QStyleOptionToolButton::MenuButtonPopup)) {
            const int side = qMax(bopt->iconSize.width(), bopt->iconSize.height()) + 2 * TOOL_PADDING;
            s = QSize(side, side);
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
        auto *pbopt = qstyleoption_cast<const QStyleOptionButton *>(opt);
        if (ct == CT_PushButton && pbopt && pbopt->text.isEmpty() && isMenuRowButton(w)) {
            QStyleOptionButton o(*pbopt);
            o.text = QStringLiteral("X");
            const QSize withText = QProxyStyle::sizeFromContents(
                ct, &o, size.expandedTo(QSize(0, o.fontMetrics.height())), w);
            s.setHeight(qMax(s.height(), withText.height()));
        }
        auto *mopt = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        if (ct == CT_MenuItem && mopt && mopt->menuItemType != QStyleOptionMenuItem::Separator)
            s.setHeight(qMin(s.height(), mopt->fontMetrics.height() + 2 * MENUITEM_FRAME));
        return s;
    }

private:
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

    PanePainter m_pane;
    PopupPainter m_popup;
    MenuButtonForwarder m_menuForwarder;
    UrlButtonHover m_urlHover;
    // Mục 14: ô đệm phải của tiêu đề cột, Dolphin vẽ ngay trước cột cuối.
    mutable QRect m_headerEnd;
};

}  // namespace

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
