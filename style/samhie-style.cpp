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
//      container) — nút đó được nới tối thiểu bằng ô vuông. Nút ⊖ ⟲ ⊕ dòng
//      Zoom (menu ▦ của Dolphin): ô hover vuông, KHÔNG có khung nền thường
//      trực — Dolphin dựng QToolButton không autoRaise nên Kvantum vẽ khung
//      vuông cho cả ba nút, rê chuột thì thêm ô hover bo góc đè lên (ảnh user
//      2026-10-06: "như có tận 2 ô hover"); Dolphin cho nút cao bằng dòng
//      menu − 4 (~36px).
//  24. Bớt chi tiết thừa trong Dolphin (user 2026-10-06): bút chì "sửa
//      đường dẫn" (trỏ vào thanh đã hiện con trỏ chữ I), nút "lưu tìm kiếm"
//      trong ô tìm (thêm mục vào sidebar), dòng Zoom trong menu (phím tắt
//      vẫn chạy), 🔒 + Aᴮ của thanh lọc, dòng KFind và nhóm "Search using:"
//      (Simple search / File Indexing — image không có Baloo) trong popup
//      Filter của thanh tìm, hai dòng "Focus … Panel" trong Show Panels
//      (phím tắt vẫn chạy) cùng dòng Folders (F7 vẫn chạy), Rating / Tags /
//      Comment trong ▦ → Sort By (2026-10-07, cần Baloo), panel
//      Information (F11) ẩn hẳn. Thanh lọc (F10 → Filter) cũng không viền
//      focus cyan, ô hover vuông như thanh tìm (mục 20, 21), cách đáy 8px.
//  23. Vạch ngăn thanh công cụ (giữa ⊙→ và ▦ của Dolphin): 1px màu
//      FRAME_LINE, cao TOOLBAR_SEPARATOR_HEIGHT, ở giữa khoảng trống giữa ô
//      icon hai nút bên cạnh (user 2026-10-06). Kvantum vẽ 2px gần trùng nền
//      (#282c2d trên #262a2b), cao 29px — đo ảnh máy ảo.
//  25. Radio/checkbox không có khung focus cyan quanh chữ (user 2026-10-06,
//      popup Filter của thanh tìm Dolphin). Bấm chuột: không gì cả. Đi bằng
//      Tab: nền như ô hover bo MENUITEM_RADIUS quanh vòng + chữ (user
//      2026-10-07) — trên popup/menu màu hover, trên nền cửa sổ sub-secondary.
//  26. Popup Filter của thanh tìm Dolphin trôi 1px sang phải mỗi lần đổi
//      radio (user 2026-10-06; đo container: cả style gốc cũng trôi). Đổi
//      radio → Popup::updateState → WidgetMenu::resizeToFitContents gỡ/gắn
//      lại nội dung → popup co rồi phình → Qt đặt lại vị trí. Lỗi Qt 6.11
//      (qwaylandxdgshell.cpp createPositioner): popup không khai loại thì
//      neo vào GÓC PHẢI ô 1×1 tại chính vị trí hiện tại → mỗi lần đặt lại
//      +1px (lúc mở cũng đã +1). Khai neo góc trái qua thuộc tính
//      _q_waylandPopupAnchor của QWindow (Qt đọc mỗi lần tạo positioner).
//  27. Panel Terminal của Dolphin (F4) chỉ nằm dưới vùng file (user
//      2026-10-07): góc dưới cửa sổ giao cho vùng dock trái/phải
//      (QMainWindow::setCorner) → sidebar kéo xuống tận đáy. Panel trông như
//      vùng file: thẻ thứ hai cách 8px, nền card, viền trên + trái, bo góc
//      trên-trái PANE_RADIUS (PanePainter vẽ trên dock "terminalDock"); vùng
//      file lúc đó khép đáy + bo góc dưới-trái. Chữ Konsole lùi thêm
//      TERMINAL_PAD px so với viền (cộng TerminalMargin của profile Samhie).
//  28. Đi bằng Tab không còn khung/sọc cyan nào trong Dolphin (máy ảo user
//      2026-10-07): nút công cụ (▦, kính lúp, ☰) và nút bấm có focus bàn
//      phím → nền hover như mục 16, 25; tab đang mở bỏ khung focus (tab đó
//      vốn nổi sẵn, mũi tên đổi tab đang mở). Nhãn dòng trạng thái ("1
//      folder") và nhãn tiêu đề popup Filter không nhận Tab (Dolphin cho chọn
//      chữ bằng bàn phím → con trỏ chữ trắng). Radio của popup Filter lùi vào
//      KEY_FOCUS_PAD px → ô nền Tab có khoảng trống bên trái vòng tròn
//      (widget vốn chỉ thừa 2px); MỌI radio/checkbox (user 2026-10-07: hộp
//      thoại Configure Dolphin cũng cần). Popup Filter: lề trái bớt đúng
//      ngần ấy (nhãn "Search in:" thụt theo) → đứng chỗ cũ, thẳng cột nhãn.
//  29. Menu ☰ Dolphin không có tầng Configure (user 2026-10-07): đưa thẳng
//      Configure Keyboard Shortcuts… và Configure Dolphin… ra menu ☰ chỗ dòng
//      Configure, bỏ Window Color Scheme (màu Samhie đi qua Kvantum, đổi ở đây
//      chỉ lẫn lộn), Configure Language (giao diện đã chốt tiếng Anh),
//      Configure Toolbars (thanh do style chỉnh, mục 18). Menu con dựng mới
//      mỗi lần mở (DolphinMainWindow::updateHamburgerMenu, 26.08.1) → làm như
//      mục 17: lúc polish + mỗi aboutToShow. Nhận ra theo tên hành động.
//  31. Nút chỉ có icon (không chữ) cao bằng nút có chữ, vuông — Kvantum tính
//      theo icon 16px (nút chọn thư mục Configure Dolphin cao 18px cạnh ô
//      nhập 40px, máy ảo user 2026-10-07). Như mục 16.
//  32. Danh sách trang hộp thoại cài đặt (KPageListView: Interface, View…):
//      ô hover/chọn cách hai mép và cách nhau PAGE_LIST_GAP (user
//      2026-10-07: đang sát nhau, sát mép).
//  33. Thanh trượt (cỡ icon ở Configure Dolphin → View) không khung focus
//      cyan, không sáng lên khi trỏ chuột (user 2026-10-07).
//  34. Configure Dolphin → View không có tab Compact: Samhie đã bỏ chế độ
//      Compact (files/scripts/dolphin-ui.sh). Nhận tab theo icon
//      view-list-details (viewsettingspage.cpp 26.08.1) — chữ đổi theo ngôn ngữ.
//  35. Danh sách Configure Keyboard Shortcuts bớt các dòng KHÔNG còn dùng
//      được ở Samhie (user 2026-10-07: "ẩn và không còn sử dụng"; ẩn khỏi
//      menu mà phím tắt vẫn chạy thì giữ), và dòng trùng việc dòng khác / không
//      phải lệnh (user, đợt 2 cùng ngày). Bộ lọc tìm của danh sách hiện lại
//      dòng ẩn → xoá hẳn dòng (hành động vẫn còn). Danh sách: REMOVED_SHORTCUTS.
//  36. Trang Interface của Configure Dolphin dùng icon nét riêng
//      (samhie-settings-interface) thay icon APP org.kde.dolphin — đổi tên đó
//      trong bộ icon là đổi cả icon Dolphin ở dock. Trang là KPageWidgetItem
//      KHÔNG có cha → tìm qua KPageWidgetModel::item() (API công khai KF6
//      WidgetsAddons — style link thư viện này, image đã có sẵn), đổi
//      Q_PROPERTY icon. Chỉ trong hộp thoại Dolphin.
//  37. Danh sách trong hộp thoại cài đặt (QListView trong KPageDialog, vd
//      Context Menu): ô hover cách mép PAGE_LIST_GAP, cách nhau
//      PAGE_LIST_GAP (user 2026-10-07); danh sách tích chọn không có viền
//      chọn ("chọn dòng" vô nghĩa), danh sách chọn (Trash: thùng rác từng ổ)
//      có nền + viền chọn như danh sách trang. Ô tích/icon/chữ lùi theo nội
//      dung cả lúc dò cú bấm (subElementRect) — không thì vùng bấm ô tích
//      lệch trái 8px, bấm giữa ô không ăn (user 2026-10-07).
//  38. Configure Dolphin → Context Menu bớt dòng không dùng được ở Samhie
//      (CONTEXT_MENU_HIDDEN). ServiceModel không xoá được dòng → chèn một lớp
//      lọc giữa model gốc và lớp sắp xếp/tìm của trang (trang không dùng
//      mapToSource; lưu/đặt lại đọc thẳng model gốc → dòng ẩn giữ nguyên).
//  39. Trang Context Menu: icon nét riêng cho Compress, Extract, Makefile,
//      Mount ISO, Git, 'Copy To'/'Move To', Run In Konsole, Set Folder Icon
//      (CONTEXT_MENU_ICONS) — lớp lọc mục 38 trả icon khác.
//  41. Find Action… (Ctrl+Alt+I, KCommandBar) không liệt kê hành động đã bỏ
//      (REMOVED_SHORTCUTS), hành động ẩn, Show Toolbar (user 2026-10-07: gõ
//      "menubar" vẫn ra Show Menubar). Lớp lọc chèn như mục 38.
//  40. Menu ☰ Dolphin không có More, Help chỉ còn Find Action… và About
//      Dolphin (user 2026-10-07). More (KHamburgerMenu dựng: Show Menubar +
//      bản chép File/Edit/View/Go/Tools/Settings) trùng menu chuột phải, ▦,
//      phím tắt — và lộ lại Group By / Show Additional Information mà
//      dolphin-ui.sh đã ẩn, Show Toolbar, Bookmarks. Nhận ra More theo hành
//      động options_show_menubar đứng đầu (chữ đổi theo ngôn ngữ). Help là
//      menu thật của thanh menu (dùng chung) → gỡ hành động trong HELP_HIDDEN
//      khỏi nó. Làm như mục 17: lúc polish + mỗi aboutToShow.
//  42. Hộp Add/Edit Places Entry (chuột phải sidebar Dolphin): ô Label vẽ như
//      ô nhập thường — bo góc, không khung cyan, chữ cách lề như ô Location
//      (user 2026-10-07). Hộp thoại có cha là sidebar (KFilePlacesView) nên
//      Kvantum tưởng ô Label là ô sửa tên trong danh sách (Kvantum.cpp 1.1.6:
//      cha của cha là QAbstractItemView → khung Highlight 1px vuông, chữ sát
//      mép, cao theo chữ) → hỏi Kvantum không kèm widget.
//  43. Hộp chọn icon (KIconDialog — "Choose an icon" của mục 42) chỉ có icon
//      của bộ Samhie; ô chọn nhóm chỉ còn nhóm bộ đó có, bỏ "Other". Nút
//      Browse… dùng icon samhie-browse (user 2026-10-07).
//  44. Hộp thoại chọn file của KDE (KFileWidget — Browse…, hộp Open/Save của
//      mọi app KDE), user 2026-10-07/08: thanh công cụ bỏ Compact View,
//      Bookmarks, thanh zoom, Reload, Show Preview; menu ⚙ chỉ còn Show
//      Hidden Files + Show Quick Filter; menu chuột phải bỏ Sorting, View
//      Mode, Reload, Open Containing Folder, dòng mờ ẩn khi menu mở. Phím
//      tắt vẫn chạy (hành động gắn trên hộp thoại, không phải trên menu).
//  45. Sidebar chỉ dùng icon NÉT (user 2026-10-08): thư mục thêm vào (Add to
//      Places, kéo thả) mang icon thư mục màu → đổi sang icon nét, lưu luôn
//      vào user-places.xbel (icon thư mục ở vùng file giữ nguyên); Add Entry
//      mở với icon nét; hộp chọn icon của Add/Edit Entry chỉ có icon nét.
//  46. Sidebar: vạch ngang 1px màu FRAME_LINE giữa mục mặc định (… Recycle
//      Bin) và thư mục người dùng ghim thêm, hai đầu cách viền như icon, trên
//      dưới thoáng bằng khe giữa hai mục (user 2026-10-08). Chỉ có khi có mục
//      ghim. Delegate riêng cho dòng Recycle Bin (PinnedGapDelegate).
//  47. Hàng màu thư mục (menu chuột phải thư mục, user 2026-10-08):
//      [Default] [Red] [Yellow] [Teal] [Grey] [Symbol ▾]; Symbol = No Symbol +
//      14 ký hiệu (FOLDER_SYMBOLS). Màu và ký hiệu đổi độc lập: plugin
//      Dolphin vẫn ghi .directory, FolderChangeWatcher (nghe KDirNotify qua
//      D-Bus) sửa lại Icon=folder-<ký hiệu>-<màu> và lưu hai khoá
//      X-Samhie-FolderSymbol / X-Samhie-FolderColor.
//      Menu Symbol có icon (vẽ tay — Samhie tắt icon menu) + ô tròn, nút màu
//      được tích theo thư mục đang chuột phải (đọc qua giao diện trợ năng
//      của vùng file Dolphin, folderUnderCursor).
//  48. Hộp Create Folder: hàng 5 màu + ô ký hiệu như mục 47, thay lưới 20
//      ô + Choose Icon… của KIO.
//  50. Menu Open With / Open Folder With có icon app (user 2026-10-08) —
//      Samhie tắt icon menu, nên chữ lùi + style tự vẽ icon (enableMenuIcons,
//      dùng chung với menu Symbol).
//  49. Ô đổi tên tại chỗ (F2) thấp vừa chữ: khung, vùng chọn, dấu nháy theo
//      dải chữ thay vì hộp dòng cao 1,45 em của Dongle (user 2026-10-08).
//  51. Thanh đường dẫn không có mũi tên › ở đầu (chỗ không có thư mục cha:
//      Recycle Bin, /) thì chữ lùi bằng bề ngang mũi tên — đứng cùng chỗ với
//      "Home" (user 2026-10-08: "Recycle Bin" dính mép trái).
//  53. Hộp Properties (Alt+Return, KPropertiesDialog — user 2026-10-08):
//      - General bỏ dòng Size on disk, Accessed, File system, Mounted on,
//        Mounted from; thanh Free space ngang nhãn, chữ dung lượng nhỏ hơn
//        nằm dưới thanh. Hộp co theo nội dung mới, dải tab không bị cắt chữ.
//      - Permissions: nhãn hai nhóm canh phải, cùng bề ngang → User/Group
//        thẳng cột với Owner/Group/Others.
//      - Details: cột giá trị thẳng cột General, ô Comment to (dòng thừa của
//        tab bỏ bằng /etc/xdg/baloofileinformationrc).
//      - Checksums (chỉ hộp của file): nhãn MD5… canh phải, nút Calculate /
//        ô mã + Copy bắt đầu thẳng cột General.
//      - Chữ có "&" trong ô chọn ("Can View & Modify Content") — Kvantum
//        coi "&" là phím tắt, nuốt mất.
//  52. Menu chuột phải Dolphin không có "Git Clone…" (plugin git của
//      dolphin-plugins, ngoài repo git) — giống gõ git clone trong Konsole.
//      Dấu trạng thái git trên file và menu Git trong repo vẫn giữ (user
//      2026-10-08).
//  30.Danh sách phím tắt (Configure Keyboard Shortcuts, máy ảo user
//      2026-10-07). KExtendableItemDelegate (KF 6.30) vẽ mỗi ô HAI lần: ô nội
//      dung đã cắt bớt bên trái chỗ cho "›", rồi riêng dải "›" (style của
//      QApplication, KHÔNG kèm widget); cả hai tự đặt vị trí ô đầu/giữa/cuối
//      → Kvantum coi ô dính nhau, vuông một phía → ô hover hai lớp có khấc.
//      → dải "›" không vẽ nền; ô nội dung vẽ nền một lần trên CẢ ô (gồm "›"),
//      đứng riêng, bo đủ góc; ô đang chọn = nền hover, không viền (viền chọn
//      nháy khi bấm ô Action). Ô hiện tại không khung focus (Kvantum vẽ trong
//      CE_ItemViewItem, không qua PE_FrameFocusRect của style này — áp cho
//      mọi danh sách). Dải sửa phím tắt mở dưới dòng không còn nét
//      Highlight dày 6px (TabConnectedWidget::paintEvent tự vẽ) — chặn sự
//      kiện vẽ của nó; các con vẫn tự vẽ.
//
// Kvantum chủ động hỗ trợ bị bọc trong QProxyStyle (Kvantum.cpp
// setSurfaceFormat: "qt5ct uses QProxyStyle").

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
#include <QPainter>
#include <QPainterPath>
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
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QListView>
#include <QHash>
#include <QSortFilterProxyModel>
#include <QPointer>
#include <QSet>
#include <QTreeWidget>
#include <QTimer>
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

