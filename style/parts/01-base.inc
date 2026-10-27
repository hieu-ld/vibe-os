// Hằng số, nhận ra widget (Dolphin, menu, dải tab…), vẽ chung (tối hoá mục 83, viền vùng file).
// Một phần của samhie-style.cpp: nằm trong namespace vô danh của file đó,
// được #include theo thứ tự số (phần sau dùng phần trước) — không biên dịch riêng.
// kate: hl C++;

constexpr int SIDEBAR_INDENT = 8;  // bản vẽ: icon sidebar cách mép ~16px (KIO tự chừa 4)
constexpr int PANE_RADIUS = 12;    // góc viền vùng file Dolphin (bản vẽ ~12) = góc cửa sổ
constexpr int CARD_RADIUS = samhie::TAB_RADIUS;  // thẻ Kate (mục 60) = góc tab, tab liền thẻ
// Mục 61: nhịp dòng cây Projects của Kate (user 2026-10-12, sau khi đo VS Code
// trên máy thật: 22px, chữ 13px, icon 16px — Dongle 20 trông cùng cỡ chữ).
// Xem 24px trên máy ảo rồi chọn 28. Thụt mỗi cấp giữ 12 của Kate
// (kateprojectviewtree.cpp setIndentation) — VS Code 8, Zed 20.
constexpr int KATE_TREE_ROW = 28;
// Mục 70: ô chọn/trỏ của cây cách mép thẻ + bo góc — đo Explorer VS Code
// (ảnh user 2026-10-13): lùi 5px mỗi bên, góc ~4px, cao trọn dòng
constexpr int TREE_ROW_INSET = 5;
constexpr int TREE_ROW_RADIUS = 4;
// Mục 64: khe thêm giữa cột số dòng và code của Kate
constexpr int GUTTER_EXTRA = 8;
// Mục 66: ô nút cột icon Kate = ô activity bar VS Code (48×48, icon ở giữa)
constexpr int SIDEBAR_TAB = 48;
// Mục 65: minimap Kate — cột lấy màu nền, cột chữ đầu tiên (s_pixelMargin của
// Kate), độ đậm chữ còn lại, bề rộng tối thiểu để coi là minimap (thanh cuộn
// thường ~12px)
constexpr int MINIMAP_BASE_X = 7;
constexpr int MINIMAP_TEXT_X = 8;
constexpr qreal MINIMAP_INK = 0.7;
constexpr int MINIMAP_MIN_WIDTH = 40;
constexpr int TOOL_PADDING = 6;    // nút icon trên thanh công cụ: icon + 6px mỗi phía
constexpr int TOOLBAR_SEPARATOR_HEIGHT = 18;  // icon thanh công cụ cao ~20px (mục 23)
constexpr int KEY_FOCUS_PAD = 6;   // ô nền focus bàn phím rộng hơn vòng radio + chữ (mục 25, 28)
constexpr int PAGE_LIST_GAP = 4;   // danh sách trang hộp thoại cài đặt: khe quanh mỗi ô (mục 32)
constexpr int TAB_LABEL_GAP = 6;  // mục 85: icon ↔ chữ trên tab thường (đo container)
constexpr int TAB_LABEL_PAD = 12; // mục 85: đệm hai bên nhãn — đủ để Kvantum không cắt "…" chữ
constexpr int TAB_LABEL_SLACK = 40; // mục 85: tab rộng hơn nội dung chừng này mới tính là bị giãn
constexpr int TERMINAL_PAD = 6;    // panel Terminal Dolphin: lề trong thêm (mục 27)
constexpr int TERMINAL_GAP = 7;    // khe trên panel Terminal: + vạch ngăn dock 1px = 8 (mục 27)
constexpr int POPUP_MARGIN = 8;    // thẻ tìm cách mép vùng file
constexpr int POPUP_PADDING = 10;  // nội dung cách mép thẻ
constexpr int POPUP_RADIUS = 12;
constexpr int POPUP_CARD_PAD = 12;  // thẻ Find Action: lề trong (= QUICK_OPEN_PAD của plugin Kate)
constexpr int POPUP_CARD_GAP = 8;   // khe ô gõ – danh sách (= QUICK_OPEN_GAP)
// = PANE_RADIUS − góc tab (8, tools/kvantum.py) → góc tab/nút + đồng tâm với
// góc khung; cũng bằng khe trên của tab (TAB_INSET) nên lùi đều hai phía.
constexpr int DOLPHIN_TAB_INSET = 4;
// Khe giữa nút + (ô hover vuông, mục 21) và tab đầu (user 2026-10-05: dính sát).
constexpr int NEW_TAB_GAP = 4;
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

