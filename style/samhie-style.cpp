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
//
// Kvantum chủ động hỗ trợ bị bọc trong QProxyStyle (Kvantum.cpp
// setSurfaceFormat: "qt5ct uses QProxyStyle").

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAction>
#include <QDir>
#include <QAbstractScrollArea>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QProxyStyle>
#include <QScrollArea>
#include <QStyleFactory>
#include <QStyleOption>
#include <QStylePlugin>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>

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

// setViewportMargins là protected — gọi qua lớp con chỉ để mở quyền truy cập.
class ScrollAreaAccess : public QAbstractScrollArea
{
public:
    using QAbstractScrollArea::setViewportMargins;
};

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
        QProxyStyle::unpolish(w);
    }
    using QProxyStyle::unpolish;

    int styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *ret) const override
    {
        if (hint == SH_Menu_SubMenuSloppyCloseTimeout)
            return SUBMENU_SLOPPY_MS;
        return QProxyStyle::styleHint(hint, opt, w, ret);
    }

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const override
    {
        if (pe == PE_FrameLineEdit && w && w->inherits("KUrlNavigator")) {
            baseStyle()->drawPrimitive(PE_PanelLineEdit, opt, p, w);
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
        // Vạch giữa sidebar và vùng file: viền của vùng file (PanePainter) thay.
        if (pe == PE_IndicatorDockWidgetResizeHandle)
            return;
        QProxyStyle::drawPrimitive(pe, opt, p, w);
    }

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
        auto *mopt = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        if (ct == CT_MenuItem && mopt && mopt->menuItemType != QStyleOptionMenuItem::Separator)
            s.setHeight(qMin(s.height(), mopt->fontMetrics.height() + 2 * MENUITEM_FRAME));
        return s;
    }

private:
    PanePainter m_pane;
    PopupPainter m_popup;
    MenuButtonForwarder m_menuForwarder;
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