namespace {

constexpr int SIDEBAR_INDENT = 8;  // bản vẽ: icon sidebar cách mép ~16px (KIO tự chừa 4)
constexpr int PANE_RADIUS = 12;    // góc viền vùng file Dolphin (bản vẽ ~12) = góc cửa sổ
constexpr int TOOL_PADDING = 6;    // nút icon trên thanh công cụ: icon + 6px mỗi phía
constexpr int TOOLBAR_SEPARATOR_HEIGHT = 18;  // icon thanh công cụ cao ~20px (mục 23)
constexpr int KEY_FOCUS_PAD = 6;   // ô nền focus bàn phím rộng hơn vòng radio + chữ (mục 25, 28)
constexpr int PAGE_LIST_GAP = 4;   // danh sách trang hộp thoại cài đặt: khe quanh mỗi ô (mục 32)
constexpr int TERMINAL_PAD = 6;    // panel Terminal Dolphin: lề trong thêm (mục 27)
constexpr int TERMINAL_GAP = 7;    // khe trên panel Terminal: + vạch ngăn dock 1px = 8 (mục 27)
constexpr int POPUP_MARGIN = 8;    // thẻ tìm cách mép vùng file
constexpr int POPUP_PADDING = 10;  // nội dung cách mép thẻ
constexpr int POPUP_RADIUS = 12;
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

QColor paneColor()
{
    QColor c(samhie::PANE);
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

// QTabWidget và con của nó tự vẽ — con có nền trong suốt nên thấy lớp này.
class PanePainter : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Paint) {
            if (auto *tw = qobject_cast<QTabWidget *>(obj))
                paint(tw);
            else
                paintTerminalDock(static_cast<QWidget *>(obj));
        }
        return false;
    }