// Nút ở góc dải tab: + của Dolphin (Samhie thêm) và của Konsole (mục 21).
bool isTabCornerButton(const QWidget *w)
{
    auto *tw = w ? qobject_cast<const QTabWidget *>(w->parentWidget()) : nullptr;
    return tw && (tw->cornerWidget(Qt::TopLeftCorner) == w || tw->cornerWidget(Qt::TopRightCorner) == w);
}

// Trong thanh đường dẫn, thanh tìm hoặc thanh lọc (F10 → Filter) của Dolphin
// (mục 20).
bool inDolphinLocationOrSearch(const QWidget *w)
{
    for (; w; w = w->parentWidget()) {
        if (w->inherits("KUrlNavigator") || w->inherits("Search::Bar") || w->inherits("FilterBar"))
            return true;
    }
    return false;
}

// Mục 42: ô nhập nằm thẳng trong một hộp thoại mà cha của hộp thoại là danh
// sách — Add/Edit Places Entry (cha là sidebar KFilePlacesView). Kvantum coi
// "cha của cha là QAbstractItemView" là ô sửa tên ngay trong danh sách. Ô sửa
// tên thật nằm trong viewport, không phải trong một cửa sổ.
bool isFakeItemEditor(const QWidget *w)
{
    const QWidget *p = w ? w->parentWidget() : nullptr;
    return qobject_cast<const QLineEdit *>(w) && p && p->isWindow()
        && qobject_cast<const QAbstractItemView *>(p->parentWidget());
}

// Nằm (ở cấp nào cũng được) trong một widget thuộc lớp `cls`.
bool insideClass(const QWidget *w, const char *cls)
{
    for (w = w ? w->parentWidget() : nullptr; w; w = w->parentWidget()) {
        if (w->inherits(cls))
            return true;
    }
    return false;
}

