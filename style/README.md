# Widget style "samhie"

Mã: `samhie-style.cpp` + `parts/`. Build trong image (`files/scripts/samhie-style.sh`).
Dưới đây là danh sách "mục N" mà chú thích trong mã nhắc tới (chép nguyên
từ đầu `samhie-style.cpp` lúc tách file, 2026-10-27).

```text
Widget style "samhie": bọc theme Kvantum (QProxyStyle) để vẽ thêm vài chỗ
mà theme Kvantum KHÔNG vẽ được bằng SVG/kvconfig — đã đọc mã Kvantum 1.1.6,
xem docs/NOTES.md giai đoạn 4. Mọi thứ còn lại chuyển nguyên cho Kvantum.

  1. Khung thanh đường dẫn Dolphin. KUrlNavigator vẽ khung bằng
     PE_FrameLineEdit, Kvantum bỏ qua lệnh này (chỉ vẽ cho LibreOffice).
     → vẽ bằng PE_PanelLineEdit của Kvantum: y hệt ô nhập chữ.
  2. Sidebar Places (KFilePlacesView): trỏ chuột / đang chọn CHỈ đổi màu chữ,
     không tô nền; lùi vào thêm SIDEBAR_INDENT px (lề vùng hiển thị).
     Màu chữ: KFilePlacesViewDelegate::paint (KIO 6.30) chép option ra một
     bản CỤC BỘ, gọi style vẽ nền mục (PE_PanelItemViewItem) bằng bản đó,
     RỒI mới lấy màu chữ/icon từ chính bản đó — nên lúc vẽ nền ta sửa màu
     trong bản sao (icon: KIconLoader đọc WindowText/HighlightedText). KHÔNG bọc delegate: KIO tìm delegate bằng
     qobject_cast<KFilePlacesViewDelegate*>(itemDelegate()) và có chỗ không
     kiểm rỗng → Dolphin crash khi bấm sidebar (máy ảo 2026-10-03, gdb:
     KFilePlacesViewDelegate::pointIsHeaderArea).
  3. Vùng nội dung của khung tab kiểu "document" (vùng file Dolphin, vùng
     chữ Konsole) nền card, trên nền cửa sổ secondary — bản vẽ user. Nền
     cửa sổ do Kvantum tô (window.color); vùng nội dung tô ở đây, ĐÈ hẳn
     (CompositionMode_Source) để giữ đúng độ trong thay vì chồng hai lớp.
     Kvantum chừa khe 2px giữa dải tab và nội dung (SE_TabWidgetTabContents)
     — khe nằm trong vùng tô nên cùng màu nội dung.
  4. Dolphin: vùng file có viền trên + trái, bo góc trên-trái (bản vẽ: 12px
     như góc cửa sổ) — thay vạch tay nắm dock của Kvantum. Thanh trạng thái
     Dolphin tự tô ô nền bằng màu cửa sổ → đổi sang màu vùng file.
  5. Nút ở góc dải tab (+ của Konsole) cao bằng dải tab — thấp hơn thì Qt
     dồn nó xuống đáy (đo container 2026-10-03).
  6. Nút chỉ-có-icon trên thanh công cụ VUÔNG, icon giữa (Kvantum cho ô
     hover 32×22 lệch tâm — máy ảo 2026-10-03).
  7. Thanh tìm Dolphin (Search::Bar) trông như một popup: thẻ bo góc có
     viền, cách mép vùng file (bản vẽ user). Nó vẫn nằm TRONG bố cục, trượt
     xuống và đẩy file xuống như cũ — Dolphin dựng nó vậy, style không đổi
     được chỗ đặt.
  9. Dolphin: nút + (tab mới) ở góc trái dải tab, như Konsole — Dolphin không
     có nút này. Tab mới (nút +, Ctrl+T, menu) mở thư mục Home thay vì thư
     mục đang xem (user 2026-10-04); "Open in New Tab" vẫn mở thư mục đó. Dải tab + nút lùi vào DOLPHIN_TAB_INSET px: viền trái vùng
     file nằm ở cột 0 và góc khung bo 12, tab bo 8 — không lùi thì tab đè
     lên viền, góc tab lòi ra ngoài đường cong khung (máy ảo 2026-10-04).
     Tab Dolphin không icon (bản vẽ, như Konsole).
 11. Tab đang mở trong như vùng nội dung (tô đè, không chồng lớp).
 10. Dòng menu cao bằng dòng sidebar (bỏ khoản "font + 8" của Kvantum);
     dòng widget trong menu (Zoom của Dolphin) cũng cao bằng.
  8. Ô file Dolphin / mục danh sách không có khung focus (khung của Kvantum
     bo 2px vẽ đè thành góc vuông; ô đầu thư mục luôn có viền lơ lửng).
 12. Nút trong dòng widget của menu (nút "Other" màu thư mục, ô màu, Zoom)
     bấm được cả khi menu con còn mở. Lúc đó menu con giữ chuột; nó chuyển
     cú bấm cho MENU CHA (QMenuPrivate::mouseEventTaken) chứ không cho nút,
     menu cha coi như chọn cả dòng (QWidgetAction) → đóng hết. User đi từ
     "Compress" lên "Other" rồi bấm ngay (máy ảo 2026-10-04). → bộ lọc trên
     QMenu: cú bấm/thả tới chính menu mà dưới chuột là một nút thì đóng menu
     con, chuyển cho nút (MenuButtonForwarder). Bình thường cú bấm đi thẳng
     vào nút, menu không thấy — bộ lọc chỉ chạy ở trường hợp chuyển tiếp.
     Thêm: menu con đóng sau SUBMENU_SLOPPY_MS thay vì 1000ms của Kvantum.
 13. Dolphin chế độ Details không kẻ sọc. KItemListWidget::paint tự tô dòng
     xen kẽ bằng fillRect(AlternateBase) — không qua style — mà vùng file
     trong (Kvantum transparent_dolphin_view) → dòng sọc đục #171717 giữa
     các dòng thấy hình nền (container 2026-10-05). AlternateBase trong
     suốt trên QGraphicsView + scene của vùng file: KItemListView lấy
     palette từ view lúc vào scene, sau đó từ scene (PaletteChange).
 14. Tiêu đề cột Dolphin (Details) — KItemListHeaderWidget vẽ từng cột +
     phần đệm hai bên bằng CE_Header, widget = viewport (máy ảo 2026-10-05):
     - nền đục như vùng file: hàng cuộn xuống nằm dưới tiêu đề (z 10)
       không còn lộ chữ qua tiêu đề trong suốt;
     - ô hover cột đầu kéo tới mép trái vùng file, cột cuối tới mép phải
       (Dolphin vẽ phần đệm thành ô riêng, ngay TRƯỚC cột kề nó). Chỉ ô
       nền kéo dài (CE_HeaderSection); chữ + mũi tên sắp xếp vẽ ở chỗ cũ;
     - chữ "Size"/"Permissions" căn phải như số liệu bên dưới (Dolphin
       căn phải hai cột này — KFileItemListWidget::isRoleRightAligned —
       còn tiêu đề thì căn trái). Nhận ra theo chữ: giao diện tiếng Anh;
     - cao thêm HEADER_GAP ở dưới, vẽ trong phần trên: dòng đầu cách ô
       hover đúng bằng khe từ mép trên vùng file tới ô hover (đo 10px).
     Ô hover/chọn của dòng file (Details) cũng rộng bằng, và VUÔNG (user
     2026-10-05; chế độ Icons giữ ô bo của Kvantum): Dolphin đưa
     PE_PanelItemViewItem khung từ lề trái tới hết các cột → style tự tô
     cả bề ngang viewport (= ô hover tiêu đề: viewport tự cách viền vùng
     file 7px — đo container 2026-10-05, cả hai 630…1528), cùng màu phần tử
     itemview của Kvantum (trỏ = nền hover, chọn = nền ROW_SELECTED, không
     viền). Nhận ra dòng Details theo bề ngang (> 60% vùng file; ô chế
     độ Icons hẹp hơn nhiều).
     Cột Size/Modified rộng cố định (user 2026-10-05: đổi thư mục là cột
     xê dịch): Dolphin cho mỗi cột rộng tối thiểu = chữ tiêu đề + grip +
     2 × PM_HeaderMargin (KItemListView::preferredColumnWidths, hỏi KHÔNG
     kèm option/widget), còn lại theo chữ dài nhất trong thư mục ("Yesterday
     at 20:37" < "29/09/2026 at 20:38" → lệch 7px). Trả lề đủ để "Modified"
     tối thiểu bằng ngày giờ dài nhất có thể → cột không đổi theo thư mục.
 15. Dòng trạng thái nhỏ của Dolphin (nổi ở góc dưới-trái vùng file) tô
     nền đè lên viền trái vùng file → vẽ lại đoạn viền đó.
 16. Nút bấm (QPushButton) trong dòng widget của menu — hàng màu thư mục
     của Dolphin (setfoldericonitemaction: 5 ô màu chỉ icon + "Other"):
     - ô chỉ icon cao bằng nút có chữ: Kvantum tính theo icon 16px → ô
       lùn hơn hẳn "Other" (chữ Dongle cao; ảnh user 2026-10-05);
     - không có khung focus: bấm Other thì nút nhận focus, khung focus
       của Kvantum (accent@0.7) bị viền nút che trên/dưới, chỉ còn hai sọc
       dọc xanh (đo ảnh user: #0d93af ở hai cột). Focus BÀN PHÍM (đi bằng
       Tab/mũi tên trong hàng) hiện bằng nền hover, như dòng menu.
 17. Menu chuột phải Dolphin khi bấm VÀO một thư mục không có "Create New"
     (user 2026-10-05: tạo file BÊN TRONG thư mục đó, dễ nhầm với bấm chỗ
     trống). Dolphin không có tuỳ chọn (các mục khác tắt bằng /etc/xdg/
     dolphinrc [ContextMenu]). Menu con có icon list-add: menu chỗ trống
     đặt nó ĐẦU TIÊN (DolphinContextMenu::addViewportContextMenu, 26.08),
     menu thư mục đặt sau Open in…/Open With → cái không đứng đầu thì ẩn.
     Làm lúc polish: QMenu::popup gọi ensurePolished trước khi tính cỡ.
     Menu ☰ cũng không có "Create New" (trùng chuột phải chỗ trống). Menu
     con đó là MỘT với menu chỗ trống (hành động "new_menu" của cửa sổ,
     DolphinMainWindow::m_newFileMenu) → gỡ khỏi menu ☰ (removeAction),
     không ẩn hành động. Menu ☰ hiện ra là bản CHÉP KHamburgerMenu dựng
     (QMenu không cha, khamburgermenu.cpp newMenu) — dựng mới khi Dolphin
     đổi danh sách → gỡ lúc polish + mỗi aboutToShow. Mọi QMenu không cha
     của Dolphin (trừ menu chuột phải) đều gỡ — kể cả File trong More.
     Menu chuột phải CHỖ TRỐNG không có hàng màu thư mục: ở đó nó đổi màu
     THƯ MỤC ĐANG MỞ — dễ nhầm, đã có ở menu của thư mục và hộp thoại tạo
     thư mục (user 2026-10-05). Menu chỗ trống = menu có Create New của cửa
     sổ ("new_menu"); hàng màu = QWidgetAction có nút kèm menu (Other).
     QMenu tạo widget cho QWidgetAction ngay lúc thêm hành động → lúc
     polish đã có. Gỡ khỏi menu (removeAction) chứ không ẩn.
 18. Thanh công cụ Dolphin không có menu chuột phải (Text Position, Icon
     Size, Configure Toolbars, Lock — bấm nhầm là hỏng thanh theo bản vẽ;
     user 2026-10-05). Ô đường dẫn trong thanh vẫn có menu riêng.
 19. Bớt menu chuột phải tab và sidebar Dolphin (user 2026-10-05; Dolphin
     26.08 / KIO 6.30 không có tuỳ chọn). Nhận mục theo TÊN ICON — chữ thì
     đổi theo ngôn ngữ. Gỡ khỏi menu (removeAction), hành động vẫn còn.
     - Tab (DolphinTabBar::contextMenuEvent, QMenu cha = dải tab): chỉ giữ
       Close Other Tabs (tab-close-other). New/Close Tab trùng nút + / ×.
     - Sidebar (KFilePlacesView::contextMenuEvent): QMenu KHÔNG cha, polish
       ngay lúc tạo khi còn trống → bớt lúc aboutToShow (đủ mục, cả mục
       Dolphin thêm). Nhận ra: có mục icon "hint" (Hide / Hide Section) hoặc
       hành động lock_panels (menu chỗ trống). Bỏ Open in New Window
       (window-new, menu thư mục cũng đã tắt), Properties
       (document-properties), Hide Section (mục "hint" CUỐI: mục sidebar
       có Hide rồi Hide Section, tiêu đề nhóm chỉ có Hide Section — sidebar
       chỉ một nhóm, ẩn là trống trơn), Icon Size (menu con duy nhất; cỡ 20
       đặt ở /etc/xdg/dolphinrc), Unlock Panels (lock_panels — mở khoá là
       hiện thanh tiêu đề panel, phá bố cục).
 20. Thanh đường dẫn Dolphin (KUrlNavigator, user 2026-10-05):
     - trỏ vào tên thư mục / mũi tên › : KHÔNG tô ô, chữ + mũi tên đổi sang
       HOVER_ACCENT. KIO vẽ chữ bằng palette Text của chính nút, ép độ đục
       (thư mục cha 50% khi trỏ, thư mục đang mở 100% — foregroundColor,
       KIO 6.30) → đổi Text của nút lúc chuột vào/ra (UrlButtonHover).
     - ô sửa đường dẫn (bấm vào thanh) không có viền focus cyan.
     - nút ✓ (thoát chế độ sửa): ô hover VUÔNG như nút hàng xóm.
     - ô tìm của Dolphin (Search::Bar) cũng không viền focus cyan.
 22. Dấu nháy ô nhập chữ ngắn lại (user 2026-10-05). Qt vẽ dấu nháy cao
     bằng CẢ hộp dòng của font — Dongle khai ascent+descent 1,448 em mà chữ
     hoa chỉ cao 0,405 em → nháy 29px cạnh chữ ~8px. Qt không cho đổi chiều
     cao (QTextLayout::drawCursor) → PM_TextCursorWidth = 0 (Qt vẫn chạy
     nháy nhưng không vẽ gì) và một lớp con trong suốt (ShortCaret) vẽ dấu
     nháy theo chiều cao chữ: CARET_UP × cap trên chân chữ, CARET_DOWN ×
     cap dưới. Chỉ ô nhập dùng Dongle (font giao diện).
 21. Ô hover vuông, icon + TOOL_PADDING mỗi phía, ở giữa nút: nút ở góc dải
     tab (+ của Dolphin lẫn Konsole — user 2026-10-09; cao bằng dải tab, mục
     5 — Konsole trước đó ô hover cao bằng dải) và nút icon trên thanh công cụ
     Dolphin (☰ do KHamburgerMenu dựng cao bằng thanh, 34×42 — container),
     nút × của thanh tìm (Dolphin xếp nó lùn hơn icon: ô hover 26×14 —
     container) — nút đó được nới tối thiểu bằng ô vuông. Nút ⊖ ⟲ ⊕ dòng
     Zoom (menu ▦ của Dolphin): ô hover vuông, KHÔNG có khung nền thường
     trực — Dolphin dựng QToolButton không autoRaise nên Kvantum vẽ khung
     vuông cho cả ba nút, rê chuột thì thêm ô hover bo góc đè lên (ảnh user
     2026-10-06: "như có tận 2 ô hover"); Dolphin cho nút cao bằng dòng
     menu − 4 (~36px).
 24. Bớt chi tiết thừa trong Dolphin (user 2026-10-06): bút chì "sửa
     đường dẫn" (trỏ vào thanh đã hiện con trỏ chữ I), nút "lưu tìm kiếm"
     trong ô tìm (thêm mục vào sidebar), dòng Zoom trong menu (phím tắt
     vẫn chạy), 🔒 + Aᴮ của thanh lọc, dòng KFind và nhóm "Search using:"
     (Simple search / File Indexing — image không có Baloo) trong popup
     Filter của thanh tìm, hai dòng "Focus … Panel" trong Show Panels
     (phím tắt vẫn chạy) cùng dòng Folders (F7 vẫn chạy), Rating / Tags /
     Comment trong ▦ → Sort By (2026-10-07, cần Baloo), panel
     Information (F11) ẩn hẳn. Thanh lọc (F10 → Filter) cũng không viền
     focus cyan, ô hover vuông như thanh tìm (mục 20, 21), cách đáy 8px.
 23. Vạch ngăn thanh công cụ (giữa ⊙→ và ▦ của Dolphin): 1px màu
     FRAME_LINE, cao TOOLBAR_SEPARATOR_HEIGHT, ở giữa khoảng trống giữa ô
     icon hai nút bên cạnh (user 2026-10-06). Kvantum vẽ 2px gần trùng nền
     (#282c2d trên #262a2b), cao 29px — đo ảnh máy ảo.
 25. Radio/checkbox không có khung focus cyan quanh chữ (user 2026-10-06,
     popup Filter của thanh tìm Dolphin). Bấm chuột: không gì cả. Đi bằng
     Tab: nền như ô hover bo MENUITEM_RADIUS quanh vòng + chữ (user
     2026-10-07) — trên popup/menu màu hover, trên nền cửa sổ sub-secondary.
 26. Popup Filter của thanh tìm Dolphin trôi 1px sang phải mỗi lần đổi
     radio (user 2026-10-06; đo container: cả style gốc cũng trôi). Đổi
     radio → Popup::updateState → WidgetMenu::resizeToFitContents gỡ/gắn
     lại nội dung → popup co rồi phình → Qt đặt lại vị trí. Lỗi Qt 6.11
     (qwaylandxdgshell.cpp createPositioner): popup không khai loại thì
     neo vào GÓC PHẢI ô 1×1 tại chính vị trí hiện tại → mỗi lần đặt lại
     +1px (lúc mở cũng đã +1). Khai neo góc trái qua thuộc tính
     _q_waylandPopupAnchor của QWindow (Qt đọc mỗi lần tạo positioner).
 27. Panel Terminal của Dolphin (F4) chỉ nằm dưới vùng file (user
     2026-10-07): góc dưới cửa sổ giao cho vùng dock trái/phải
     (QMainWindow::setCorner) → sidebar kéo xuống tận đáy. Panel trông như
     vùng file: thẻ thứ hai cách 8px, nền card, viền trên + trái, bo góc
     trên-trái PANE_RADIUS (PanePainter vẽ trên dock "terminalDock"); vùng
     file lúc đó khép đáy + bo góc dưới-trái. Chữ Konsole lùi thêm
     TERMINAL_PAD px so với viền (cộng TerminalMargin của profile Samhie).
 28. Đi bằng Tab không còn khung/sọc cyan nào trong Dolphin (máy ảo user
     2026-10-07): nút công cụ (▦, kính lúp, ☰) và nút bấm có focus bàn
     phím → nền hover như mục 16, 25; tab đang mở bỏ khung focus (tab đó
     vốn nổi sẵn, mũi tên đổi tab đang mở). Nhãn dòng trạng thái ("1
     folder") và nhãn tiêu đề popup Filter không nhận Tab (Dolphin cho chọn
     chữ bằng bàn phím → con trỏ chữ trắng). Radio của popup Filter lùi vào
     KEY_FOCUS_PAD px → ô nền Tab có khoảng trống bên trái vòng tròn
     (widget vốn chỉ thừa 2px); MỌI radio/checkbox (user 2026-10-07: hộp
     thoại Configure Dolphin cũng cần). Popup Filter: lề trái bớt đúng
     ngần ấy (nhãn "Search in:" thụt theo) → đứng chỗ cũ, thẳng cột nhãn.
 29. Menu ☰ Dolphin không có tầng Configure (user 2026-10-07): đưa thẳng
     Configure Keyboard Shortcuts… và Configure Dolphin… ra menu ☰ chỗ dòng
     Configure, bỏ Window Color Scheme (màu Samhie đi qua Kvantum, đổi ở đây
     chỉ lẫn lộn), Configure Language (giao diện đã chốt tiếng Anh),
     Configure Toolbars (thanh do style chỉnh, mục 18). Menu con dựng mới
     mỗi lần mở (DolphinMainWindow::updateHamburgerMenu, 26.08.1) → làm như
     mục 17: lúc polish + mỗi aboutToShow. Nhận ra theo tên hành động.
 31. Nút chỉ có icon (không chữ) cao bằng nút có chữ, vuông — Kvantum tính
     theo icon 16px (nút chọn thư mục Configure Dolphin cao 18px cạnh ô
     nhập 40px, máy ảo user 2026-10-07). Như mục 16.
 32. Danh sách trang hộp thoại cài đặt (KPageListView: Interface, View…):
     ô hover/chọn cách hai mép và cách nhau PAGE_LIST_GAP (user
     2026-10-07: đang sát nhau, sát mép).
 33. Thanh trượt (cỡ icon ở Configure Dolphin → View) không khung focus
     cyan, không sáng lên khi trỏ chuột (user 2026-10-07).
 34. Configure Dolphin → View không có tab Compact: Samhie đã bỏ chế độ
     Compact (files/scripts/dolphin-ui.sh). Nhận tab theo icon
     view-list-details (viewsettingspage.cpp 26.08.1) — chữ đổi theo ngôn ngữ.
 35. Danh sách Configure Keyboard Shortcuts bớt các dòng KHÔNG còn dùng
     được ở Samhie (user 2026-10-07: "ẩn và không còn sử dụng"; ẩn khỏi
     menu mà phím tắt vẫn chạy thì giữ), và dòng trùng việc dòng khác / không
     phải lệnh (user, đợt 2 cùng ngày). Bộ lọc tìm của danh sách hiện lại
     dòng ẩn → xoá hẳn dòng (hành động vẫn còn). Danh sách: REMOVED_SHORTCUTS.
 36. Trang Interface của Configure Dolphin dùng icon nét riêng
     (samhie-settings-interface) thay icon APP org.kde.dolphin — đổi tên đó
     trong bộ icon là đổi cả icon Dolphin ở dock. Trang là KPageWidgetItem
     KHÔNG có cha → tìm qua KPageWidgetModel::item() (API công khai KF6
     WidgetsAddons — style link thư viện này, image đã có sẵn), đổi
     Q_PROPERTY icon. Chỉ trong hộp thoại Dolphin.
 37. Danh sách trong hộp thoại cài đặt (QListView trong KPageDialog, vd
     Context Menu): ô hover cách mép PAGE_LIST_GAP, cách nhau
     PAGE_LIST_GAP (user 2026-10-07); danh sách tích chọn không có viền
     chọn ("chọn dòng" vô nghĩa), danh sách chọn (Trash: thùng rác từng ổ)
     có nền chọn như danh sách trang (mục 77). Ô tích/icon/chữ lùi theo nội
     dung cả lúc dò cú bấm (subElementRect) — không thì vùng bấm ô tích
     lệch trái 8px, bấm giữa ô không ăn (user 2026-10-07).
 38. Configure Dolphin → Context Menu bớt dòng không dùng được ở Samhie
     (CONTEXT_MENU_HIDDEN). ServiceModel không xoá được dòng → chèn một lớp
     lọc giữa model gốc và lớp sắp xếp/tìm của trang (trang không dùng
     mapToSource; lưu/đặt lại đọc thẳng model gốc → dòng ẩn giữ nguyên).
 39. Trang Context Menu: dòng không có icon (user 2026-10-22: "nhìn cho
     gọn"; trước đó là icon nét riêng từng dòng) — lớp lọc mục 38 trả
     icon rỗng, ô chữ tự dồn sang trái (delegate không chừa chỗ icon).
 41. Find Action… (Ctrl+Alt+I, KCommandBar) không liệt kê hành động đã bỏ
     (REMOVED_SHORTCUTS), hành động ẩn, Show Toolbar (user 2026-10-07: gõ
     "menubar" vẫn ra Show Menubar). Lớp lọc chèn như mục 38.
 40. Menu ☰ Dolphin không có More, Help chỉ còn Find Action… và About
     Dolphin (user 2026-10-07). More (KHamburgerMenu dựng: Show Menubar +
     bản chép File/Edit/View/Go/Tools/Settings) trùng menu chuột phải, ▦,
     phím tắt — và lộ lại Group By / Show Additional Information mà
     dolphin-ui.sh đã ẩn, Show Toolbar, Bookmarks. Nhận ra More theo hành
     động options_show_menubar đứng đầu (chữ đổi theo ngôn ngữ). Help là
     menu thật của thanh menu (dùng chung) → gỡ hành động trong HELP_HIDDEN
     khỏi nó. Làm như mục 17: lúc polish + mỗi aboutToShow.
 42. Hộp Add/Edit Places Entry (chuột phải sidebar Dolphin): ô Label vẽ như
     ô nhập thường — bo góc, không khung cyan, chữ cách lề như ô Location
     (user 2026-10-07). Hộp thoại có cha là sidebar (KFilePlacesView) nên
     Kvantum tưởng ô Label là ô sửa tên trong danh sách (Kvantum.cpp 1.1.6:
     cha của cha là QAbstractItemView → khung Highlight 1px vuông, chữ sát
     mép, cao theo chữ) → hỏi Kvantum không kèm widget.
 43. Hộp chọn icon (KIconDialog — "Choose an icon" của mục 42) chỉ có icon
     của bộ Samhie; ô chọn nhóm chỉ còn nhóm bộ đó có, bỏ "Other". Nút
     Browse… dùng icon samhie-browse (user 2026-10-07).
 44. Hộp thoại chọn file của KDE (KFileWidget — Browse…, hộp Open/Save của
     mọi app KDE), user 2026-10-07/08: thanh công cụ bỏ Compact View,
     Bookmarks, thanh zoom, Reload, Show Preview; menu ⚙ chỉ còn Show
     Hidden Files + Show Quick Filter; menu chuột phải bỏ Sorting, View
     Mode, Reload, Open Containing Folder, dòng mờ ẩn khi menu mở. Phím
     tắt vẫn chạy (hành động gắn trên hộp thoại, không phải trên menu).
 45. Sidebar chỉ dùng icon NÉT (user 2026-10-08): thư mục thêm vào (Add to
     Places, kéo thả) mang icon thư mục màu → đổi sang icon nét, lưu luôn
     vào user-places.xbel (icon thư mục ở vùng file giữ nguyên); Add Entry
     mở với icon nét; hộp chọn icon của Add/Edit Entry chỉ có icon nét.
 46. Sidebar: vạch ngang 1px màu FRAME_LINE giữa mục mặc định (… Recycle
     Bin) và thư mục người dùng ghim thêm, hai đầu cách viền như icon, trên
     dưới thoáng bằng khe giữa hai mục (user 2026-10-08). Chỉ có khi có mục
     ghim. Delegate riêng cho dòng Recycle Bin (PinnedGapDelegate).
 47. Hàng màu thư mục (menu chuột phải thư mục, user 2026-10-08):
     [Default] [Red] [Yellow] [Teal] [Blue] [Symbol ▾] (Blue thay Grey
     2026-10-20: thư mục mặc định thành xám như Ubuntu); Symbol = No Symbol +
     14 ký hiệu (FOLDER_SYMBOLS). Màu và ký hiệu đổi độc lập: plugin
     Dolphin vẫn ghi .directory, FolderChangeWatcher (nghe KDirNotify qua
     D-Bus) sửa lại Icon=folder-<ký hiệu>-<màu> và lưu hai khoá
     X-Samhie-FolderSymbol / X-Samhie-FolderColor.
     Menu Symbol có icon (vẽ tay — Samhie tắt icon menu) + ô tròn, nút màu
     được tích theo thư mục đang chuột phải (đọc qua giao diện trợ năng
     của vùng file Dolphin, folderUnderCursor).
 48. Hộp Create Folder: hàng 5 màu + ô ký hiệu như mục 47, thay lưới 20
     ô + Choose Icon… của KIO.
 50. Menu Open With / Open Folder With có icon app (user 2026-10-08) —
     Samhie tắt icon menu, nên chữ lùi + style tự vẽ icon (enableMenuIcons,
     dùng chung với menu Symbol).
 49. Ô đổi tên tại chỗ (F2) thấp vừa chữ: khung, vùng chọn, dấu nháy theo
     dải chữ thay vì hộp dòng cao 1,45 em của Dongle (user 2026-10-08).
 51. Thanh đường dẫn không có mũi tên › ở đầu (chỗ không có thư mục cha:
     Recycle Bin, /) thì chữ lùi bằng bề ngang mũi tên — đứng cùng chỗ với
     "Home" (user 2026-10-08: "Recycle Bin" dính mép trái).
 53. Hộp Properties (Alt+Return, KPropertiesDialog — user 2026-10-08):
     - General bỏ dòng Size on disk, Accessed, File system, Mounted on,
       Mounted from; thanh Free space ngang nhãn, chữ dung lượng nhỏ hơn
       nằm dưới thanh. Hộp co theo nội dung mới, dải tab không bị cắt chữ.
     - Permissions: nhãn hai nhóm canh phải, cùng bề ngang → User/Group
       thẳng cột với Owner/Group/Others.
     - Details: cột giá trị thẳng cột General, ô Comment to (dòng thừa của
       tab bỏ bằng /etc/xdg/baloofileinformationrc).
     - Checksums giữ nguyên KDE (user 2026-10-09: thẳng cột General thì
       bên trái nhãn MD5… trống một khoảng lớn).
     - Chữ có "&" trong ô chọn ("Can View & Modify Content") — Kvantum
       coi "&" là phím tắt, nuốt mất.
 54. Menu chuột phải trên tab Konsole không có "Configure or Rename Tab…"
     (user 2026-10-09): tên tab do shell đặt mỗi dấu nhắc
     (/etc/profile.d/samhie-title.sh, OSC 30) nên tên tự đặt bị đè ngay.
     Menu do mã Konsole dựng (TabbedViewContainer, cha là dải tab), mục có
     objectName "edit-rename" — bỏ lúc polish (menu hiện lần đầu, đã đủ
     mục). Bấm đúp tab vẫn mở hộp đó (còn đổi được màu tab).
 55. Menu ☰ Konsole ("Menu" ở cuối menu chuột phải; thanh menu ẩn) gọn như
     ☰ Dolphin (user 2026-10-09): bỏ More (Show Menubar + bản chép
     File/Edit/View/Settings — hiện thanh menu KDE cũ là vỡ bố cục, và lộ lại
     Read-Only / Mouse Tracking / Set Encoding mà konsole-ui.sh đã gỡ khỏi
     menu chuột phải), Bookmarks, Switch Profile, Create New Profile; tầng
     Settings phẳng ra (mục 29: Configure Keyboard Shortcuts… + Configure
     Notifications… + Configure Konsole…, bỏ Toolbars Shown / Configure
     Toolbars / Configure Language); Help như mục 40. Profile "Built-in"
     (giao diện gốc Konsole, bấm nhầm mất theme) bỏ khỏi mọi danh sách
     profile (New Tab ›, nút + giữ chuột) — Konsole luôn tự thêm nó
     (ProfileManager), không có cấu hình ẩn; nhận theo chữ (giao diện chốt
     tiếng Anh). Configure Konsole → Profiles vẫn quản lý được profile.
     Menu ☰ dựng lại mỗi lần mở (MainWindow::updateHamburgerMenu,
     26.08.1) → như mục 17: lúc polish + mỗi aboutToShow. Phím tắt Ctrl+Shift+M
     / Ctrl+Shift+B xoá ở konsole-ui.sh.
 56. Hộp Configure Konsole: icon trang nét Hugeicons (user 2026-10-09).
     Ba trang xin tên mà đổi chung là đổi chỗ khác — utilities-terminal
     (icon app), folder-temp (thư mục /tmp trong Dolphin), image-jpeg (loại
     file) — nên gán tên riêng samhie-konsole-*; ba trang còn lại (memory,
     preferences-desktop-theme, preferences-tabs) vẽ thẳng theo tên gốc
     trong bộ icon. Cùng cách mục 36.
     Hộp Edit Profile / Create New Profile (Profiles → New…, Edit…) cũng
     vậy: utilities-terminal, kcolorchooser (icon app — Appearance dùng
     chung hình trang Profiles), input-keyboard / input-mouse (icon thiết
     bị); Tabs, Scrolling, Advanced vẽ theo tên gốc.
 57. Find Action… (KCommandBar) bấm từ MENU không hiện (máy ảo user
     2026-10-09; Ctrl+Alt+I thì hiện): hành động chạy lúc menu còn là
     popup; ô gõ nhận focus rồi FocusOut lý do ActiveWindowFocusReason —
     popup đóng, cửa sổ thoáng mất kích hoạt trên Wayland (log container:
     FocusIn lý do 7 → FocusOut lý do 3, không còn popup nào) — và
     KCommandBar::eventFilter tự đóng (kcommandbar.cpp, KF 6.30). Lớp lọc
     trên ô gõ nuốt FocusOut vì popup / mất kích hoạt trong 1 s đầu sau khi
     hiện, lấy lại focus ở vòng sự kiện sau; sau đó chuyển cửa sổ khác thì
     nó vẫn tự đóng như cũ.
 58. Nút icon trong hộp thoại (QToolButton chỉ icon): ô hover vuông như
     mục 21. App dựng nút cao bằng icon — nút chọn thư mục "Initial
     directory" của Edit Profile ô hover 24×16, ▶ "Play a sound" của
     Configure Notifications (user 2026-10-09: "ô hover vẫn bị lùn").
     Nút trong ô nhập, trên thanh công cụ, mũi tên cuộn dải tab giữ nguyên.
 59. Ô chọn dự án đầu khung Projects của Kate vẽ phẳng thành tên dự án (bản
     phác Figma của user 2026-10-10): không khung, không icon; nền hover
     như nút công cụ (vẫn bấm được để chuyển dự án); mũi tên ▾ chỉ hiện
     khi đang mở từ 2 dự án — một dự án thì danh sách chỉ có chính nó.
     Plugin Kate (kate/) đánh dấu ô bằng thuộc tính samhieFlatCombo.
 60. Kate: cây Projects và vùng code là hai THẺ bo góc CARD_RADIUS nền
     PANE trên nền cửa sổ (bản phác Figma của user 2026-10-10); hàng tab,
     hàng tên dự án, cột icon, thanh trạng thái nằm trên nền cửa sổ. Plugin
     Kate đánh dấu widget bằng samhieCard rồi polish lại (CardPainter):
     - view space (KateViewSpace): thẻ = breadcrumb + editor, dưới hàng tab;
     - cây Projects: thẻ = cả widget, bỏ khung Kvantum, vùng hiển thị trong.
     Editor Kate tự tô nền vuông (không trong suốt được) → bốn góc là bốn
     widget con nhỏ nằm trên cùng (CardCorner), tô nền cửa sổ phần ngoài
     cung. Không viền: tab đang mở (mục 11) cũng không viền, liền với thẻ.
     Khe giữa các thẻ là tay nắm splitter — không vẽ vạch (Kvantum vẽ).
 61. Cây Projects của Kate: đường nhánh ├ └ │ nối mục con với thư mục cha,
     không mũi tên ▸ (bản phác Figma của user 2026-10-10). Mục cấp gốc
     không có đường (cột nhánh đầu bỏ trống). Plugin Kate đánh dấu cây
     bằng samhieTreeLines. Màu FRAME_LINE như mọi viền của Samhie.
     Dòng cao KATE_TREE_ROW (Kvantum + Dongle cho 42px — thưa hơn bản phác).
 62. Vùng code Kate trong đúng WINDOW_OPACITY như thẻ cây bên cạnh (user
     2026-10-12; nền = theme màu editor "Samhie", gen-theme.py). KTextEditor
     6.30 tô nền dòng có chữ HAI lần (KateViewInternal::paintEvent rồi
     KateRenderer::paintTextLineBackground), cột số dòng tô lại mép phải,
     minimap tô rãnh đè — nền 90% thành ~99% ở đó, 90% ở chỗ khác (đo
     container: lộ vệt quanh từng dòng). Vẽ xong mỗi widget đó, hạ điểm ảnh
     nào đục hơn WINDOW_OPACITY xuống đúng mức ấy trong bộ đệm cửa sổ
     (EditorAlpha) — chữ cũng 90%, như mọi thứ trên thẻ.
 63. Dải tab cuộn (nhiều tab hơn chỗ) không có vạch xanh 2px ở mép: "dấu
     xé tab" của Qt (PE_IndicatorTabTearLeft/Right, QTabBar vẽ khi tab đầu
     bị che một phần) ra #198cff — ngoài bảng màu (máy ảo user 2026-10-12,
     Kate nhiều tab; tái hiện container). Tab bị cắt ở mép đã đủ báo còn tab.
 64. Khe giữa cột số dòng và code của Kate rộng thêm GUTTER_EXTRA (user
     2026-10-12: "nới ra 1 chút"). KateIconBorder::sizeHint = mép phải vùng
     cuối nó vẽ (cột gập code); nó tô nền + dòng đang đứng hết bề rộng
     thật → đặt bề rộng tối thiểu = sizeHint + GUTTER_EXTRA sau mỗi lần vẽ
     (sizeHint đổi theo số chữ số). Kate bố trí widget Fixed theo sizeHint
     nhưng tôn trọng minimumWidth. Bên trái cột: plugin Kate mục 8 chừa ô
     đệm (ngoài KateIconBorder) → style tô ô đệm từng hàng bằng màu mép
     trái của cột số — dải dòng đang đứng liền tới mép thẻ (không thì hở
     một khấc ~10px, đo container). Màu đọc NGAY sau khi cột số vẽ, cất
     theo hàng (thuộc tính trên widget): ngoài vùng đang vẽ, bộ đệm Wayland
     (nhiều bộ luân phiên) có thể còn hình cũ — đọc lúc tô ô đệm thì chép
     nhầm dải dòng cũ (đo container).
 65. Minimap của Kate thoáng như VS Code (user 2026-10-12 chọn kiểu D từ bản
     giả lập): xen một hàng trống giữa các hàng chữ + chữ nhạt còn
     MINIMAP_INK. Kate vẽ cứng (KateScrollBar, KTextEditor 6.30): mỗi dòng
     code 2px liền nhau, mỗi ký tự một khối đặc rồi ép ngang 108px vào bề
     rộng minimap → vệt đặc "dày cộm". Sau khi Kate vẽ: màu nền thật của
     từng hàng lấy ở cột MINIMAP_BASE_X (lề 8px bên trái dành cho vạch dòng
     đã sửa — vạch nằm ở cột 2–4, cột 7 luôn trống), đã gồm lớp tô khung
     xem + lớp làm mờ; hàng lẻ = màu đó, hàng chẵn pha về nó. Viền khung
     xem chạy ngang qua cột nền nên giữ nguyên.
 66. Cột icon trái của Kate (KMultiTabBarTab chỉ có icon) trong
     WINDOW_OPACITY như phần còn lại của cửa sổ (user 2026-10-12). Kvantum
     1.1.6 coi nút này là QPushButton không chữ → tô ĐẶC cả ô bằng màu
     Button (#262626; đang mở: +30 mức xám = #444444) — cả ở trạng thái
     thường, qua bộ lọc sự kiện riêng (eventFiltering.cpp, Paint) → máy ảo
     thấy dải đục sau 4 nút, dưới nút cuối lại trong. Bộ lọc của style
     này chạy trước và tự vẽ cả nút: nút đang mở/đang bấm tô
     SIDEBAR_TAB_OPEN ở độ đục cửa sổ, icon giữa ô thật. Ô 48×48
     (SIDEBAR_TAB) như activity bar VS Code (ACTION_HEIGHT 48, rộng 48);
     cỡ icon đặt ở /etc/xdg/katerc. Nút có chữ (Output, Diagnostics ở
     đáy) giữ nguyên Kvantum.
 67. Tên dự án đầu khung Projects của Kate (mục 59) bắt đầu thẳng icon thư
     mục cấp gốc của cây bên dưới (user 2026-10-13). Plugin Kate (mục 13)
     đo vị trí icon, ghi samhieTextIndent lên ô chọn; style tự vẽ chữ.
 68. Nút icon hàng trên của Kate (⟳ « đầu khung Projects; ‹ › ⌘ ⧉ ☰ đầu
     dải tab): ô hover vuông như mục 21 (user 2026-10-13: ô 28×18 — Kate
     xếp nút cao bằng icon). Plugin Kate đánh dấu samhieSquareHover; nút
     được nới tối thiểu bằng ô vuông (hàng cao 40 — đủ chỗ).
 69. Quick Open của Kate (bản phác user 2026-10-13): khung là MỘT thẻ
     POPUP bo POPUP_RADIUS viền POPUP_BORDER (như thẻ thanh tìm Dolphin);
     danh sách kết quả nằm thẳng trên nền thẻ, ô đang chọn/trỏ tô
     POPUP_ITEM bo MENUITEM_RADIUS (như dòng menu — Kvantum tô `hover` vốn
     cho nền card, trên nền secondary thì chìm mất), không viền chọn; ô chỉ
     đang trỏ chuột tô nhạt một nửa — phân biệt với ô đang chọn (Enter mở).
     Plugin Kate (mục 14) đánh dấu samhiePopupPanel / samhiePopupList.
     Dòng nhiều cột (popup Ctrl+Tab, plugin mục 24): một dải liền, bo hai
     đầu dòng.
 70. Cây Projects của Kate (samhieTreeLines, mục 61) chọn/trỏ như Explorer
     của VS Code (user 2026-10-13): ô dài gần hết bề ngang cây — cả phần
     thụt đầu dòng của mục con, lùi TREE_ROW_INSET mỗi bên, bo
     TREE_ROW_RADIUS (đo VS Code) — thay ô bắt đầu sau phần thụt; không
     viền cyan (khung focus cả dòng của QTreeView — Qt vẽ KHÔNG kèm
     widget nên style không nhận ra — plugin Kate tắt
     allColumnsShowFocus). Dòng đang chọn TREE_ROW_SELECTED, dòng chỉ trỏ chuột ROW_HOVER. Đường
     nhánh vẽ trên nền (trên dòng chọn: TREE_LINE_SELECTED — nền chọn trùng
     màu đường FRAME_LINE).
 71. Kết quả panel Search của Kate không có ô tích khi ô Replace đang
     đóng (plugin Kate mục 15 đánh dấu samhieNoCheck): ô tích chỉ để chọn
     dòng cho "Replace Checked" — VS Code không có. Bỏ HasCheckIndicator
     khi vẽ VÀ khi tính chỗ (chữ dịch về chỗ ô tích; vùng bấm ô tích rỗng →
     bấm không đổi tích). Delegate của Kate có chỗ hỏi chỗ chữ KHÔNG kèm
     widget (dòng tên file) → đọc widget trong option.
 72. Panel Git / Symbols của Kate (user 2026-10-13): ô chọn/trỏ dòng của
     cây hết bề ngang thẻ, vuông (samhieRowFlush — cây Projects giữ lùi
     TREE_ROW_INSET bo góc, mục 70); nút Commit hover như tên dự án (mục
     59): ô vuông hết nút, nút sát cột icon (samhieFlatHover). Plugin Kate
     mục 17 đánh dấu.
 73. Breadcrumb trên vùng code Kate (BreadCrumbView) không có ô hover/ô
     chọn (user 2026-10-13: "chuyển sang hover text"): trỏ vào thì chữ
     HOVER_ACCENT như tên thư mục trên thanh đường dẫn Dolphin. Delegate
     của Kate (kateurlbar.cpp 26.08.1) đổi chữ sang windowText khi trỏ, mà
     chữ thường đã cùng màu đó (đo container: 189,190,193 cả hai) → không
     nhận ra.
 74. Chữ đều nét trong nhãn rich text (<tt> <code> <pre>) cùng cỡ mắt thấy
     với chữ thường (user 2026-10-13: đường dẫn trong hộp xoá file to, tràn
     hai mép). Qt giữ cỡ điểm của nhãn cho đoạn đều nét: Dongle 20pt ≈ Hack
     11pt → Hack 20pt to gấp đôi. Trước khi nhãn vẽ, ghi font-size =
     cỡ nhãn × [fonts] mono-size / size vào thẻ (MonoSpans).
 75. Find Action… (Ctrl+Alt+I, KCommandBar — chung Kate, Dolphin, Konsole)
     là thẻ như Quick Open của Kate (mục 69, user 2026-10-13: "vẫn dùng
     style cũ"): thẻ POPUP bo góc viền POPUP_BORDER, lề trong
     POPUP_CARD_PAD, ô gõ có khung ở trên cách danh sách POPUP_CARD_GAP,
     danh sách trên nền thẻ. Ô đang chọn/trỏ tô POPUP_ITEM cả dòng (hai cột
     — tên + phím tắt — mỗi cột một delegate của KDE), không viền cyan. Bọc
     hai delegate (CommandBarRow): tô dòng rồi để KDE vẽ như dòng thường —
     dòng chọn giữ chữ xám "Kate Part:" và chữ khớp cyan như Quick Open
     (kcommandbar.cpp 6.30 bỏ màu khi chọn vì nền Highlight). Mặt phím tắt
     KDE tô màu cửa sổ = nền thẻ → đổi sang KEYCAP. Bỏ bóng đổ của KDE
     (vệt đen quanh thẻ — máy ảo user 2026-10-15).
 77. Danh sách trang bên trái hộp cài đặt (KPageView: Configure Kate dạng
     danh sách phẳng, Dolphin/Konsole dạng icon — user 2026-10-13 "giao diện nào cũ thì
     làm hết"): trang đang chọn tô PAGE_ITEM_SELECTED, không viền;
     trỏ chuột
     vẫn nền hover của Kvantum — chọn sáng hơn trỏ như cây Projects (mục 70).
 78. Tiêu đề nhóm trong menu (QMenu::addSection — vd menu "Soft Tabs" của
     thanh trạng thái Kate: Tab Width / Indentation Width / Indentation
     Mode) hiện chữ (user 2026-10-20 chốt Kate). Kvantum báo không hỗ trợ
     (SH_Menu_SupportsSections) → Qt vẽ thành vạch trơn, hai nhóm 8/4/2
     giống hệt nhau. Dòng tiêu đề vẽ như một dòng menu bị tắt (Kvantum lo
     lề + màu chữ mờ), không trỏ, không ô tích; cao như dòng thường.
 79. Thẻ như Quick Open (mục 69/75) cho: HUDDialog của Kate (popup chọn
     nhánh git khi bấm tên nhánh ở thanh trạng thái — dựng y như
     KCommandBar, quickdialog.cpp 26.08.1); danh sách chế độ (bấm "Python"
     ở thanh trạng thái, KateModeMenuList — mọi danh sách nằm trong menu)
     có ô chọn POPUP_ITEM, không viền cyan.
 76. Popup dạng QMenu mang samhiePopupPanel (lịch sử clipboard của Kate,
     plugin Kate mục 25) vẽ thẻ như mục 69 thay nền + khung menu Kvantum
     (menu trong suốt → chữ code phía sau lộ qua chỗ trống). Vùng xem trước
     (KTextEditor) trong popup không hạ độ đục (mục 62 chỉ cho cửa sổ).
 52. Menu chuột phải Dolphin không có "Git Clone…" (plugin git của
     dolphin-plugins, ngoài repo git) — giống gõ git clone trong Konsole.
     Dấu trạng thái git trên file và menu Git trong repo vẫn giữ (user
     2026-10-08).
 30.Danh sách phím tắt (Configure Keyboard Shortcuts, máy ảo user
     2026-10-07). KExtendableItemDelegate (KF 6.30) vẽ mỗi ô HAI lần: ô nội
     dung đã cắt bớt bên trái chỗ cho "›", rồi riêng dải "›" (style của
     QApplication, KHÔNG kèm widget); cả hai tự đặt vị trí ô đầu/giữa/cuối
     → Kvantum coi ô dính nhau, vuông một phía → ô hover hai lớp có khấc.
     → dải "›" không vẽ nền; ô nội dung vẽ nền một lần trên CẢ ô (gồm "›"),
     đứng riêng, bo đủ góc; ô đang chọn = nền hover, không viền (viền chọn
     nháy khi bấm ô Action). Ô hiện tại không khung focus (Kvantum vẽ trong
     CE_ItemViewItem, không qua PE_FrameFocusRect của style này — áp cho
     mọi danh sách). Dải sửa phím tắt mở dưới dòng không còn nét
     Highlight dày 6px (TabConnectedWidget::paintEvent tự vẽ) — chặn sự
     kiện vẽ của nó; các con vẫn tự vẽ.
 80. Bản sáng/tối (2026-10-20). Theme Kvantum có hai bản cùng thư mục:
     Samhie (sáng) và SamhieDark (tối) — style "kvantum-dark" của Kvantum
     nạp "<theme>Dark" (Kvantum.cpp setTheme). Chế độ đọc ở
     ~/.config/kdeglobals [General] ColorScheme: color-mode (ô Dark style
     của quick settings) ghi SamhieLight khi sáng, xoá khi tối. Màu riêng
     của style (colors.h) đổi theo bằng samhie::useLight().
     Đổi lúc app đang mở: color-mode phát KGlobalSettings.notifyChange
     (PaletteChanged) → nạp lại style. KDE (plasma-integration) có lệnh
     StyleChanged nhưng bỏ qua khi tên style không đổi — vẫn là "samhie".
     Kate: luôn bản tối (xem lightMode()).
 81. Konsole bản sáng (user 2026-10-20: "như Windows Terminal"): dải tab,
     menu, hộp thoại sáng; vùng chữ GIỮ TỐI (Samhie.colorscheme một bản) —
     tab đang mở + vùng dưới dải tab tô nền terminal (paneColor), chữ và
     × trên tab đó màu bản tối. Tab khác chữ thường của chế độ.
 82. Kate bản sáng (user 2026-10-20): khung, cột icon, panel sáng; THẺ CODE
     tối trọn như Konsole (tab đang mở + breadcrumb + code, theme "Samhie"
     một bản) và thẻ kết quả Search tối (Kate tô kết quả bằng màu theme
     code — plugin đánh dấu samhieDarkCard; khung Output cũng vậy). Nhãn tab đang mở tô một màu
     chữ bản tối (mục 81); icon breadcrumb lấy bản Selected (nét sáng).
 83. Phần tử Kvantum trên thẻ tối ở bản sáng (máy ảo user 2026-10-20: dòng
     kết quả Search trỏ/chọn là dải SÁNG, chữ theme code trên đó mờ đi; nút
     ⤢ góc thẻ sáng). Kvantum chỉ nạp một bộ SVG theo chế độ → vẽ bản sáng
     vào ảnh tạm, đổi xám card/secondary/sub-secondary thành lớp trắng mờ
     nâng khỏi thẻ đúng nấc bản tối (giữ đúng dáng ô/nút của Kvantum).
     Bảng kiểu chữ Theme Editor (KateStyleTreeWidget — nền/chữ màu theme
     code) cũng là vùng tối: tiêu đề cột, ô "None set" (user 2026-10-21).
     Thanh cuộn trên thẻ tối: màu thanh cuộn bản tối (user 2026-10-21).
 84. Tiêu đề trang hộp cài đặt ("Appearance", KPageView) đổi màu theo chế
     độ (máy ảo user 2026-10-21: sáng → tối thì chữ vẫn đen trên nền tối,
     tối → sáng thì chữ sáng mờ). KPageView chép cứng bảng màu app vào
     KTitleWidget lúc dựng (kpageview.cpp:166, KF 6.31) → chép lại mỗi lần
     bảng màu app đổi / style nạp lại. Hộp About (Dolphin, Kate) mở lúc đổi
     chế độ (user 2026-10-21): dòng mô tả dưới tên người/thành phần, link,
     tiêu đề "Tên / Version" (to ra) — xem FollowAppPalette.
 85. Tab thường có icon + chữ (Configure Dolphin › View): icon đứng sát chữ,
     cả cụm giữa tab (Kvantum đặt icon ở mép trái tab, chữ canh giữa).
 86. Bảng màu Theme Editor của Kate (KateColorTreeWidget): ô hover nằm
     DƯỚI ô màu + nút reset (máy ảo user 2026-10-21: ô hover đục che mất ô
     màu, cả hai chế độ). Delegate của KTextEditor vẽ ô màu rồi mới gọi
     QStyledItemDelegate::paint (nền ô) → bọc delegate, tô nền trước
     (HoverUnderRow), CE_ItemViewItem của bảng đó bỏ nền.

Kvantum chủ động hỗ trợ bị bọc trong QProxyStyle (Kvantum.cpp
setSurfaceFormat: "qt5ct uses QProxyStyle").
```