private:
    // Mục 27: dock Terminal là thẻ thứ hai dưới vùng file, cách TERMINAL_GAP
    // (phần trên dock để trống — nền cửa sổ): nền card bo góc trên-trái, viền
    // trên + trái. Konsole tô nền cùng màu (Samhie.colorscheme) trong phần
    // lùi vào, góc bo nằm ngoài nó.
    static void paintTerminalDock(QWidget *dock)
    {
        const QRect all = dock->rect().adjusted(0, TERMINAL_GAP, 0, 0);
        QPainter p(dock);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillPath(leftOutline(QRectF(all), PANE_RADIUS, 0, true), paneColor());
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setPen(QPen(QColor(samhie::FRAME_LINE), 1));
        p.setBrush(Qt::NoBrush);
        p.drawPath(leftOutline(QRectF(all).adjusted(0.5, 0.5, 0, 0), PANE_RADIUS, 0, false));
    }

    static void paint(QTabWidget *tw)
    {
        const bool dolphin = tw->inherits("DolphinTabWidget");
        const QRect all = tw->rect();
        // Vùng tô: dưới dải tab (nếu dải đang hiện) tới đáy.
        QRect pane = all;
        const QTabBar *bar = tw->tabBar();
        if (bar && bar->isVisible() && tw->tabPosition() == QTabWidget::North)
            pane.setTop(bar->geometry().bottom() + 1);

        const int bottomRad = dolphin ? paneBottomRadius(tw) : 0;

        QPainter p(tw);
        p.setRenderHint(QPainter::Antialiasing);
        // Dolphin: góc bo ở ĐỈNH khung. Có dải tab thì dải nằm trong khung,
        // giữ nền cửa sổ — vùng file khi đó vuông góc ở trên.
        const QPainterPath fill = dolphin
            ? leftOutline(QRectF(pane), pane.top() == all.top() ? PANE_RADIUS : 0, bottomRad, true)
            : rectPath(QRectF(pane));
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillPath(fill, paneColor());
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        if (dolphin) {
            // Viền trên + trái (+ đáy), nét 1px rơi đúng giữa hàng điểm ảnh.
            // Không khép phải: đó là mép cửa sổ.
            p.setPen(QPen(QColor(samhie::FRAME_LINE), 1));
            p.setBrush(Qt::NoBrush);
            p.drawPath(paneBorder(tw));
        }
    }

    static QPainterPath rectPath(const QRectF &r)
    {
        QPainterPath path;
        path.addRect(r);
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
        // Viewport đổi cỡ (thanh trượt xuống/lên): vẽ lại cả thẻ — Qt chỉ vẽ
        // phần mới lộ ra, viền đáy cũ sẽ nằm lại giữa thẻ.
        if (event->type() == QEvent::Resize && !isSearchContents(w)) {
            if (auto *area = qobject_cast<QScrollArea *>(w->parentWidget()); area && area->widget())
                area->widget()->update();
            return false;
        }
        if (event->type() == QEvent::Paint && isSearchContents(w)) {
            QPainter p(w);
            p.setRenderHint(QPainter::Antialiasing);
            // Mép dưới thẻ = mép dưới phần NHÌN THẤY của khung nội dung, không
            // phải mép dưới khung: khung cao hơn thanh vài px (máy ảo
            // 2026-10-06: Dolphin tính Bar::preferredHeight với đúng 4px dưới
            // dòng nút thứ hai, khung nội dung thì theo sizeHint) → viewport
            // cắt mất viền đáy + hai góc bo. Lúc thanh đang trượt, thẻ cũng
            // co theo phần nhìn thấy — góc dưới luôn bo.
            qreal bottom = w->height();
            for (const QWidget *a = w->parentWidget(); a; a = a->parentWidget()) {
                bottom = qMin<qreal>(bottom, w->mapFrom(a, QPoint(0, a->height())).y());
                if (a->inherits("Search::Bar"))
                    break;
            }
            const QRectF card(QPointF(POPUP_MARGIN + 0.5, POPUP_MARGIN + 0.5),
                              QPointF(w->width() - POPUP_MARGIN - 0.5, bottom - 0.5));
            p.setPen(QPen(QColor(samhie::POPUP_BORDER), 1));
            p.setBrush(QColor(samhie::POPUP));
            p.drawRoundedRect(card, POPUP_RADIUS, POPUP_RADIUS);
        }
        return false;
    }
};

// Mục 24. Thanh đường dẫn không có bút chì "sửa đường dẫn" ở mép phải khi trỏ
// chuột (user 2026-10-06: trỏ vào đã hiện con trỏ chữ I). Nút KIO vẽ nó là
// cả khoảng trống bên phải thanh — bấm chỗ nào cũng vào chế độ sửa, con trỏ
// chữ I cũng do nó đặt — nên giữ nút, chỉ không vẽ: lúc chưa bấm, nút KHÔNG
// vẽ gì khác ngoài bút chì (KUrlNavigatorToggleButton::paintEvent, KIO 6.30);
// lúc đang sửa nó vẽ ✓ như cũ.
class NoEditPencil : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        return event->type() == QEvent::Paint && !static_cast<QAbstractButton *>(obj)->isChecked();
    }
};

// Mục 24. Panel Information của Dolphin (F11) luôn ẩn: Dolphin dựng lại nó
// theo trạng thái lưu lần trước (QMainWindow::restoreState — ai đã mở nó ở
// bản cũ thì nó tự hiện) → hễ được hiện là ẩn lại, ngay vòng sự kiện sau.
// Mục 30: dải sửa phím tắt (ShortcutEditWidget : TabConnectedWidget) không vẽ
// nét Highlight — paintEvent của nó chỉ có nét đó.
class NoPaint : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *, QEvent *event) override
    {
        return event->type() == QEvent::Paint;
    }
};

// Mục 35: hành động không còn dùng được ở Samhie → bỏ khỏi danh sách phím tắt.
// Đo 2026-10-07 (Dolphin 26.08.1, danh sách đầy đủ in từ hộp thoại).
const QSet<QString> REMOVED_SHORTCUTS = {
    // dolphin-ui.sh ẩn (visible=false → phím tắt chết theo)
    QStringLiteral("compact"), QStringLiteral("toggle_selection_mode"), QStringLiteral("view_properties"),
    // Selection Mode bản thanh công cụ: cùng tính năng đã bỏ
    QStringLiteral("toggle_selection_mode_tool_bar"),
    // Group By / Show Additional Information (dolphin-ui.sh ẩn menu cha)
    QStringLiteral("group_by_none"), QStringLiteral("group_by_same_as_sort"),
    QStringLiteral("group_accesstime"), QStringLiteral("group_comment"), QStringLiteral("group_creationtime"),
    QStringLiteral("group_modificationtime"), QStringLiteral("group_text"), QStringLiteral("group_rating"),
    QStringLiteral("group_size"), QStringLiteral("group_tags"), QStringLiteral("group_type"),
    QStringLiteral("show_accesstime"), QStringLiteral("show_comment"), QStringLiteral("show_creationtime"),
    QStringLiteral("show_modificationtime"), QStringLiteral("show_rating"), QStringLiteral("show_size"),
    QStringLiteral("show_tags"), QStringLiteral("show_type"),
    // Panel Information (style mục 24: hành động ẩn, F11 chết)
    QStringLiteral("show_information_panel"),
    // Dolphin tự ẩn khi thiếu phần mềm mà image không cài: kio-stash, KFind
    QStringLiteral("split_stash"), QStringLiteral("open_preferred_search_tool"),
    // Bỏ cùng tầng Configure của menu ☰ (mục 29)
    QStringLiteral("window_color_sheme"), QStringLiteral("switch_application_language"),
    QStringLiteral("options_configure_toolbars"),
    // Đợt 2 (user 2026-10-07: "dòng nào dư thừa thì bỏ luôn"):
    // cần app image không cài — Kompare; Filelight (không có thì hành động chỉ
    // mời cài)
    QStringLiteral("compare_files"), QStringLiteral("manage_disk_space"),
    // sắp theo dữ liệu Baloo: image không có bộ đánh chỉ mục, và chỗ gắn nhãn/
    // điểm/ghi chú đã bỏ (Assign Tags — mục 38, panel Information — mục 24)
    QStringLiteral("sort_by_rating"), QStringLiteral("sort_by_tags"), QStringLiteral("sort_by_comment"),
    // Unlock Panels: đã gỡ khỏi menu sidebar (mục 19) — mở khoá phá bố cục
    QStringLiteral("lock_panels"),
    // bản "toggle" của Search…/Filter… (Ctrl+F, Ctrl+I) — cùng việc, chỉ để
    // gắn lên thanh công cụ; nút 🔍 vẫn chạy
    QStringLiteral("toggle_search"), QStringLiteral("toggle_filter"),
    // không phải lệnh: "Location Bar" là chính thanh đường dẫn đặt trên thanh
    // công cụ; "Change View Mode" là nút chọn chế độ xem mà thanh của Samhie
    // không dùng (có ▦, Icons Ctrl+1, Details Ctrl+2)
    QStringLiteral("url_navigators"), QStringLiteral("view_mode"),
    // Mục 40: menu ☰ bỏ More và bớt Help (user 2026-10-07); dolphin-ui.sh xoá
    // luôn phím tắt Ctrl+M, Ctrl+B, F1, Shift+F1
    QStringLiteral("options_show_menubar"),
    QStringLiteral("add_bookmark"), QStringLiteral("add_bookmarks_list"), QStringLiteral("edit_bookmarks"),
    QStringLiteral("help_contents"), QStringLiteral("help_whats_this"), QStringLiteral("help_report_bug"),
    QStringLiteral("help_donate"), QStringLiteral("help_about_kde"),
};