// Trong thanh lọc Dolphin (F10 → Filter, Ctrl+I).
bool inFilterBar(const QWidget *w)
{
    for (; w; w = w->parentWidget()) {
        if (w->inherits("FilterBar"))
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

// Mục 21: nút icon trong dòng widget của menu (dòng Zoom của Dolphin).
bool isMenuRowToolButton(const QWidget *w)
{
    return qobject_cast<const QToolButton *>(w) && w->parentWidget()
        && qobject_cast<const QMenu *>(w->parentWidget()->parentWidget());
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

// Mục 81, 82: nền luôn tối (nền terminal) ở cả hai chế độ — w nằm trong
// (hoặc là) khung tab Konsole, view space Kate (thẻ code + dải tab của nó),
// hay thẻ plugin Kate đánh dấu samhieDarkCard (kết quả Search: Kate tô bằng
// màu theme code; bảng kiểu chữ Theme Editor cũng vậy — mục 83). Breadcrumb
// Kate nằm sâu 4 cấp trong view space. Dừng ở mép
// cửa sổ: popup gợi ý gõ là con của view code nhưng là cửa sổ sáng riêng.
bool darkPane(const QWidget *w)
{
    for (int i = 0; w && i < 5; ++i, w = w->parentWidget()) {
        if (w->inherits("Konsole::TabbedViewContainer") || w->inherits("KateViewSpace")
            || w->inherits("KateStyleTreeWidget") || w->property("samhieDarkCard").toBool())
            return true;
        if (w->isWindow())
            return false;
    }
    return false;
}

// Mục 83: paint vẽ (toạ độ của p) vào ảnh tạm cỡ r rồi đặt lại vào r, xám bản
// sáng đổi thành lớp TRẮNG MỜ: bản tối nâng secondary/sub-secondary lên khỏi
// card một nấc (#171717 → #262626 = trắng 6,5%) → đổi đúng nấc đó. Thẻ tối ở
// bản sáng trong 90% trên nền sáng (#2b2b2a, đo container) — tô xám tối đặc
// thì ô trỏ #262626 TỐI hơn thẻ, nút ⤢ như lõm xuống. Nội suy ở mép giữa hai
// vai trò (viền khử răng cưa, mặt giáp viền); màu khác giữ nguyên — trừ sát
// mép dưới: góc bo khử răng cưa làm tròn #d0 thành #cf (đo container).
// base hợp lệ: lót màu đó trong dáng phần tử trước (nút che nội dung sau nó
// như mặt nút đặc bản tối; lớp trắng mờ thì lộ chữ).
template<typename Paint>
void paintDarkened(QPainter *p, const QRect &r, Paint &&paint, const QColor &base = QColor())
{
    // độ xám bản sáng → độ đục lớp trắng (0…1), -1 = giữ nguyên
    static const std::array<qreal, 256> lift = [] {
        constexpr int n = std::size(samhie::DARKEN_FROM);
        const auto gray = [](const char *c) { return QColor(c).red(); };
        const auto alpha = [&](int i) {
            const int card = gray(samhie::DARKEN_TO[0]);
            return qreal(gray(samhie::DARKEN_TO[i]) - card) / (255 - card);
        };
        std::array<qreal, 256> t;
        const int lo = gray(samhie::DARKEN_FROM[n - 1]);
        for (int v = 0; v < 256; ++v) {
            t[v] = v < lo && v >= lo - 16 ? alpha(n - 1) : -1;
            for (int i = 0; i + 1 < n; ++i) {
                const int a = gray(samhie::DARKEN_FROM[i]), b = gray(samhie::DARKEN_FROM[i + 1]);
                if (v <= a && v >= b) {
                    t[v] = alpha(i) + (alpha(i + 1) - alpha(i)) * (a - v) / qMax(1, a - b);
                    break;
                }
            }
        }
        return t;
    }();
    const qreal dpr = p->device() ? p->device()->devicePixelRatioF() : 1.0;
    QImage img(r.size() * dpr, QImage::Format_ARGB32);  // không premultiplied: đọc thẳng màu
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::transparent);
    {
        QPainter ip(&img);
        ip.translate(-r.topLeft());
        paint(&ip);
    }
    QImage shape;  // dáng phần tử, đục hết (mép khử răng cưa: lót cả pixel)
    if (base.isValid()) {
        shape = QImage(img.size(), QImage::Format_ARGB32);
        shape.setDevicePixelRatio(dpr);
        shape.fill(Qt::transparent);
    }
    const QRgb opaqueBase = base.isValid() ? QColor(base.red(), base.green(), base.blue()).rgba() : 0;
    for (int y = 0; y < img.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const QRgb c = line[x];
            if (!qAlpha(c))
                continue;
            if (!shape.isNull())
                reinterpret_cast<QRgb *>(shape.scanLine(y))[x] = opaqueBase;
            const qreal a = lift[qRed(c)];
            if (a >= 0 && qRed(c) == qGreen(c) && qGreen(c) == qBlue(c))
                line[x] = qRgba(255, 255, 255, qRound(qAlpha(c) * a));
        }
    }
    if (!shape.isNull()) {
        // xoá nội dung dưới dáng rồi lót base (độ đục của base: thẻ trong 90%)
        p->save();
        p->setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p->drawImage(r.topLeft(), shape);
        p->setCompositionMode(QPainter::CompositionMode_SourceOver);
        p->setOpacity(base.alphaF());
        p->drawImage(r.topLeft(), shape);
        p->restore();
    }
    p->drawImage(r.topLeft(), img);
}

// Mục 83: thanh cuộn trên thẻ tối ở bản sáng — vẽ bản sáng vào ảnh tạm rồi
// đổi ĐÚNG màu thanh cuộn bản sáng sang màu bản tối, giữ độ đục từng điểm
// (mép khử răng cưa, thanh vô hiệu hoá 50%, lúc tự ẩn mờ dần). Thanh cuộn
// đục nên đổi thẳng màu, không qua lớp trắng mờ như paintDarkened.
template<typename Paint>
void paintScrollbarDarkened(QPainter *p, const QRect &r, Paint &&paint)
{
    static const auto table = [] {
        std::array<std::pair<QRgb, QRgb>, std::size(samhie::SCROLLBAR_FROM)> t;
        for (size_t i = 0; i < t.size(); ++i)
            t[i] = {QColor(samhie::SCROLLBAR_FROM[i]).rgb() & RGB_MASK, QColor(samhie::SCROLLBAR_TO[i]).rgb() & RGB_MASK};
        return t;
    }();
    const qreal dpr = p->device() ? p->device()->devicePixelRatioF() : 1.0;
    QImage img(r.size() * dpr, QImage::Format_ARGB32);  // không premultiplied: đọc thẳng màu
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::transparent);
    {
        QPainter ip(&img);
        ip.translate(-r.topLeft());
        paint(&ip);
    }
    for (int y = 0; y < img.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            if (!qAlpha(line[x]))
                continue;
            for (const auto &[from, to] : table) {
                if ((line[x] & RGB_MASK) == from) {
                    line[x] = (line[x] & ~RGB_MASK) | to;
                    break;
                }
            }
        }
    }
    p->drawImage(r.topLeft(), img);
}

// Mục 81: Konsole tô vùng dưới dải tab + tab đang mở bằng nền TERMINAL ở cả
// hai chế độ — vùng chữ (Samhie.colorscheme) luôn tối. Mục 82: thẻ Kate cũng vậy.
QColor paneColor(const QWidget *w = nullptr)
{
    QColor c(darkPane(w) ? samhie::TERMINAL : samhie::PANE);
    c.setAlphaF(samhie::WINDOW_OPACITY);
    return c;
}

// Tô vùng nội dung của QTabWidget kiểu document (+ viền ở Dolphin) TRƯỚC khi
// Mục 28: radio/checkbox (widget) — vẽ lùi vào KEY_FOCUS_PAD.
bool isToggle(const QWidget *w)
{
    return w && (w->inherits("QRadioButton") || w->inherits("QCheckBox"));
}

// Mục 28: radio/checkbox của popup Filter thanh tìm Dolphin.
bool isPopupToggle(const QWidget *w)
{
    return isToggle(w) && w->window()->inherits("Search::Popup");
}

// Mục 28: nhãn không nhận Tab — bỏ chọn chữ / mở liên kết bằng bàn phím; bôi
// chữ bằng chuột vẫn giữ (QLabel tự hạ focus xuống ClickFocus).
void dropKeyboardSelection(QLabel *label)
{
    label->setTextInteractionFlags(label->textInteractionFlags()
                                   & ~(Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByKeyboard));
}

// (Cạnh đáy + góc bo dưới-trái nếu bottomRad > 0) + cạnh trái + góc bo
// trên-trái (topRad, 0 = vuông) + cạnh trên; closed = khép thành vùng tô.
QPainterPath leftOutline(const QRectF &r, qreal topRad, qreal bottomRad, bool closed)
{
    // Không khép đáy thì kéo quá mép 1px: nét/vùng tô chạm hẳn mép widget.
    const qreal bottom = bottomRad > 0 ? r.bottom() : r.bottom() + 1;
    QPainterPath path;
    if (bottomRad > 0) {
        path.moveTo(r.right() + 1, bottom);
        path.lineTo(r.left() + bottomRad, bottom);
        path.arcTo(QRectF(r.left(), bottom - 2 * bottomRad, 2 * bottomRad, 2 * bottomRad), 270, -90);
    } else {
        path.moveTo(r.left(), bottom);
    }
    path.lineTo(r.left(), r.top() + topRad);
    if (topRad > 0)
        path.arcTo(QRectF(r.left(), r.top(), 2 * topRad, 2 * topRad), 180, -90);
    path.lineTo(r.right() + 1, r.top());
    if (closed) {
        path.lineTo(r.right() + 1, bottom);
        path.closeSubpath();
    }
    return path;
}

// Mục 27: dưới vùng file Dolphin còn panel (Terminal) thì vùng file khép đáy
// + bo góc dưới-trái — hai thẻ chồng nhau; không thì đáy là mép cửa sổ.
int paneBottomRadius(const QTabWidget *tw)
{
    const QWidget *win = tw->window();
    return tw->mapTo(win, QPoint(0, tw->height())).y() < win->height() ? PANE_RADIUS : 0;
}

// Nét viền vùng file Dolphin (trên + trái, + đáy nếu khép), toạ độ của tw.
QPainterPath paneBorder(const QTabWidget *tw)
{
    const int bottomRad = paneBottomRadius(tw);
    return leftOutline(QRectF(tw->rect()).adjusted(0.5, 0.5, 0, bottomRad ? -0.5 : 0), PANE_RADIUS, bottomRad, false);
}