// Mục 40: dòng bỏ khỏi menu Help (tên hành động của KHelpMenu, KF 6.30).
const QSet<QString> HELP_HIDDEN = {
    QStringLiteral("help_contents"),    // Dolphin Handbook: tài liệu theo giao diện Dolphin gốc
    QStringLiteral("help_whats_this"),  // What's This?: đã có tooltip
    QStringLiteral("help_report_bug"),  // Report Bug / Donate: tới KDE, không phải Samhie
    QStringLiteral("help_donate"),
    QStringLiteral("help_about_kde"),   // About KDE: Samhie không dùng Plasma
};

// Mục 35: xoá dòng của hành động trong REMOVED_SHORTCUTS, rồi nhóm con rỗng.
// ObjectRole = Qt::UserRole + 2 (kshortcutsdialog_p.h, KF 6.30) trả QAction.
void trimShortcutList(QTreeWidgetItem *parent)
{
    for (int i = parent->childCount() - 1; i >= 0; --i) {
        QTreeWidgetItem *it = parent->child(i);
        auto *a = qobject_cast<QAction *>(it->data(0, Qt::UserRole + 2).value<QObject *>());
        if (a && REMOVED_SHORTCUTS.contains(a->objectName())) {
            delete it;
            continue;
        }
        if (!a && it->childCount()) {
            trimShortcutList(it);
            if (!it->childCount())
                delete it;
        }
    }
}

// Mục 38: dòng trang Context Menu không dùng được ở Samhie (id =
// ServiceModel::DesktopEntryNameRole, contextmenusettingspage.cpp 26.08.1).
const QSet<QString> CONTEXT_MENU_HIDDEN = {
    // Assign Tags: nhãn cần Baloo (image không cài) — kservicemenurc đã tắt
    QStringLiteral("tagsfileitemaction"),
    // Link Files to Activities: Activity của Plasma, Hyprland không có
    QStringLiteral("kactivitymanagerd_fileitem_linking_plugin"),
    // Dropbox (cần ứng dụng Dropbox), Bazaar (quản lý phiên bản đã ngừng),
    // Mercurial/Subversion (plugin gọi lệnh hg/svn — image không cài; cài
    // trong toolbox thì Dolphin ở ngoài cũng không thấy)
    QStringLiteral("_version_control_Dropbox"),
    QStringLiteral("_version_control_Bazaar"),
    QStringLiteral("_version_control_Mercurial"),
    QStringLiteral("_version_control_Subversion"),
};

// Mục 39: icon riêng cho từng dòng (tên gốc là icon app Ark, icon LOẠI FILE,
// hoặc code-class chung cho mọi hệ quản lý phiên bản — tools/build-icons.py).
const QHash<QString, QString> CONTEXT_MENU_ICONS = {
    {QStringLiteral("compressfileitemaction"), QStringLiteral("samhie-compress")},
    {QStringLiteral("extractfileitemaction"), QStringLiteral("samhie-extract")},
    {QStringLiteral("makefileactions"), QStringLiteral("samhie-makefile")},
    {QStringLiteral("mountisoaction"), QStringLiteral("samhie-disk-image")},
    {QStringLiteral("_version_control_Git"), QStringLiteral("samhie-git")},
    // Ba dòng dùng chung icon với dòng khác (edit-copy, utilities-terminal,
    // folder-new — user 2026-10-07: "icon đang lặp lại")
    {QStringLiteral("_copy_to_move_to"), QStringLiteral("samhie-copy-move-to")},
    {QStringLiteral("runInKonsole"), QStringLiteral("samhie-run-in-konsole")},
    {QStringLiteral("setfoldericonitemaction"), QStringLiteral("samhie-folder-icon")},
};

class ContextMenuRowFilter : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::DecorationRole) {  // mục 39
            const QString icon = CONTEXT_MENU_ICONS.value(index.data(Qt::UserRole).toString());
            if (!icon.isEmpty())
                return QIcon::fromTheme(icon);
        }
        return QSortFilterProxyModel::data(index, role);
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        // Dòng mới chèn chưa có id (addRow: insertRow rồi mới setData) →
        // nhận trước, setData đổi dữ liệu thì lọc lại (dynamicSortFilter).
        const QString id = sourceModel()->index(row, 0, parent).data(Qt::UserRole).toString();
        return !CONTEXT_MENU_HIDDEN.contains(id);
    }
};

// Mục 41: Find Action… (KCommandBar) không liệt kê hành động Samhie đã bỏ.
// KCommandBarModel (KF 6.30) trả QAction ở Qt::UserRole, chỉ bỏ hành động
// TẮT, và lấy cả hành động con trong menu của hành động đã ẩn (đo container
// 2026-10-07: Group By > Audio…, Window Color Scheme: Breeze…).
class CommandBarFilter : public QSortFilterProxyModel
{
public:
    CommandBarFilter(QWidget *window, QObject *parent)
        : QSortFilterProxyModel(parent)
        , m_window(window)
    {
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        return !hidden(sourceModel()->index(row, 0, parent).data(Qt::UserRole).value<QAction *>());
    }

private:
    static bool removed(const QAction *a)
    {
        return !a->isVisible() || REMOVED_SHORTCUTS.contains(a->objectName())
            || a->objectName() == QLatin1String("new_bookmark_folder")  // menu Bookmarks (mục 40)
            || a->inherits("KToggleToolBarAction");                     // Show Toolbar (More → Settings)
    }

    bool hidden(const QAction *a, int depth = 0) const
    {
        if (!a)
            return false;
        if (removed(a))
            return true;
        // Nhóm vai trò của Group By / Show Additional Information: mọi
        // hành động có tên (trừ menu con Audio, Image…) đều đã bỏ. Dòng con
        // của menu con: nhóm của nó thuộc hành động menu con → xét lên.
        if (const QActionGroup *g = a->actionGroup(); g && depth < 3) {
            bool any = false, all = true;
            for (const QAction *m : g->actions()) {
                if (m->objectName().isEmpty() || m->menu())
                    continue;
                any = true;
                all = all && removed(m);
            }
            if (any && all)
                return true;
            if (hidden(qobject_cast<const QAction *>(g->parent()), depth + 1))
                return true;
        }
        // Dòng chỉ nằm trong menu của hành động đã bỏ — Window Color Scheme
        // (mục 29): Dolphin gắn menu của KColorSchemeMenu vào một QAction
        // con của cửa sổ (dolphinmainwindow.cpp 26.08.1).
        if (!m_scanned) {
            m_scanned = true;
            for (const QAction *w : m_window ? m_window->findChildren<QAction *>() : QList<QAction *>()) {
                if (w->menu() && removed(w))
                    m_hiddenMenus.insert(w->menu());
            }
        }
        bool inMenu = false;
        for (const QObject *o : a->associatedObjects()) {
            if (auto *menu = qobject_cast<const QMenu *>(o)) {
                if (!m_hiddenMenus.contains(menu))
                    return false;
                inMenu = true;
            }
        }
        return inMenu;
    }

    QPointer<QWidget> m_window;
    mutable bool m_scanned = false;
    mutable QSet<const QMenu *> m_hiddenMenus;
};

// Thư mục con của bộ icon đang dùng, đọc từ index.theme: (thư mục, Context=,
// có phải Type=Scalable). Bộ icon không đổi trong lúc app chạy → đọc một lần.
struct IconThemeDir {
    QString path;  // đường dẫn đầy đủ, có "/" ở cuối
    QString context;
    bool scalable;
};

const QList<IconThemeDir> &iconThemeDirs()
{
    static const QList<IconThemeDir> dirs = [] {
        QList<IconThemeDir> out;
        const QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                    QStringLiteral("icons/%1/index.theme").arg(QIcon::themeName()));
        if (file.isEmpty())
            return out;
        const QString root = QFileInfo(file).absolutePath() + QLatin1Char('/');
        QSettings theme(file, QSettings::IniFormat);
        for (const QString &dir : theme.value(QStringLiteral("Icon Theme/Directories")).toStringList()) {
            out.append({root + dir + QLatin1Char('/'), theme.value(dir + QStringLiteral("/Context")).toString(),
                        theme.value(dir + QStringLiteral("/Type")).toString() == QLatin1String("Scalable")});
        }
        return out;
    }();
    return dirs;
}

// Mục 45: icon NÉT = SVG trong thư mục Scalable, tô theo màu chữ (class
// ColorScheme-Text — tools/build-icons.py). Thư mục màu
// (tools/build-folder-icons.py), icon app thì không; bản 16px chỉ có ký hiệu
// (places/16) cũng không — chọn nó thì cỡ 20 của sidebar lại ra thư mục màu.
bool isStrokeIconFile(const QString &path)
{
    static QHash<QString, bool> cache;
    auto it = cache.constFind(path);
    if (it != cache.constEnd())
        return *it;
    bool stroke = false;
    if (path.endsWith(QLatin1String(".svg"))) {
        for (const IconThemeDir &d : iconThemeDirs()) {
            if (d.scalable && path.startsWith(d.path) && !path.mid(d.path.size()).contains(QLatin1Char('/'))) {
                QFile f(path);
                stroke = f.open(QIODevice::ReadOnly) && f.readAll().contains("ColorScheme-Text");
                break;
            }
        }
    }
    cache.insert(path, stroke);
    return stroke;
}

bool isStrokeIconName(const QString &name)
{
    for (const IconThemeDir &d : iconThemeDirs()) {
        const QString path = d.path + name + QStringLiteral(".svg");
        if (d.scalable && QFile::exists(path))
            return isStrokeIconFile(path);
    }
    return false;
}

// Icon nét thay cho icon thư mục MÀU của một mục sidebar (thư mục cục bộ).
// Home và Download có bản nét riêng như mục mặc định (user-places.xbel.in).
QString strokePlaceIcon(const QString &icon)
{
    if (icon == QLatin1String("user-home"))
        return QStringLiteral("samhie-user-home");
    if (icon == QLatin1String("folder-download"))
        return QStringLiteral("samhie-folder-download");
    return QStringLiteral("samhie-folder");
}

// Mục 46: vạch ngăn mục mặc định của sidebar (Home … Recycle Bin) với thư mục
// người dùng ghim thêm, trên dưới thoáng bằng khe giữa hai mục (user
// 2026-10-08). Mục ghim mới luôn nằm SAU Recycle Bin trong nhóm Places (KIO
// thêm vào cuối; mục mặc định của Samhie kết thúc bằng trash:/ —
// user-places.xbel.in). Khe thêm chia đôi: dòng Recycle Bin cao thêm ở dưới,
// dòng ghim đầu tiên cao thêm ở trên, vạch ở ranh giới — rê chuột/bấm vào
// nửa khe nào thì trúng dòng gần nó, như phần đệm của các dòng khác.
// Qt hỏi cỡ dòng KHÔNG kèm số dòng (QListViewPrivate::itemSize, qtbase
// 6.11) nên style không nới được một dòng → gắn delegate riêng cho đúng hai
// dòng đó (setItemDelegateForRow). Delegate chung của KIO giữ nguyên — KIO ép
// kiểu nó ở nhiều chỗ (xem mục 2) — và vẫn vẽ nội dung dòng.
class PinnedGapDelegate : public QAbstractItemDelegate
{
public:
    explicit PinnedGapDelegate(QListView *view)
        : QAbstractItemDelegate(view)
        , m_view(view)
    {
        setObjectName(QStringLiteral("samhie-pinned-gap"));
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return base()->sizeHint(option, index) + QSize(0, HALF_GAP);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        constexpr int LATERAL_MARGIN = 4;  // KIO s_lateralMargin, kfileplacesview.cpp 6.30
        const bool above = index.row() == m_above;  // dòng Recycle Bin: khe ở dưới
        QStyleOptionViewItem o(option);
        if (above)
            o.rect.setBottom(o.rect.bottom() - HALF_GAP);
        else
            o.rect.setTop(o.rect.top() + HALF_GAP);
        base()->paint(p, o, index);
        if (!above)
            return;
        // Lề trái thẳng icon; phải bớt 1: khung sidebar 1px nằm ngoài ô bên
        // trái nhưng trong ô bên phải → hai đầu cùng cách viền 12px (đo
        // container 2026-10-08; viewport đã lùi SIDEBAR_INDENT). Hàng điểm
        // ảnh cuối của dòng: rê chuột dòng dưới không xoá vạch.
        const int y = option.rect.bottom();
        p->save();
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(samhie::FRAME_LINE));
        p->drawRect(QRect(QPoint(o.rect.left() + LATERAL_MARGIN, y),
                          QPoint(o.rect.right() - LATERAL_MARGIN - SIDEBAR_INDENT + 1, y)));
        p->restore();
    }

    bool editorEvent(QEvent *e, QAbstractItemModel *m, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return base()->editorEvent(e, m, o, i);
    }

    bool helpEvent(QHelpEvent *e, QAbstractItemView *v, const QStyleOptionViewItem &o, const QModelIndex &i) override
    {
        return base()->helpEvent(e, v, o, i);
    }

    // Tìm dòng Recycle Bin có ngay sau nó một dòng hiện cùng nhóm, gắn
    // delegate cho hai dòng đó; gỡ khỏi dòng cũ (dòng xê dịch khi thêm/bớt/
    // kéo mục, KIO ẩn/hiện dòng).
    void update()
    {
        constexpr int URL_ROLE = 0x069CD12B, GROUP_ROLE = 0x0a5b64ee;  // kfileplacesmodel.h
        const QAbstractItemModel *model = m_view->model();
        if (!model)
            return;
        int above = -1, below = -1;
        for (int r = 0; r < model->rowCount() && above < 0; ++r) {
            const QModelIndex i = model->index(r, 0);
            if (m_view->isRowHidden(r) || i.data(URL_ROLE).toUrl().scheme() != QLatin1String("trash"))
                continue;
            int next = r + 1;
            while (next < model->rowCount() && m_view->isRowHidden(next))
                ++next;
            if (next < model->rowCount() && model->index(next, 0).data(GROUP_ROLE) == i.data(GROUP_ROLE)) {
                above = r;
                below = next;
            }
        }
        if (above == m_above && below == m_below)
            return;
        for (int r : {m_above, m_below}) {
            if (r >= 0 && m_view->itemDelegateForRow(r) == this)
                m_view->setItemDelegateForRow(r, nullptr);
        }
        m_above = above;
        m_below = below;
        for (int r : {above, below}) {
            if (r >= 0)
                m_view->setItemDelegateForRow(r, this);
        }
    }

private:
    // Mỗi dòng thêm 12px → vạch cách icon trên/dưới 23–24px trống, bằng khe
    // giữa hai mục (dòng 41, icon 20 — hình icon nét không kín khung; đo
    // container 2026-10-08).
    static constexpr int HALF_GAP = 12;

    QAbstractItemDelegate *base() const { return m_view->itemDelegate(); }

    QListView *m_view;
    int m_above = -1;
    int m_below = -1;
};

// Mục 43: hộp chọn icon (KIconDialog) chỉ liệt kê icon của bộ đang dùng
// (Samhie, user 2026-10-07), không lấy Breeze/hicolor mà bộ đó thừa kế.
// KIconDialogModel (KF 6.30) giữ đường dẫn file ở Qt::UserRole; KIconLoader
// đã bỏ tên trùng, giữ bản của bộ đứng trước → bản Samhie thắng bản Breeze.
// Mục 45: chọn icon cho mục sidebar thì chỉ icon nét.
class IconThemeFilter : public QSortFilterProxyModel
{
public:
    IconThemeFilter(bool strokeOnly, QObject *parent)
        : QSortFilterProxyModel(parent)
        , m_dir(QStringLiteral("/icons/%1/").arg(QIcon::themeName()))
        , m_strokeOnly(strokeOnly)
    {
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        const QString path = sourceModel()->index(row, 0, parent).data(Qt::UserRole).toString();
        return m_strokeOnly ? isStrokeIconFile(path) : path.contains(m_dir);
    }

private:
    const QString m_dir;
    const bool m_strokeOnly;
};

// Nhóm (Context=) có icon trong bộ đang dùng (strokeOnly: có icon nét), theo
// giá trị KIconLoader::Context mà ô chọn nhóm của KIconDialog lưu ở dữ liệu
// từng dòng (kiconloader.h KF 6.30: Any 0, Action 1, Application 2, Device 3,
// MimeType 4, Animation 5, Category 6, Emblem 7, Emote 8, International 9,
// Place 10, StatusIcon 11 — cố định trong ABI KF6; style không link
// KIconThemes nên ghi số). Bộ thêm nhóm mới thì ô chọn tự có.
QSet<int> iconThemeContexts(bool strokeOnly)
{
    static const QHash<QString, int> ids = {
        {QStringLiteral("Actions"), 1}, {QStringLiteral("Applications"), 2},
        {QStringLiteral("Devices"), 3}, {QStringLiteral("MimeTypes"), 4},
        {QStringLiteral("Categories"), 6}, {QStringLiteral("Emblems"), 7},
        {QStringLiteral("Emotes"), 8}, {QStringLiteral("Places"), 10},
        {QStringLiteral("Status"), 11},
    };
    QSet<int> found = {0};  // All
    for (const IconThemeDir &d : iconThemeDirs()) {
        if (!ids.contains(d.context))
            continue;
        bool any = !strokeOnly;
        if (strokeOnly && d.scalable) {
            const QStringList files = QDir(d.path).entryList({QStringLiteral("*.svg")}, QDir::Files);
            any = std::any_of(files.begin(), files.end(),
                              [&d](const QString &f) { return isStrokeIconFile(d.path + f); });
        }
        if (any)
            found.insert(ids.value(d.context));
    }
    return found;
}

// Mục 45: mục sidebar là thư mục cục bộ mang icon thư mục màu (Add to Places
// của Dolphin lấy icon của chính thư mục, kéo thả lấy KIO::iconNameForUrl)
// → đổi sang icon nét; icon thư mục trong vùng file giữ nguyên. Đổi qua
// KFilePlacesModel::editPlace (lưu vào ~/.local/share/user-places.xbel, mọi
// app thấy). Hàm đó không gọi được qua meta-object; style không link KIO
// (style nạp vào MỌI app Qt) → tìm hàm trong thư viện KIO mà app ĐÃ nạp (có
// KFilePlacesModel nghĩa là đã nạp). Hàm không ảo, gọi với `this` là tham
// số đầu (Itanium C++ ABI). Không tìm thấy (KIO đổi ABI) thì thôi, không đổi.
// appName rỗng: mục thêm kiểu này không gắn app; Add Entry thì hộp thoại đã
// đổi sẵn icon (polish) nên không tới đây.
void strokePlaceIcons(QAbstractItemModel *model, int first, int last)
{
    using EditPlace = void (*)(QAbstractItemModel *, const QModelIndex &, const QString &, const QUrl &,
                               const QString &, const QString &);
    static const auto editPlace = reinterpret_cast<EditPlace>(
        dlsym(RTLD_DEFAULT, "_ZN16KFilePlacesModel9editPlaceERK11QModelIndexRK7QStringRK4QUrlS5_S5_"));
    constexpr int URL_ROLE = 0x069CD12B, HIDDEN_ROLE = 0x0741CAAC, ICON_NAME_ROLE = 0x00a45c00;  // kfileplacesmodel.h
    if (!editPlace || !QIcon::hasThemeIcon(QStringLiteral("samhie-folder")))
        return;
    for (int row = first; row <= last; ++row) {
        const QModelIndex index = model->index(row, 0);
        bool device = false;
        QMetaObject::invokeMethod(model, "isDevice", Qt::DirectConnection, Q_RETURN_ARG(bool, device),
                                  Q_ARG(QModelIndex, index));
        const QString icon = index.data(ICON_NAME_ROLE).toString();
        const QUrl url = index.data(URL_ROLE).toUrl();
        if (device || index.data(HIDDEN_ROLE).toBool() || !url.isLocalFile() || icon.isEmpty()
            || icon.startsWith(QLatin1Char('/')) || isStrokeIconName(icon))
            continue;
        editPlace(model, index, index.data(Qt::DisplayRole).toString(), url, strokePlaceIcon(icon), QString());
    }
}

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
// Màu bỏ khỏi hàng (user 2026-10-08: còn Default, Red, Yellow, Teal, Grey).
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
//   [Default] [Red] [Yellow] [Teal] [Grey]  [Symbol ▾]
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
            QAction *grey = nullptr;
            for (QAction *s : sub->actions()) {
                if (s->icon().name() == QLatin1String("inode-directory"))
                    carrier = s;
                else if (s->icon().name() == QLatin1String("folder-grey"))
                    grey = s;
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
            if (grey)
                addButton(QStringLiteral("folder-grey"), QStringLiteral("grey"), QStringLiteral("Set folder color to Grey"),
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
        {QStringLiteral("grey"), QStringLiteral("Grey")},
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

// Mục 37: danh sách trong hộp thoại cài đặt (không tính danh sách trang).
bool isSettingsList(const QWidget *w)
{
    return qobject_cast<const QListView *>(w) && !w->inherits("KDEPrivate::KPageListView")
        && w->window()->inherits("KPageDialog");
}

// Mục 30: danh sách dùng KExtendableItemDelegate (ô có dải "›" bên trái).
const QAbstractItemView *extendableView(const QStyleOption *opt, const QWidget *w)
{
    auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
    auto *view = qobject_cast<const QAbstractItemView *>(w ? w : (vopt ? vopt->widget : nullptr));
    return view && view->itemDelegate() && view->itemDelegate()->inherits("KExtendableItemDelegate") ? view : nullptr;
}

class HideDock : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::ShowToParent)
            QMetaObject::invokeMethod(static_cast<QWidget *>(obj), &QWidget::hide, Qt::QueuedConnection);
        return false;
    }
};

// Mục 26. Popup neo góc trái — đặt lúc Show: cửa sổ nền (QWindow) đã có, Qt
// chưa gửi vị trí cho compositor (show_sys chạy sau QShowEvent).
class PopupAnchor : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Show) {
            if (QWindow *win = static_cast<QWidget *>(obj)->windowHandle())
                win->setProperty("_q_waylandPopupAnchor", QVariant::fromValue(Qt::Edges(Qt::TopEdge | Qt::LeftEdge)));
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

// Mục 51. KUrlNavigator (KIO 6.30, updateButtonVisibility) chỉ hiện mũi tên
// đầu thanh (KUrlNavigatorDropDownButton) khi còn thư mục cha → thiếu nó thì
// tên thư mục đầu dính mép khung. Lề trái của layout thanh = bề ngang mũi
// tên (sizeHint: nửa chiều cao) khi mũi tên ẩn và đang ở dạng nút (ô sửa
// đường dẫn KUrlComboBox ẩn). Nghe Show/Hide của mũi tên và ô sửa; xét sau
// một nhịp vì KIO đổi hai cái lần lượt.
class CrumbIndent : public QObject
{
public:
    using QObject::QObject;

    static bool watches(const QWidget *w)
    {
        return w && w->parentWidget() && w->parentWidget()->inherits("KUrlNavigator")
            && (w->inherits("KDEPrivate::KUrlNavigatorDropDownButton") || w->inherits("KUrlComboBox"));
    }

    static void schedule(QWidget *nav)
    {
        QTimer::singleShot(0, nav, [nav] {
            QWidget *dropDown = nullptr, *pathBox = nullptr;
            for (QWidget *c : nav->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
                if (c->inherits("KDEPrivate::KUrlNavigatorDropDownButton"))
                    dropDown = c;
                else if (c->inherits("KUrlComboBox"))
                    pathBox = c;
            }
            if (!dropDown || !nav->layout())
                return;
            const int indent = dropDown->isHidden() && (!pathBox || pathBox->isHidden())
                ? dropDown->sizeHint().width() : 0;
            QMargins m = nav->layout()->contentsMargins();
            if (nav->layoutDirection() == Qt::RightToLeft)
                m.setRight(indent);
            else
                m.setLeft(indent);
            if (m != nav->layout()->contentsMargins())
                nav->layout()->setContentsMargins(m);
        });
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Show || event->type() == QEvent::Hide)
            schedule(static_cast<QWidget *>(obj)->parentWidget());
        return false;
    }
};

// Mục 53. Hộp Properties. Tên widget trang General: kfilepropspluginwidget.ui
// (KIO 6.30). KIO tự hiện lại vài dòng sau khi tính xong (setSizeOnDisk,
// dòng ổ đĩa) → không ẩn mà dời chúng sang một widget ẩn: setVisible(true)
// của KIO khi đó không lộ ra, layout cũng tự bỏ ô của chúng.
const QStringList PROPS_HIDDEN_ROWS = {
    QStringLiteral("sizeOnDiskLabelLeft"), QStringLiteral("sizeOnDiskLabel"),
    QStringLiteral("accessTimeLabel_Left"), QStringLiteral("accessTimeLabel"),
    QStringLiteral("fsLabel_Left"), QStringLiteral("fsLabel"),
    QStringLiteral("mountPointLabel_Left"), QStringLiteral("mountPointLabel"),
    QStringLiteral("mountSrcLabel_Left"), QStringLiteral("mountSrcLabel"),
};
constexpr int CAPACITY_BAR = 6;      // = progressbar_thickness (tools/kvantum.py)
constexpr int CAPACITY_GAP = 6;      // thanh → chữ dung lượng
constexpr qreal CAPACITY_TEXT = 0.85;  // cỡ chữ dung lượng so với chữ thường

QFont capacityFont(const QWidget *bar)
{
    QFont f = bar->font();
    f.setPointSizeF(f.pointSizeF() * CAPACITY_TEXT);
    return f;
}

// Thanh Free space: KCapacityBar vẽ thanh 12px rồi chữ cùng cỡ ngay dưới,
// cả khối canh giữa nhãn → thanh nằm cao hơn chữ "Free space:" (ảnh user
// 2026-10-08). Vẽ lại: thanh progress của Kvantum (giống mọi thanh tiến độ)
// ngang giữa dòng chữ đầu, chữ nhỏ dưới thanh, canh trái như giá trị khác.
// Nhãn canh trên (setupPropertiesDialog) để dòng chữ đầu của hai bên trùng.
class CapacityBarPainter : public QObject
{
public:
    using QObject::QObject;

    static int heightFor(const QWidget *bar)
    {
        return bar->fontMetrics().height() / 2 + CAPACITY_BAR / 2 + CAPACITY_GAP
            + QFontMetrics(capacityFont(bar)).height();
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() != QEvent::Paint)
            return false;
        auto *bar = static_cast<KCapacityBar *>(obj);
        const QRect r = bar->contentsRect();
        const int middle = r.top() + bar->fontMetrics().height() / 2;
        QPainter p(bar);
        QStyleOptionProgressBar o;
        o.initFrom(bar);
        o.rect = QRect(r.left(), middle - CAPACITY_BAR / 2, r.width(), CAPACITY_BAR);
        o.minimum = 0;
        o.maximum = 100;
        o.progress = bar->value();
        o.textVisible = false;
        o.state |= QStyle::State_Horizontal;
        bar->style()->drawControl(QStyle::CE_ProgressBar, &o, &p, bar);
        p.setFont(capacityFont(bar));
        p.setPen(bar->palette().color(QPalette::WindowText));
        const QRect text(r.left(), middle + CAPACITY_BAR / 2 + CAPACITY_GAP, r.width(), r.bottom());
        const Qt::Alignment side = bar->layoutDirection() == Qt::RightToLeft ? Qt::AlignRight : Qt::AlignLeft;
        p.drawText(text, side | Qt::AlignTop, p.fontMetrics().elidedText(bar->text(), Qt::ElideRight, r.width()));
        return true;
    }
};

void setupPropertiesDialog(QWidget *dialog)
{
    auto *holder = dialog->findChild<QWidget *>(QStringLiteral("samhie-hidden-rows"), Qt::FindDirectChildrenOnly);
    if (!holder) {
        holder = new QWidget(dialog);
        holder->setObjectName(QStringLiteral("samhie-hidden-rows"));
        holder->hide();
    }
    for (const QString &name : PROPS_HIDDEN_ROWS) {
        if (auto *row = dialog->findChild<QWidget *>(name); row && row->parentWidget() != holder)
            row->setParent(holder);
    }
    if (auto *label = dialog->findChild<QLabel *>(QStringLiteral("freespaceLabel")))
        label->setAlignment(Qt::AlignRight | Qt::AlignTop);
    if (auto *bar = dialog->findChild<KCapacityBar *>(QStringLiteral("capacityBar"))) {
        if (!bar->property("_samhie_capacity").toBool()) {
            bar->setProperty("_samhie_capacity", true);
            bar->installEventFilter(new CapacityBarPainter(bar));
            bar->setMinimumHeight(CapacityBarPainter::heightFor(bar));
        }
    }
    // Permissions: hai QFormLayout (Access Permissions, Ownership) trong
    // QGroupBox, mỗi cái tự đo cột nhãn và Kvantum canh nhãn trái.
    QList<QFormLayout *> forms;
    for (QGroupBox *box : dialog->findChildren<QGroupBox *>()) {
        if (auto *form = qobject_cast<QFormLayout *>(box->layout()))
            forms << form;
    }
    int labelWidth = 0;
    for (QFormLayout *form : forms) {
        for (int r = 0; r < form->rowCount(); ++r) {
            if (QLayoutItem *item = form->itemAt(r, QFormLayout::LabelRole); item && item->widget())
                labelWidth = qMax(labelWidth, item->widget()->sizeHint().width());
        }
    }
    for (QFormLayout *form : forms) {
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        for (int r = 0; r < form->rowCount(); ++r) {
            QLayoutItem *item = form->itemAt(r, QFormLayout::LabelRole);
            if (!item || !item->widget())
                continue;
            item->widget()->setMinimumWidth(labelWidth);
            if (auto *label = qobject_cast<QLabel *>(item->widget()))  // chữ canh phải trong ô đã nới
                label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    // Tab Details/Checksums thêm sau (Details: khi đọc xong metadata) — dải
    // tab không cắt chữ, không nút cuộn → bề ngang tối thiểu của hộp tính cả
    // dải tab, và hộp tự nới khi có tab mới.
    if (auto *tabs = dialog->findChild<QTabWidget *>()) {
        tabs->tabBar()->setElideMode(Qt::ElideNone);
        tabs->setUsesScrollButtons(false);
    }
    // KIO khoá cỡ tối thiểu = cỡ lúc dựng (gồm các dòng vừa bỏ). Bỏ khoá để
    // layout tự đặt lại, rồi co về cỡ nội dung mới.
    dialog->setMinimumSize(0, 0);
    dialog->resize(dialog->sizeHint());
}

// Chạy một lần, lúc hộp sắp hiện (Show đến trước khi cửa sổ lên màn hình):
// mọi trang đã dựng, mọi widget đã polish.
class PropertiesDialogSetup : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Show && !obj->property("_samhie_props").toBool()) {
            obj->setProperty("_samhie_props", true);
            setupPropertiesDialog(static_cast<QWidget *>(obj));
        }
        return false;
    }
};

// Tab Details (baloo-widgets 26.08.1 FileMetaDataWidget): lưới nhãn | khe |
// giá trị, cột nhãn giãn 4 phần, giá trị 6 phần → nhãn trôi ra giữa hộp.
// Cột nhãn không giãn, rộng sao cho giá trị bắt đầu đúng chỗ giá trị trang
// General (mimeCommentLabel); ô Comment kéo tới mép phải ô tên file. Đo lại
// mỗi lần lưới đổi — baloo dựng lại lưới khi đọc xong metadata. Dòng đầu
// cách dải tab như icon đầu trang General (lưới của baloo không có lề).
constexpr int COMMENT_LINES = 4;

void alignDetails(QWidget *meta)
{
    QWidget *dialog = meta->window();
    auto *grid = qobject_cast<QGridLayout *>(meta->layout());
    auto *general = dialog->findChild<QWidget *>(QStringLiteral("mimeCommentLabel"));
    auto *nameEdit = dialog->findChild<QWidget *>(QStringLiteral("fileNameLineEdit"));
    if (!meta->isVisible() || !grid || !general || !dialog->inherits("KPropertiesDialog"))
        return;
    QWidget *value = nullptr;
    for (int r = 0; r < grid->rowCount() && !value; ++r) {
        if (QLayoutItem *item = grid->itemAtPosition(r, 2); item && item->widget() && !item->widget()->isHidden())
            value = item->widget();
    }
    if (!value)
        return;
    if (grid->columnStretch(0) != 0 || grid->columnStretch(2) != 1) {
        grid->setColumnStretch(0, 0);
        grid->setColumnStretch(2, 1);
        return;  // đo lại sau khi lưới xếp lại (LayoutRequest)
    }
    QWidget *icon = dialog->findChild<QWidget *>(QStringLiteral("iconButton"));
    if (!icon || icon->isHidden())
        icon = dialog->findChild<QWidget *>(QStringLiteral("iconLabel"));
    if (icon && !icon->isHidden()) {
        QMargins m = grid->contentsMargins();
        const int top = m.top() + icon->mapTo(dialog, QPoint()).y() - value->mapTo(dialog, QPoint()).y();
        if (top >= 0 && top != m.top()) {
            m.setTop(top);
            grid->setContentsMargins(m);
            return;
        }
    }
    const int delta = general->mapTo(dialog, QPoint()).x() - value->mapTo(dialog, QPoint()).x();
    if (delta != 0) {
        grid->setColumnMinimumWidth(0, qMax(0, grid->cellRect(0, 0).width() + delta));
        return;
    }
    if (!nameEdit || nameEdit->isHidden())
        return;
    for (QWidget *c : meta->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        if (!c->inherits("KCommentWidget"))
            continue;
        const int right = nameEdit->mapTo(dialog, QPoint(nameEdit->width(), 0)).x();
        const int width = right - c->mapTo(dialog, QPoint()).x();
        if (width > 0 && c->minimumWidth() != width)
            c->setMinimumWidth(width);
    }
}

// Tab Checksums (checksumswidget.ui, KIO 6.30): QFormLayout trong
// calculateWidget, nhãn MD5/SHA… | Calculate (bấm xong thành ô mã + Copy).
// Nới cột nhãn cho ô bên phải bắt đầu đúng chỗ giá trị trang General — ô mã
// hẹp lại (chữ dài vẫn cuộn trong ô), nút Copy giữ nguyên.
void alignChecksums(QWidget *calc)
{
    QWidget *dialog = calc->window();
    auto *form = qobject_cast<QFormLayout *>(calc->layout());
    auto *general = dialog->findChild<QWidget *>(QStringLiteral("mimeCommentLabel"));
    if (!calc->isVisible() || !form || !general || !dialog->inherits("KPropertiesDialog"))
        return;
    QWidget *label = nullptr, *field = nullptr;
    for (const char *name : {"md5Button", "md5LineEdit"}) {
        if (auto *f = calc->findChild<QWidget *>(QLatin1String(name)); f && !f->isHidden())
            field = f;
    }
    if (QLayoutItem *item = form->itemAt(0, QFormLayout::LabelRole))
        label = item->widget();
    if (!label || !field)
        return;
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    const int delta = general->mapTo(dialog, QPoint()).x() - field->mapTo(dialog, QPoint()).x();
    const int width = qMax(0, field->mapTo(calc, QPoint()).x() - label->x() - form->horizontalSpacing() + delta);
    for (int r = 0; r < form->rowCount(); ++r) {
        QLayoutItem *item = form->itemAt(r, QFormLayout::LabelRole);
        auto *l = item ? qobject_cast<QLabel *>(item->widget()) : nullptr;
        if (!l)
            continue;
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (delta != 0)
            l->setMinimumWidth(width);
    }
}

class DetailsAlign : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        auto *w = static_cast<QWidget *>(obj);
        switch (event->type()) {
        case QEvent::LayoutRequest:
        case QEvent::Show:
        case QEvent::Resize:
            QTimer::singleShot(0, w, [w] {
                if (w->objectName() == QLatin1String("calculateWidget"))
                    alignChecksums(w);
                else
                    alignDetails(w);
            });
            break;
        default:
            break;
        }
        return false;
    }
};

// Ô Comment: KCommentWidget đặt chiều cao CỐ ĐỊNH = số dòng chữ + 1
// (setHeightToFitContent, mỗi lần chữ đổi) → trống thì chỉ 2 dòng. Giữ ít
// nhất COMMENT_LINES dòng, chữ dài hơn thì để KIO nới.
class CommentHeight : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Show) {
            auto *edit = static_cast<QPlainTextEdit *>(obj);
            QTimer::singleShot(0, edit, [edit] {
                const QMargins m = edit->contentsMargins();
                const int want = COMMENT_LINES * QFontMetrics(edit->font()).lineSpacing() + m.top() + m.bottom();
                if (edit->height() < want)
                    edit->setFixedHeight(want);
            });
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

// Mục 49: ô đổi tên tại chỗ của Dolphin (KItemListRoleEditor — QTextEdit,
// kitemlistroleeditor.cpp 26.08.1). Dolphin đặt ô bằng hộp dòng của chữ
// (Dongle: cao 1,45 em, chữ hoa 0,4 em) → ô cao gấp mấy lần chữ, sát icon
// thư mục (user 2026-10-08). Giữ nguyên khung Dolphin đặt (chữ đứng đúng chỗ
// nhãn cũ), nền trong suốt; style vẽ ô bo góc ôm các dòng chữ + RENAME_PAD.
constexpr qreal RENAME_UP = 1.45;   // trên chân chữ: chữ hoa có dấu (Ố, Ẫ)
constexpr qreal RENAME_DOWN = 0.4;  // dưới chân chữ: chân g, y
constexpr int RENAME_PAD = 6;

bool isRenameEditor(const QWidget *w)
{
    return w && w->inherits("KItemListRoleEditor") && qobject_cast<const QTextEdit *>(w);
}

// Dải chữ (theo chân chữ) của từng dòng, toạ độ ô đổi tên: (dòng, x1, x2,
// chân chữ). Dòng = QTextLine trong khối đầu (tên file là một khối).
struct RenameLine {
    qreal x1, x2, baseline;
};

QList<RenameLine> renameLines(const QTextEdit *te, int from = -1, int to = -1)
{
    QList<RenameLine> out;
    const QTextBlock block = te->document()->firstBlock();
    const QTextLayout *layout = block.layout();
    if (!layout)
        return out;
    const QPointF origin = QPointF(te->frameWidth() - te->horizontalScrollBar()->value(),
                                   te->frameWidth() - te->verticalScrollBar()->value())
        + te->document()->documentLayout()->blockBoundingRect(block).topLeft() + layout->position();
    for (int i = 0; i < layout->lineCount(); ++i) {
        const QTextLine line = layout->lineAt(i);
        int a = line.textStart(), b = a + line.textLength();
        if (from >= 0) {
            a = qMax(a, from);
            b = qMin(b, to);
            if (a >= b)
                continue;
        }
        out.append({origin.x() + line.cursorToX(a), origin.x() + line.cursorToX(b),
                    origin.y() + line.y() + line.ascent()});
    }
    return out;
}

QRect renameBox(const QWidget *w)
{
    auto *te = static_cast<const QTextEdit *>(w);
    const QFontMetricsF fm(te->font());
    const QList<RenameLine> lines = renameLines(te);
    const qreal top = lines.isEmpty() ? te->frameWidth() + fm.ascent() : lines.first().baseline;
    const qreal bottom = lines.isEmpty() ? top : lines.last().baseline;
    const int y0 = qRound(top - RENAME_UP * fm.capHeight()) - RENAME_PAD;
    const int y1 = qRound(bottom + RENAME_DOWN * fm.capHeight()) + RENAME_PAD;
    return QRect(0, y0, w->width(), y1 - y0).intersected(w->rect());
}

// Dấu nháy ngắn cho ô đổi tên (như ShortCaret của ô nhập một dòng): Qt vẽ
// cao bằng hộp dòng → cursorWidth 0, lớp con trên viewport vẽ vạch theo chữ.
class TextEditCaret : public QWidget
{
public:
    explicit TextEditCaret(QTextEdit *edit)
        : QWidget(edit->viewport())
        , m_edit(edit)
    {
        setObjectName(QStringLiteral("samhie-text-caret"));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
        edit->setCursorWidth(0);
        const int flash = QApplication::cursorFlashTime();
        m_timer.setInterval(flash > 0 ? flash / 2 : 0);
        QObject::connect(&m_timer, &QTimer::timeout, this, [this] {
            m_on = !m_on;
            update();
        });
        QObject::connect(edit, &QTextEdit::cursorPositionChanged, this, [this] { restart(); });
        QObject::connect(edit, &QTextEdit::textChanged, this, [this] { restart(); });
        edit->viewport()->installEventFilter(this);
        edit->installEventFilter(this);
        setGeometry(edit->viewport()->rect());
        raise();
        restart();
    }

protected:
    bool eventFilter(QObject *o, QEvent *event) override
    {
        if (event->type() == QEvent::Resize && o == m_edit->viewport())
            setGeometry(m_edit->viewport()->rect());
        else if (event->type() == QEvent::FocusIn)
            restart();
        else if (event->type() == QEvent::FocusOut) {
            m_timer.stop();
            update();
        }
        return false;
    }

    void paintEvent(QPaintEvent *) override
    {
        if (!m_on || !m_edit->hasFocus())
            return;
        const QRect line = m_edit->cursorRect();
        const QFontMetricsF fm(m_edit->font());
        const qreal base = line.top() + fm.ascent();
        QPainter p(this);
        p.fillRect(QRectF(line.left(), qRound(base - 1.3 * fm.capHeight()), 1,
                          qRound(base + 0.4 * fm.capHeight()) - qRound(base - 1.3 * fm.capHeight())),
                   m_edit->palette().color(QPalette::Text));
    }

private:
    void restart()
    {
        m_on = true;
        if (m_edit->hasFocus() && m_timer.interval() > 0)
            m_timer.start();
        update();
    }

    QTextEdit *m_edit;
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
        if (w->inherits("KDEPrivate::KPageListView")) {
            QTimer::singleShot(0, w, [w] {
                if (!w->window()->inherits("DolphinSettingsDialog"))
                    return;
                const QWidget *a = w->parentWidget();
                while (a && !qobject_cast<const KPageView *>(a))
                    a = a->parentWidget();
                auto *model = a ? qobject_cast<KPageWidgetModel *>(static_cast<const KPageView *>(a)->model()) : nullptr;
                for (int r = 0; model && r < model->rowCount(); ++r) {
                    KPageWidgetItem *item = model->item(model->index(r, 0));
                    if (item && item->icon().name() == QLatin1String("org.kde.dolphin"))
                        item->setIcon(QIcon::fromTheme(QStringLiteral("samhie-settings-interface")));
                }
            });
        }
        if (w->inherits("TabConnectedWidget"))
            w->installEventFilter(&m_noPaint);  // mục 30
        if (w->inherits("KPropertiesDialog"))  // mục 53
            w->installEventFilter(&m_propsSetup);
        if (w->inherits("Baloo::FileMetaDataWidget")
            || (w->objectName() == QLatin1String("calculateWidget") && w->parentWidget()
                && w->parentWidget()->objectName() == QLatin1String("ChecksumsWidget")))
            w->installEventFilter(&m_detailsAlign);
        if (qobject_cast<QPlainTextEdit *>(w) && w->parentWidget() && w->parentWidget()->inherits("KCommentWidget"))
            w->installEventFilter(&m_commentHeight);
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
        w->removeEventFilter(&m_popup);
        if (isSearchContents(w))
            w->parentWidget()->removeEventFilter(&m_popup);
        w->removeEventFilter(&m_menuForwarder);
        w->removeEventFilter(&m_urlHover);
        w->removeEventFilter(&m_noPencil);
        w->removeEventFilter(&m_hideDock);
        w->removeEventFilter(&m_popupAnchor);
        w->removeEventFilter(&m_noPaint);
        w->removeEventFilter(&m_crumbIndent);
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
        // Mục 32: ô danh sách trang cách mép phải bằng khe bên trái (setSpacing
        // chỉ chừa trái/trên/dưới — mục dài hết khung bên phải).
        if (pe == PE_PanelItemViewItem && w && w->inherits("KDEPrivate::KPageListView")) {
            if (auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                QStyleOptionViewItem o(*vopt);
                const int right = static_cast<const QAbstractScrollArea *>(w)->viewport()->width() - 1 - PAGE_LIST_GAP;
                o.rect.setRight(qMin(o.rect.right(), right));
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
            if (w && tbopt->toolButtonStyle == Qt::ToolButtonIconOnly
                && (w->objectName() == QLatin1String("samhie-new-tab")
                    || (inDolphinLocationOrSearch(w) && !qobject_cast<QLineEdit *>(w->parentWidget()))
                    || (qobject_cast<QToolBar *>(w->parentWidget()) && w->window()->inherits("DolphinMainWindow")))) {
                const int side = qMax(o.iconSize.width(), o.iconSize.height()) + 2 * TOOL_PADDING;
                o.rect = centeredSquare(o.rect, side);
            }
            QProxyStyle::drawComplexControl(cc, &o, p, w);
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
        // Mục 53: "&" trong chữ ô chọn là chữ thật (QComboBox không có phím
        // tắt trong chữ) — Kvantum vẽ như nhãn nút, "&" thành gạch phím tắt.
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
                const QAbstractItemView *view = extendableView(opt, w);
                if (!view && isSettingsList(w) && o.index.isValid()) {  // mục 37
                    QStyleOptionViewItem panel(o);
                    panel.viewItemPosition = QStyleOptionViewItem::OnlyOne;
                    panel.rect.adjust(PAGE_LIST_GAP, PAGE_LIST_GAP / 2, -PAGE_LIST_GAP, -PAGE_LIST_GAP / 2);
                    // Danh sách tích chọn: "dòng đang chọn" vô nghĩa → không
                    // viền chọn. Danh sách chọn (thùng rác của từng ổ, trang
                    // Trash) thì giữ nền + viền chọn như danh sách trang.
                    if (o.features & QStyleOptionViewItem::HasCheckIndicator)
                        panel.state &= ~State_Selected;
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
            p->fillPath(path, paneColor());
            p->restore();
            return;
        }
        auto *button = qstyleoption_cast<const QStyleOptionButton *>(opt);
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
        // Mục 42: cao như ô nhập thường. Không kèm widget thì Kvantum nâng lên
        // cỡ mặc định của Qt (nội dung + khung — cao 56 thay vì 41, đo
        // container) → đưa nội dung cao 0: chiều cao chỉ còn theo font + khung
        // như khi có widget (ô nhập thường không dùng chiều cao nội dung).
        if (ct == CT_LineEdit && isFakeItemEditor(w))
            return QProxyStyle::sizeFromContents(ct, opt, QSize(size.width(), 0), nullptr);
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
            QAction *keys = nullptr, *prefs = nullptr;
            for (QAction *s : sub->actions()) {
                if (s->objectName() == QLatin1String("options_configure_keybinding"))
                    keys = s;
                else if (s->objectName() == QLatin1String("options_configure"))
                    prefs = s;
            }
            if (!prefs)
                continue;
            if (keys)
                menu->insertAction(a, keys);
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
    PopupPainter m_popup;
    MenuButtonForwarder m_menuForwarder;
    UrlButtonHover m_urlHover;
    NoEditPencil m_noPencil;
    HideDock m_hideDock;
    PopupAnchor m_popupAnchor;
    NoPaint m_noPaint;
    CrumbIndent m_crumbIndent;
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
