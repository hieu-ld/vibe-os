#!/usr/bin/env python3
"""
Lắp icon nét Hugeicons (theme/icons/hugeicons/) vào bộ icon Samhie.

    python3 tools/build-icons.py          # sinh lại + so hình với bản gốc (cần rsvg-convert + Pillow)
    python3 tools/build-icons.py --check  # chỉ kiểm file sinh ra có khớp nguồn, thư viện chuẩn (CI chạy)
    python3 tools/build-icons.py --tune   # dò lại bảng NUDGE (cần rsvg-convert + Pillow), in ra để chép vào

Chạy xong thì chạy tiếp tools/build-icon-theme.sh (sinh lại index.theme khi có
nhóm mới, kiểm từng SVG).

NGUỒN (theme/icons/hugeicons/, xem README ở đó): user tải từ hugeicons.com
(bộ free, Stroke Rounded, khung 24, nét 1,5), tên file = tên user đặt. Bảng
MAP bên dưới nói mỗi hình thành những tên freedesktop nào — một hình có thể
mang nhiều tên (nhà → samhie-user-home và go-home). Hình nằm trong UNUSED là
có sẵn, chưa có chỗ dùng. File không nằm trong bảng nào là báo lỗi, để hình
mới tải không nằm im mà không ai biết.

ĐỔI GÌ so với bản Hugeicons:
  * Màu. Hugeicons ghi `currentColor` (đôi khi `black`) mà không định nghĩa
    `color` ở đâu — trong KDE nó rơi về ĐEN. KDE và SDDM cần `currentColor` +
    class ColorScheme-Text + khối <style id="current-color-scheme"> để tô lại
    theo bảng màu (cách Breeze làm — xem tools/import-tabler-icon.sh). Màu lạ
    là báo lỗi, theo bảng INK.
  * Màu khai ở thẻ <svg> gốc (stroke/fill) được đẩy xuống TỪNG phần tử vẽ —
    class phải nằm trên phần tử vẽ, và gỡ fill="none" ở gốc mà không đẩy
    xuống thì mọi <path> thành tô đen kín. Thuộc tính không dính màu
    (stroke-width, linecap, linejoin) để nguyên ở gốc cho các phần tử thừa
    kế — giống icon Tabler đã chạy trong KDE/QtSvg.
  * width/height bị gỡ, chỉ giữ viewBox — icon co giãn theo chỗ dùng.
  * Dời ≤ nửa điểm ảnh cho nét rơi vào lưới (bảng NUDGE).
"""

import re
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "theme" / "icons" / "hugeicons"
THEME = ROOT / "files/system/usr/share/icons/Samhie"
# Bảng dời cho Quickshell (ô hover lẻ 1px quanh icon đã dời — xem NUDGE).
NUDGE_JS = ROOT / "files/system/usr/share/samhie/quickshell/bar/icon-nudge.js"

SVG_NS = "http://www.w3.org/2000/svg"
ET.register_namespace("", SVG_NS)

# Màu dự phòng nếu KDE không thay khối style — trùng breeze-dark như các icon
# Tabler trong bộ, để icon ra sáng trên nền tối chứ không đen.
FALLBACK = "#fcfcfc"

# Màu trong file Hugeicons → độ đục của currentColor. `black`: trang web ghi
# màu đó khi user chỉnh tay một hình (trash-full).
INK = {"currentcolor": 1.0, "black": 1.0, "#000000": 1.0}
DRAWN = {"path", "circle", "rect", "line", "ellipse", "polyline", "polygon"}

# Hình → [(nhóm, tên freedesktop), …]
MAP = {
    # Thanh trên (Quickshell, 18px). Nút nguồn ở thanh trên và màn hình đăng
    # nhập là `power`; mục "Shut Down" trong menu nguồn là `shut-down`.
    "power":      [("actions", "samhie-power")],
    "bell":       [("status", "notification-inactive"),
                   ("status", "notification-active")],   # chấm: BellButton.qml vẽ
    "noti-off":   [("status", "notifications-disabled")],
    "ar-bottom":  [("actions", "arrow-down")],
    # Nút cửa sổ
    "close":      [("actions", "window-close-symbolic")],
    "minus":      [("actions", "window-minimize-symbolic")],
    "max":        [("actions", "window-maximize-symbolic")],
    "min":        [("actions", "window-restore-symbolic")],
    # Clipboard (Win+V, 18px): ghim + xoá. edit-delete cũng là "Delete" của KDE.
    "pin":        [("actions", "pin")],
    "trash-delete": [("actions", "edit-delete")],
    # Menu nguồn (Quickshell PowerMenu, 20px) + màn hình đăng nhập (SDDM đọc
    # thẳng actions/, xem SamhieIcon.qml)
    "shut-down":  [("actions", "system-shutdown")],
    "restart":    [("actions", "system-reboot")],
    "sleep":      [("actions", "system-suspend")],
    "lock":       [("actions", "system-lock-screen"),
                   ("actions", "object-locked")],   # Lock Panels
    "logout":     [("actions", "system-log-out")],
    "login":      [("actions", "samhie-log-in")],   # nút đăng nhập SDDM
    # Mũi tên thẳng (20px: lịch, menu khay)
    "next-2":     [("actions", "go-next")],
    "prev-2":     [("actions", "go-previous")],
    "up-2":       [("actions", "go-up")],
    "down-2":     [("actions", "go-down")],
    # Mũi tên nhọn (chevron, 14–16px: quick settings, trang Wi-Fi, SDDM)
    "ar-right":   [("actions", "arrow-right")],
    "ar-left":    [("actions", "arrow-left")],
    "ar-top":     [("actions", "arrow-up")],
    # Nút "+" trên thanh tab Konsole/Dolphin (tên đo trong
    # libkonsoleprivate/libkonsoleapp của image, 2026-09-30)
    "plus":       [("actions", "tab-new")],
    # Nút New… (Konsole: Profiles, Appearance, Keyboard) — + trơn, cạnh chữ
    # thì ⊕ của nút tab nặng quá.
    "add":        [("actions", "list-add")],
    # Sidebar Dolphin (20px, tên lấy từ usr/share/samhie/user-places.xbel.in).
    # Nhà và tải về mang tên RIÊNG: user-home/folder-download là thư mục TO
    # (tools/build-folder-icons.py) — bộ scalable chỉ có một file cho mọi cỡ.
    "smart-home": [("places", "samhie-user-home"), ("actions", "go-home")],
    "download":   [("places", "samhie-folder-download")],
    "network":    [("places", "folder-network")],
    "trash-empty": [("places", "user-trash"),
                    ("actions", "trash-empty")],   # Move to Trash…, Empty Trash
    "trash-full": [("places", "user-trash-full")],
    "storage":    [("devices", "drive-harddisk")],   # mục Storage của sidebar
    # Thư mục người dùng thêm vào sidebar (Add to Places, kéo thả, Add Entry):
    # style "samhie" mục 45 đổi icon thư mục MÀU của mục đó sang tên này.
    # Cùng hình với `folder` (16px, nút chọn thư mục) — file riêng để căn
    # lưới cho cỡ sidebar 20px.
    "place-folder": [("places", "samhie-folder")],
    "recent":     [("actions", "document-open-recent")],
    # Thanh công cụ Dolphin (22px). Tên đo bằng chuỗi trong dolphin,
    # libdolphinprivate và KIO/KF6 của image: Copy Location = edit-copy-path,
    # nút sửa đường dẫn của KUrlNavigator = edit-entry, nút ☰ = application-menu.
    "icons":      [("actions", "view-list-icons"),
                   ("actions", "preferences-desktop-icons")],   # trang View, Configure Dolphin
    # Chế độ Details. Dolphin đặt tên lệch: Details = view-list-TREE,
    # view-list-details là chế độ Compact (Samhie ẩn) — đo
    # dolphinviewactionhandler.cpp 26.08. Hộp thoại mở file của KDE dùng
    # view-list-details cho kiểu "Detailed" → giữ cả hai.
    "details":    [("actions", "view-list-tree"), ("actions", "view-list-details")],
    "menu":       [("actions", "application-menu")],
    # Nút tìm thanh công cụ Dolphin; cũng là ô tìm tổng quan/clipboard.
    "search":     [("actions", "system-search"),
                   ("actions", "edit-find"),   # Search…, Toggle Search Bar
                   ("actions", "search")],     # Find Action… (KCommandBar)
    # Nút lùi/tiến hình tròn của Dolphin. Tên RIÊNG, không đè
    # go-previous/go-next: hai tên đó còn dùng ở menu khay, lịch.
    # dolphin-ui.sh gán tên này cho go_back/go_forward.
    "prev":       [("actions", "samhie-go-back")],
    "next":       [("actions", "samhie-go-forward")],
    # Thanh đường dẫn + thanh tìm Dolphin (16px). Tên đo 2026-10-06 bằng
    # chuỗi UTF-16 trong image: ✓ của KUrlNavigator khi đang sửa = dialog-ok
    # (libKF6KIOFileWidgets), × đóng thanh tìm = dialog-close (dolphin), ⌫
    # trong ô nhập = edit-clear-locationbar-* của QLineEdit (libQt6Widgets),
    # rơi về edit-clear (KF6Completion).
    "check":      [("actions", "dialog-ok"),
                   ("actions", "dialog-ok-apply")],   # nút Apply của mọi hộp thoại
    "x":          [("actions", "dialog-close")],
    "backspace":  [("actions", "edit-clear-locationbar-rtl"),
                   ("actions", "edit-clear-locationbar-ltr"),
                   ("actions", "edit-clear")],
    "filter":     [("actions", "view-filter")],
    # Zoom (Dolphin ẩn dòng Zoom trong menu ▦ — style mục 24 — nhưng tên chuẩn
    # còn hiện ở app khác: trình xem ảnh/PDF).
    "zoom-in":    [("actions", "zoom-in"),
                   ("actions", "format-font-size-more")],   # Enlarge Font (Konsole)
    "zoom-out":   [("actions", "zoom-out"),
                   ("actions", "format-font-size-less")],   # Shrink Font (Konsole)
    "zoom-default": [("actions", "zoom-original")],
    # Hành động KDE (menu chỉ có chữ từ 2026-10-06, nhưng tên vẫn hiện ở
    # thanh công cụ, hộp thoại, nút bấm).
    "copy":       [("actions", "edit-copy")],
    # Copy Location (hành động copy_location, menu chuột phải + danh sách Context Menu)
    "copy-link":  [("actions", "edit-copy-path")],
    "cut":        [("actions", "edit-cut")],
    "paste":      [("actions", "edit-paste")],
    "edit":       [("actions", "document-edit"), ("actions", "edit-entry"),
                   ("actions", "accessories-text-editor")],  # trang Editing (Configure Kate)
    "rename":     [("actions", "edit-rename")],
    "folder-new": [("actions", "folder-new")],
    "refresh":    [("actions", "view-refresh")],
    "split":      [("actions", "view-split-left-right")],
    "bookmark":   [("actions", "bookmarks")],
    "settings":   [("actions", "configure"),
                   # Configure Konsole… (Konsole xin settings-configure trước,
                   # Breeze có tên đó nên không rơi về configure)
                   ("actions", "settings-configure")],
    # Hộp thoại Configure Dolphin (user 2026-10-07; tên đo trong mã Dolphin
    # 26.08.1): nút chọn thư mục cạnh ô đường dẫn khởi động (16px,
    # folderstabssettingspage.cpp) và icon trang Context Menu (32px,
    # dolphinsettingsdialog.cpp). folder-open KHÔNG phải thư mục trong vùng
    # file (đó là `folder`, tools/build-folder-icons.py).
    "folder":     [("actions", "folder-open"),
                   ("actions", "system-file-manager-symbolic")],  # Open File Manager (Konsole)
    "context-menu": [("actions", "preferences-desktop-menu-edit")],
    # Tab General / Content Display của trang View (16px, viewsettingspage.cpp).
    # view-choose còn là "Adjust View Display Style…" — Samhie đã ẩn hành động đó.
    "general":    [("actions", "description")],
    "content-display": [("actions", "view-choose")],
    # Trang Interface: style "samhie" (mục 36) đổi icon của RIÊNG trang đó sang
    # tên này — trang vốn dùng icon app org.kde.dolphin (icon dock).
    "interface":  [("actions", "samhie-settings-interface")],
    # Danh sách Configure Dolphin → Context Menu (16px). Tên chung (icon hành
    # động, đổi ở đâu cũng hợp) …
    "bookmark-plus": [("actions", "bookmark-new")],      # Add to Places, Add Bookmark
    "duplicate":  [("actions", "edit-duplicate"),
                   ("actions", "tab-duplicate")],          # Clone Tab (Konsole)
    "new-windows": [("actions", "window-new")],
    "sort":       [("actions", "view-sort")],
    "hide":       [("actions", "hide_table_row"),        # tên của plugin Hide (KIO)
                   ("actions", "view-hidden")],
    # … và tên RIÊNG, style "samhie" gán cho đúng dòng (mục 39): tên gốc là
    # icon app Ark / icon LOẠI FILE (text-x-makefile, application-x-cd-image —
    # đổi chung thì file Makefile, .iso trong Dolphin đổi theo) / code-class
    # chung cho mọi hệ quản lý phiên bản.
    "compress":   [("actions", "samhie-compress")],
    "extract":    [("actions", "samhie-extract")],
    "makefile-action": [("actions", "samhie-makefile")],
    "disk-image": [("actions", "samhie-disk-image")],
    "git":        [("actions", "samhie-git")],
    "folder-export": [("actions", "samhie-copy-move-to")],   # 'Copy To' and 'Move To'
    "play-square": [("actions", "samhie-run-in-konsole")],   # Run In Konsole
    "folder-edit": [("actions", "samhie-folder-icon")],      # Set Folder Icon
    # Nút "Download Services…" (KNewStuff). Hình download của sidebar trông nhỏ
    # trên nút (user 2026-10-07) → hình riêng.
    "download-service": [("actions", "get-hot-new-stuff")],
    # Hộp thoại Configure Keyboard Shortcuts (nút Shortcuts… của trang Context
    # Menu, 16px). Tên in từ chính hộp thoại 2026-10-07 (Dolphin 26.08.1,
    # KF 6.30): mọi dòng còn rơi về Breeze, cộng bốn nút dưới hộp thoại.
    # kompare/filelight là tên icon APP — hai app đó không có trong image,
    # bản Flatpak dùng tên org.kde.*, nên đè ở đây chỉ đổi dòng của Dolphin.
    # Giữ logo `kde` (About KDE) như logo Dolphin ở dòng About Dolphin.
    "compare":    [("actions", "kompare")],                # Compare Files
    "file-new":   [("actions", "document-new")],           # Create File…
    "undo":       [("actions", "edit-undo")],              # Undo, Undo close tab
    "redo":       [("actions", "edit-redo")],
    "select-all": [("actions", "edit-select-all")],
    "invert-selection": [("actions", "edit-select-invert")],
    "close-tab":  [("actions", "document-close")],
    "quit":       [("actions", "application-exit")],
    "swap":       [("actions", "swap-panels")],            # Focus Other View / Places / Terminal
    "handbook":   [("actions", "help-contents")],
    "donate":     [("actions", "help-donate-usd"), ("actions", "help-donate")],
    "bug":        [("actions", "tools-report-bug")],
    "help":       [("actions", "help-contextual")],        # What's This?
    "unlock":     [("actions", "object-unlocked")],        # Unlock Panels (khoá: `lock`)
    "disk-usage": [("actions", "filelight")],              # Manage Disk Space Usage
    "menubar":    [("actions", "show-menu")],
    "properties": [("actions", "document-properties")],
    # Restore to Defaults (trang View) và nút Defaults của hộp thoại
    "restore":    [("actions", "view-restore"), ("actions", "document-revert")],
    "show-hidden": [("actions", "view-visible")],          # ẩn: `hide`
    "sidebar":    [("actions", "compass")],                # panel Places
    # Nút « thu khung Projects của Kate (plugin samhiekate, kate/samhie-kate.cpp) —
    # user nhờ chọn 2026-10-12: khung có ‹, khác ‹ › đầu dải tab.
    "sidebar-close": [("actions", "sidebar-collapse-left")],
    "preview":    [("actions", "view-preview")],
    "open-folder": [("actions", "document-open-folder"),  # Show Target
                    # Nút "Open file dialog" cạnh ô đường dẫn (KUrlRequester,
                    # kurlrequester.cpp KIO 6.30) — hộp Add/Edit Places Entry
                    # (user 2026-10-07). Breeze cũng vẽ document-open là
                    # thư mục mở, nên "Open…" ở app khác vẫn đúng nghĩa.
                    ("actions", "document-open")],
    # Nút Browse… của hộp chọn icon (KIconDialog): style "samhie" mục 43 gán
    # tên riêng — tên gốc folder-open là nút chọn thư mục ở Configure Dolphin.
    "browse":     [("actions", "samhie-browse")],
    # Panel Terminal (F4). Dolphin cho cả menu con "Location Bar" cùng tên này.
    "terminal-panel": [("actions", "dialog-scripts")],
    "stop":       [("actions", "process-stop")],
    "cancel":     [("actions", "dialog-cancel")],
    "print":      [("actions", "document-print")],
    # Dòng "Other Application…" cuối menu Open With (KIO KFileItemActions,
    # 16px — style "samhie" mục 50 hiện icon ở menu đó). Tên chung: ở đâu
    # khác cũng là "chạy/chọn chương trình".
    "open-with":  [("actions", "system-run")],
    # Konsole (user 2026-10-09, nhờ Claude tìm): hộp Configure Konsole (trang,
    # 32px) và Configure Keyboard Shortcuts / Notifications (16px). Tên đo
    # trong mã Konsole 26.08.1 (MainWindow, SessionController, ViewManager)
    # và KNotifyConfig — chỉ những tên còn rơi về Breeze.
    # Trang: tên gốc utilities-terminal (icon app), folder-temp (thư mục
    # /tmp trong Dolphin), image-jpeg (loại file JPEG) đổi chung là đổi chỗ
    # khác → style "samhie" mục 56 gán tên riêng cho đúng trang.
    "konsole-general": [("actions", "samhie-konsole-general")],
    "konsole-profiles": [("actions", "preferences-desktop-theme")],
    "memory":     [("actions", "memory")],
    "tab-bar":    [("actions", "preferences-tabs")],
    "temp-files": [("actions", "samhie-konsole-tempfiles")],
    "thumbnails": [("actions", "samhie-konsole-thumbnails")],
    # Hộp Edit Profile / Create New Profile (EditProfileDialog.cpp 26.08.1,
    # trang 32px). Tên gốc input-keyboard / input-mouse là icon thiết bị,
    # kcolorchooser là icon app → style "samhie" mục 56 gán tên riêng (trang
    # Appearance dùng chung hình Profiles, Keyboard chung hình danh sách Key
    # bindings — cũng 32px); General cũng như hộp Configure.
    "konsole-scrolling": [("actions", "preferences-scroll")],
    "keyboard":   [("actions", "preferences-desktop-keyboard")],
    "konsole-mouse": [("actions", "samhie-konsole-mouse")],
    "konsole-advanced": [("actions", "preferences-other")],
    # Nút Choose… (font) của trang Appearance, 16px
    "text-font":  [("actions", "preferences-desktop-font")],
    # Hành động Konsole (danh sách phím tắt, Find Action…)
    "info":       [("actions", "help-about"),            # About Konsole
                   # cột State + dòng "Show a message in a popup" của
                   # Configure Notifications; hộp thông báo QMessageBox
                   ("status", "dialog-information")],
    "play":       [("actions", "media-playback-start")], # "Play a sound"
    "encoding":   [("actions", "character-set")],        # Set Encoding
    "clear-history": [("actions", "edit-clear-history")],  # Clear Scrollback
    "select-text": [("actions", "edit-select")],          # Select Mode
    "exchange":   [("actions", "exchange-positions")],   # Swap panes
    "mouse":      [("actions", "input-mouse-symbolic")], # Allow Mouse Tracking
    "plugins":    [("actions", "plugins"),
                   ("actions", "preferences-plugin")],   # trang Plugins (Configure Kate)
    "web-search": [("actions", "preferences-web-browser-shortcuts")],  # Search for … with
    "quick-commands": [("actions", "quickopen")],
    "detach":     [("actions", "tab-detach")],
    "fullscreen": [("actions", "view-fullscreen")],
    "split-top-bottom": [("actions", "view-split-top-bottom")],
    "split-auto": [("actions", "view-split-auto")],
    "monitor":    [("actions", "visibility")],           # menu View (Monitor for …)
    # Icon loại file (user 2026-10-08/09): vùng file Dolphin, tab General hộp
    # Properties (48px). Mỗi hình ra MỘT tên chính ở đây; mọi tên loại file
    # khác của Breeze (~500) là liên kết tới tên chính, do
    # tools/build-mime-icons.py xếp nhóm. Loại phổ biến có nhãn chữ (kiểu
    # Hugeicons *-02), còn lại theo nhóm (file-*).
    "txt":        [("mimetypes", "text-plain")],
    "pdf":        [("mimetypes", "application-pdf")],
    "doc":        [("mimetypes", "x-office-document")],
    "xls":        [("mimetypes", "x-office-spreadsheet")],
    "ppt":        [("mimetypes", "x-office-presentation")],
    "csv":        [("mimetypes", "text-csv")],
    "zip":        [("mimetypes", "application-zip")],
    "rar":        [("mimetypes", "application-vnd.rar")],
    "jpg":        [("mimetypes", "image-jpeg")],
    "png":        [("mimetypes", "image-png")],
    "svg":        [("mimetypes", "image-svg+xml")],
    "gif":        [("mimetypes", "image-gif")],
    "html":       [("mimetypes", "text-html")],
    "css":        [("mimetypes", "text-css")],
    "xml":        [("mimetypes", "application-xml")],
    "wav":        [("mimetypes", "audio-x-wav")],
    "file-text":  [("mimetypes", "text-x-generic")],
    "file-code":  [("mimetypes", "text-x-script")],
    "file-terminal": [("mimetypes", "application-x-shellscript")],
    "file-braces": [("mimetypes", "application-json")],
    "file-cog":   [("mimetypes", "application-x-desktop")],
    "file-image": [("mimetypes", "image-x-generic")],
    "file-video": [("mimetypes", "video-x-generic")],
    "file-music": [("mimetypes", "audio-x-generic")],
    "file-zip":   [("mimetypes", "package-x-generic")],
    "file-box":   [("mimetypes", "application-x-rpm")],
    "file-database": [("mimetypes", "application-vnd.sqlite3")],
    "file-type":  [("mimetypes", "font-x-generic")],
    "file-key":   [("mimetypes", "application-pgp-keys")],
    "file-digit": [("mimetypes", "application-octet-stream")],
    "file-question-mark": [("mimetypes", "unknown")],
    # Chọn nhiều file → hộp Properties (KIO, 48px).
    "files":      [("actions", "document-multiple")],
    # Sao Rating, tab Details (KRatingPainter, KF 6.30): sao đã chọn = `rating`,
    # chưa chọn = `rating-unrated` (thiếu thì KDE làm mờ `rating` — sao rỗng
    # và sao chọn chỉ khác độ mờ). star-full = star tô kín.
    "star":       [("actions", "rating-unrated")],
    "star-full":  [("actions", "rating")],
    # Ký hiệu thư mục (user 2026-10-08): menu Symbol của hàng màu thư mục
    # Dolphin (style "samhie" mục 47, 16px) — cùng file là ký hiệu ghép vào
    # thư mục tô màu (tools/build-folder-icons.py đọc symbol-*.svg ở đây).
    **{f"symbol-{n}": [("actions", f"samhie-symbol-{n}")] for n in (
        "cloud", "design", "development", "documents", "download", "important",
        "library", "mail", "music", "pictures", "print", "tar", "temp", "videos")},
    # Ô quick settings (20px)
    "dark-mode":  [("actions", "samhie-dark-style")],
    "bell-off":   [("actions", "samhie-do-not-disturb")],
    "night-light": [("status", "redshift-status-on")],
    "wired":      [("devices", "network-wired-activated")],
    "bluetooth":  [("devices", "network-bluetooth")],
    # Wi-Fi: ô quick settings + 4 mức sóng trong danh sách mạng (WifiPage.qml:
    # >75% / >50% / >25% / còn lại). `ok` là tên chuẩn thứ năm của KDE, đi
    # cùng hình 2 vòng.
    "wifi":       [("status", "network-wireless-connected-100"),
                   ("status", "network-wireless-signal-excellent")],
    "wifi-2":     [("status", "network-wireless-signal-good"),
                   ("status", "network-wireless-signal-ok")],
    "wifi-1":     [("status", "network-wireless-signal-weak")],
    "wifi-0":     [("status", "network-wireless-signal-none")],
    # Loa theo mức (VolumeRow.qml): tắt / ≤35% / ≤70% / trên 70%.
    "volume-mute": [("status", "audio-volume-muted")],
    "volume-35":  [("status", "audio-volume-low")],
    "volume-70":  [("status", "audio-volume-medium")],
    "volume-full": [("status", "audio-volume-high")],
    # Loại ngõ ra trong trang Sound Output (SoundPage.qml): loa / tai nghe
    # (cả Bluetooth) / HDMI-DisplayPort.
    "output-speaker":    [("devices", "audio-speakers")],
    "output-headphones": [("devices", "audio-headphones")],
    "output-display":    [("devices", "video-display")],
    "input-mic":         [("devices", "audio-input-microphone")],  # Input của trang Sound (app Settings)
    # Loại thiết bị ở trang Bluetooth (app Settings) — theo `icon` của BlueZ.
    "bt-mouse":    [("devices", "input-mouse")],
    "bt-keyboard": [("devices", "input-keyboard")],
    "bt-phone":    [("devices", "phone")],
    "bt-gamepad":  [("devices", "input-gaming")],
    "bt-laptop":   [("devices", "computer-laptop")],
    # Terminal ở THƯ MỤC CỠ 16 (nhóm "apps/16"): chỗ 16px (Dolphin "Open
    # Terminal Here") lấy hình này, cỡ khác (dock 32, thanh trên 18) vẫn lấy
    # icon app nhiều màu ở apps/scalable (tools/build-app-icons.py). Thư mục
    # cỡ cố định đứng trước scalable trong index.theme nên thắng đúng ở 16px.
    "terminal":   [("apps/16", "utilities-terminal")],
    # Kate (rà Kate đợt 2, user 2026-10-12 nhờ Claude tìm / dùng lại hình sẵn
    # có — bản chép "kate-*" khi cỡ hiển thị khác hình gốc). Tên đo trong mã Kate
    # 26.08.1 / KTextEditor 6.30.0, chỉ những tên còn rơi về Breeze.
    # Trang hộp Configure Kate (16px — khác Konsole 32, đo container):
    # Color Themes / Editing / Plugins / Documents / LSP Client gắn thẳng vào
    # hình gốc cùng cỡ 16 (symbol-design, edit, plugins, folder,
    # symbol-development) — xem dưới bảng và các dòng đó.
    "save":         [("actions", "document-save")],                 # Open/Save (+ Save trong menu)
    "kate-behavior": [("actions", "preferences-system-windows-behavior")],  # Behavior
    "kate-session": [("actions", "view-history")],                  # Session
    "kate-feedback": [("actions", "preferences-desktop-locale")],   # User Feedback
    # Cột icon trái (Kate vẽ 32px — "Icon size for left and right sidebar
    # buttons", katemdi.cpp). Projects cũng là trang Projects của Configure.
    # Git: icon Kate NHÚNG trong mã (sc-apps-git.svg) → plugin samhiekate gắn
    # tên riêng lên nút. Symbols: quickopen-class rơi về ⌘ của quickopen —
    # trùng nút Quick Open.
    "kate-projects": [("actions", "project-open")],
    "kate-git":     [("actions", "samhie-kate-git")],
    "kate-outline": [("actions", "quickopen-class")],
    # Nút đáy cửa sổ (16px) + nút icon panel Search (plugin samhiekate mục 12)
    "kate-diagnostics": [("actions", "dialog-warning-symbolic")],
    "kate-output":  [("actions", "output_win")],
    "replace":      [("actions", "edit-find-replace")],             # Replace (+ Find/Replace của Kate)
    "replace-all":  [("actions", "samhie-replace-all")],            # Replace Checked
    "match-case":   [("actions", "samhie-match-case")],             # Aa (plugin mục 15)
    "regex":        [("actions", "samhie-regex")],                  # .*
    "search-details": [("actions", "samhie-search-details")],       # ⋯ Folder/Filter/Exclude
}

# Hình gốc dùng thêm cho trang Configure Kate (16px). folder khai nhiều dòng,
# symbol-* sinh bằng vòng lặp — gắn thêm tên ở đây cho dễ thấy.
MAP["folder"].append(("actions", "folder-documents-symbolic"))       # Documents
MAP["symbol-design"].append(("actions", "preferences-desktop-color"))  # Color Themes
MAP["symbol-development"].append(("actions", "format-text-code"))    # LSP Client (+ panel LSP)

# Có sẵn, chưa lắp. `wifi-lock`: chỉ có bản sóng đầy — danh sách Wi-Fi cần
# khoá đi cùng cả bốn mức, nên vẫn dùng ổ khoá nhỏ đè góc (WifiPage.qml).
UNUSED = {"wifi-lock"}

# Ô tích + nút tròn của theme Kvantum: tools/kvantum.py đọc nét và tô màu,
# không phải icon của bộ Samhie.
KVANTUM = {p.stem for p in SRC.glob("kvantum-*.svg")}

# Cỡ hiển thị CHÍNH của từng hình (đo trong Quickshell/SDDM/Dolphin) — để dò
# NUDGE. Hình không có ở đây: cỡ 16 (menu, thanh đường dẫn Dolphin).
SIZE = {
    18: ["power", "bell", "noti-off", "ar-bottom", "close", "minus", "max",
         "min", "pin", "trash-delete"],
    20: ["shut-down", "restart", "sleep", "lock", "logout", "login",
         "next-2", "prev-2", "up-2", "down-2",
         "smart-home", "download", "network", "trash-empty", "trash-full",
         "storage", "place-folder", "recent", "dark-mode", "bell-off", "night-light", "wired",
         "bluetooth", "wifi", "wifi-2", "wifi-1", "wifi-0",
         "volume-mute", "volume-35", "volume-70", "volume-full",
         "output-speaker", "output-headphones", "output-display", "input-mic",
         "bt-mouse", "bt-keyboard", "bt-phone", "bt-gamepad", "bt-laptop"],
    22: ["icons", "details", "menu", "search", "prev", "next"],
    32: ["context-menu", "interface", "konsole-general", "konsole-profiles", "memory",
         "tab-bar", "temp-files", "thumbnails",
         "konsole-scrolling", "keyboard", "konsole-mouse", "konsole-advanced",
         "kate-projects", "kate-git", "kate-outline"],
}


def main_size(name):
    return next((s for s, names in SIZE.items() if name in names), 16)


# Dịch hình (đơn vị SVG của khung 24) cho các hình thay nhau ở cùng một chỗ
# có phần chung đứng yên. Hugeicons căn giữa TỪNG hình: loa ít sóng thì thân
# loa đứng lệch phải (mép phải thân loa: tắt/đầy x = 14, 70% x = 16; thân
# rộng 12) → kéo âm lượng là loa nhảy ngang (user 2026-10-06). Dịch về chỗ
# của bản đầy. Bản 35% (Hugeicons `volume`) còn vẽ đuôi loa khác hẳn → file
# nguồn đã thay bằng thân loa của volume-full (xem README nguồn).
ALIGN = {
    "volume-70": (-2, 0),
}

# Icon loại file (vùng file Dolphin 48–64px, hộp Properties): đo ảnh máy ảo
# 2026-10-09, TXT cỡ 64 cao 58px, nét 4px — to và đậm hơn thư mục bên cạnh
# (khối màu 58×54). User chọn: thu còn 88% (khung giữ 24, phóng viewBox quanh
# tâm) và nét 1,25 thay 1,5 → cao ~51px, nét ~3,3px.
FILE_SCALE, FILE_STROKE = 0.88, "1.25"


def is_file_icon(name):
    return name == "files" or any(g == "mimetypes" for g, _ in MAP.get(name, []))


def thin(name, svg):
    return svg.replace('stroke-width="1.5"', f'stroke-width="{FILE_STROKE}"') if is_file_icon(name) else svg


# Hình thay nhau ở CÙNG một chỗ phải dời giống nhau, không thì đổi trạng thái
# (kéo âm lượng, sóng Wi-Fi, bật Do not disturb) là hình nhảy nửa điểm ảnh.
# --tune dò chung cả họ (mờ trung bình). Mũi tên trái/phải là ảnh gương nên
# dò riêng vẫn ra số đối xứng.
FAMILIES = [
    ("volume-mute", "volume-35", "volume-70", "volume-full"),
    ("wifi-0", "wifi-1", "wifi-2", "wifi"),
    ("bell", "noti-off"),
    ("trash-empty", "trash-full"),
]


# Dời hình cho nét rơi vào lưới điểm ảnh ở CỠ HIỂN THỊ CHÍNH của icon.
# Hugeicons vẽ khung 24, nét 1,5: thu về 20px thì nét 1,25px, vạch ở tâm
# (toạ độ 12 → 10px) nằm đúng ranh giới hai điểm ảnh → 2 hàng mờ. Dời cả hình
# (đổi gốc viewBox, không sửa nét) một phần điểm ảnh → một hàng đậm + viền
# mờ mảnh. Số dời dò bằng máy (`--tune`): thử mọi bước 1/8 điểm ảnh trong
# ±0,5, vẽ bằng rsvg ở cỡ đó, chọn bản ít điểm ảnh mờ nhất (mờ =
# Σ min(a, 1−a) / Σ a); chỉ giữ hình giảm mờ ≥ 0,06. Hình lệch tâm tối đa
# nửa điểm ảnh — mắt không thấy, nét thì thấy. Đổi hình thì chạy lại --tune.
# Họ loa và họ thùng rác: dò chung không giảm mờ đủ 0,06 nên không dời.
# tên hình: (cỡ px, dời ngang px, dời dọc px)
NUDGE = {
    "kate-diagnostics": (16, -0.5, -0.5),  # mờ 0.61 → 0.40
    "save": (16, -0.5, -0.125),          # mờ 0.52 → 0.36
    "kate-behavior": (16, 0.125, -0.5),  # mờ 0.50 → 0.41
    "kate-session": (16, -0.5, -0.5),    # mờ 0.61 → 0.37
    "kate-feedback": (16, 0, -0.25),     # mờ 0.46 → 0.34
    "replace-all": (16, -0.5, -0.5),     # mờ 0.56 → 0.47
    "regex": (16, 0.375, -0.375),        # mờ 0.51 → 0.39
    "search-details": (16, 0, -0.375),   # mờ 0.40 → 0.18
    "backspace": (16, -0.5, -0.5),       # mờ 0.57 → 0.40
    "bell": (18, -0.5, 0),               # mờ 0.37 → 0.31 (họ chuông)
    "noti-off": (18, -0.5, 0),
    "bell-off": (20, 0, -0.5),           # mờ 0.40 → 0.31
    "bluetooth": (20, -0.375, -0.5),     # mờ 0.45 → 0.29
    "bt-keyboard": (20, -0.375, 0.375),  # mờ 0.47 → 0.34
    "bt-laptop": (20, -0.5, -0.375),     # mờ 0.51 → 0.26
    "bt-mouse": (20, -0.5, -0.5),        # mờ 0.44 → 0.29
    "check": (16, 0, -0.5),              # mờ 0.53 → 0.41
    "copy": (16, 0.125, 0.125),          # mờ 0.46 → 0.39
    "cut": (16, -0.25, -0.25),           # mờ 0.51 → 0.44
    "down-2": (20, -0.5, -0.25),         # mờ 0.49 → 0.26
    "download": (20, -0.5, -0.5),        # mờ 0.48 → 0.31
    "edit": (16, -0.5, -0.5),            # mờ 0.52 → 0.41
    "filter": (16, 0, -0.5),             # mờ 0.60 → 0.33
    "browse": (16, 0, -0.375),           # mờ 0.49 → 0.34
    "bookmark-plus": (16, -0.5, 0.125),  # mờ 0.55 → 0.42
    "compress": (16, 0.125, -0.5),       # mờ 0.52 → 0.35
    "content-display": (16, -0.5, 0.125),  # mờ 0.51 → 0.39
    "copy-link": (16, -0.5, -0.5),       # mờ 0.70 → 0.33
    "disk-image": (16, -0.5, -0.25),     # mờ 0.56 → 0.38
    "download-service": (16, 0.125, -0.5),  # mờ 0.57 → 0.33
    "duplicate": (16, -0.5, -0.5),       # mờ 0.54 → 0.41
    "extract": (16, 0.125, -0.25),       # mờ 0.43 → 0.35
    "folder": (16, 0, -0.375),           # mờ 0.48 → 0.32
    "folder-edit": (16, 0.125, -0.25),   # mờ 0.47 → 0.34
    "folder-export": (16, 0.125, -0.375),  # mờ 0.50 → 0.35
    "general": (16, -0.5, -0.5),         # mờ 0.58 → 0.44
    "hide": (16, -0.5, -0.375),          # mờ 0.51 → 0.36
    "makefile-action": (16, 0, -0.5),    # mờ 0.50 → 0.43
    "play-square": (16, -0.5, -0.5),     # mờ 0.55 → 0.45
    "sort": (16, -0.5, 0.125),           # mờ 0.79 → 0.17
    "folder-new": (16, 0.125, -0.375),   # mờ 0.50 → 0.38
    "icons": (22, -0.5, -0.5),           # mờ 0.44 → 0.30
    "logout": (20, 0.25, 0.375),         # mờ 0.40 → 0.30
    "network": (20, 0.375, -0.5),        # mờ 0.44 → 0.37
    "next-2": (20, -0.25, -0.5),         # mờ 0.52 → 0.27
    "pin": (18, 0.25, -0.25),            # mờ 0.45 → 0.38
    "power": (18, -0.5, -0.125),         # mờ 0.46 → 0.40
    "prev-2": (20, 0.25, -0.5),          # mờ 0.52 → 0.27
    "refresh": (16, -0.5, -0.125),       # mờ 0.54 → 0.45
    "smart-home": (20, 0.125, -0.5),     # mờ 0.38 → 0.32
    "split": (16, -0.5, -0.5),           # mờ 0.68 → 0.33
    "terminal": (16, -0.5, -0.5),        # mờ 0.69 → 0.31
    "up-2": (20, -0.5, 0.25),            # mờ 0.49 → 0.26
    "wifi-0": (20, -0.5, 0.375),         # mờ 0.63 → 0.37 (họ Wi-Fi)
    "wifi-1": (20, -0.5, 0.375),
    "wifi-2": (20, -0.5, 0.375),
    "wifi": (20, -0.5, 0.375),
    # Hộp thoại Configure Keyboard Shortcuts (2026-10-07)
    "bug": (16, -0.5, -0.125),           # mờ 0.48 → 0.36
    "compare": (16, -0.5, -0.25),        # mờ 0.51 → 0.38
    "disk-usage": (16, -0.25, 0.25),     # mờ 0.51 → 0.40
    "invert-selection": (16, -0.5, 0.125),  # mờ 0.49 → 0.38
    "menubar": (16, -0.5, -0.5),         # mờ 0.60 → 0.37
    "preview": (16, -0.5, 0.375),        # mờ 0.51 → 0.44
    "properties": (16, -0.5, 0.25),      # mờ 0.57 → 0.46
    "open-with": (16, -0.5, -0.5),   # mờ 0.61 → 0.35
    "quit": (16, -0.5, -0.5),            # mờ 0.66 → 0.29
    "redo": (16, 0.25, 0.125),           # mờ 0.46 → 0.34
    "restore": (16, -0.5, 0.375),        # mờ 0.54 → 0.42
    "sidebar": (16, -0.5, -0.5),         # mờ 0.71 → 0.16
    "sidebar-close": (16, 0.125, -0.5),  # mờ 0.50 → 0.28
    # Konsole (2026-10-09)
    "detach": (16, -0.5, -0.5),          # mờ 0.54 → 0.43
    "exchange": (16, 0.125, -0.5),       # mờ 0.76 → 0.17
    "fullscreen": (16, -0.5, -0.5),      # mờ 0.73 → 0.42
    "mouse": (16, -0.5, 0),              # mờ 0.56 → 0.33
    "quick-commands": (16, -0.5, -0.5),  # mờ 0.62 → 0.23
    "select-text": (16, 0.125, 0.375),   # mờ 0.46 → 0.37
    "split-auto": (16, -0.5, -0.5),      # mờ 0.61 → 0.31
    "split-top-bottom": (16, -0.375, -0.5),  # mờ 0.59 → 0.37
    "add": (16, -0.5, -0.5),             # mờ 0.87 → 0.09
    "keyboard": (32, 0.375, -0.375),     # mờ 0.29 → 0.22
    "temp-files": (32, 0.25, 0.375),     # mờ 0.27 → 0.20
    "thumbnails": (32, -0.5, -0.5),      # mờ 0.29 → 0.19
    "swap": (16, 0.125, -0.5),           # mờ 0.76 → 0.17
    "undo": (16, -0.25, 0.125),          # mờ 0.46 → 0.35
    # Menu Symbol (2026-10-08)
    "symbol-download": (16, -0.5, 0.25),  # mờ 0.52 → 0.44
    "symbol-mail": (16, 0.125, -0.5),    # mờ 0.50 → 0.37
    "symbol-music": (16, -0.5, -0.125),  # mờ 0.55 → 0.33
}


def nudge_js():
    """icon-nudge.js: tên icon → [cỡ, nửa điểm ảnh dời ngang, dọc] (−1/0/1).
    Quickshell (bar/HoverBox.qml) thu ô hover 1px về phía icon dời, để ô có
    tâm ở giữa điểm ảnh như icon: icon cỡ chẵn đặt ở toạ độ nguyên trong ô
    chẵn, dời −0,5 → tâm nét lệch nửa điểm ảnh so với tâm ô; ô bớt 1px (lẻ)
    thì tâm ô cũng dời theo. Dời 0,25 làm tròn về 0 (lệch 0,25 kiểu nào cũng
    còn)."""
    def half(d):
        return round(d * 2) if abs(d) > 0.25 else 0
    rows = []
    for name, (size, dx, dy) in sorted(NUDGE.items()):
        hx, hy = half(dx), half(dy)
        if hx == 0 and hy == 0:
            continue
        for _, icon in MAP[name]:
            rows.append(f'    "{icon}": [{size}, {hx}, {hy}],')
    return (
        "// SINH TỰ ĐỘNG bởi tools/build-icons.py (bảng NUDGE) — đừng sửa tay.\n"
        ".pragma library\n\n"
        "// tên icon → [cỡ px, nửa điểm ảnh dời ngang, dọc]: −1 trái/lên, 1 phải/xuống.\n"
        "var NUDGE = {\n" + "\n".join(sorted(rows)) + "\n};\n\n"
        "// {x, y} (−1/0/1) của icon ở đúng cỡ đó; cỡ khác thì {0, 0}.\n"
        "function half(name, size) {\n"
        "    const n = NUDGE[name];\n"
        "    return n && n[0] === size ? { x: n[1], y: n[2] } : { x: 0, y: 0 };\n"
        "}\n"
    )


def view_box(name, original, nudge=None):
    """viewBox cuối của hình: dịch theo ALIGN, rồi dời theo NUDGE (hoặc theo
    `nudge` khi đang dò)."""
    vb = [float(v) for v in original.split()]
    if is_file_icon(name):
        grow = vb[2] / FILE_SCALE - vb[2]
        vb = [vb[0] - grow / 2, vb[1] - grow / 2, vb[2] + grow, vb[3] + grow]
    ax, ay = ALIGN.get(name, (0, 0))
    vb[0] -= ax
    vb[1] -= ay
    size, dx, dy = nudge or NUDGE.get(name, (1, 0, 0))
    unit = vb[2] / size          # đơn vị SVG trên một điểm ảnh
    vb[0] -= dx * unit
    vb[1] -= dy * unit
    return " ".join(f"{v:g}" for v in vb)


def q(tag):
    return f"{{{SVG_NS}}}{tag}"


def convert(name, text, nudge=None):
    """Trả về SVG đã chuyển đổi (chuỗi), hoặc thoát với lỗi."""
    root = ET.fromstring(text)
    if "viewBox" not in root.attrib:
        sys.exit(f"{name}.svg: thiếu viewBox")
    for a in ("width", "height", "color"):
        root.attrib.pop(a, None)
    root.set("viewBox", view_box(name, root.get("viewBox"), nudge))
    for el in root.iter():
        el.attrib.pop("id", None)

    # Màu ở gốc → đẩy xuống phần tử vẽ (xem đầu file).
    inherited = {p: root.attrib.pop(p) for p in ("stroke", "fill") if p in root.attrib}

    painted = 0
    for el in root.iter():
        tag = el.tag.split("}")[-1]
        if el is root:
            continue
        if tag not in DRAWN:
            sys.exit(f"{name}.svg: có <{tag}> — script chưa biết xử lý")
        decls = []
        for prop in ("stroke", "fill"):
            v = el.attrib.pop(prop, inherited.get(prop))
            if v is None:
                continue
            if v == "none":
                decls.append(f"{prop}:none")
                continue
            alpha = INK.get(v.lower())
            if alpha is None:
                sys.exit(f"{name}.svg: màu lạ {v} — chỉ nhận {', '.join(INK)}")
            decls.append(f"{prop}:currentColor")
            if alpha != 1.0:
                decls.append(f"{prop}-opacity:{alpha}")
        el.set("style", ";".join(decls))
        el.set("class", "ColorScheme-Text")
        painted += 1
    if painted == 0:
        sys.exit(f"{name}.svg: không có phần tử vẽ nào")

    style = ET.Element(q("style"), {"id": "current-color-scheme", "type": "text/css"})
    style.text = f".ColorScheme-Text {{ color: {FALLBACK}; }}"
    defs = ET.Element(q("defs"))
    defs.append(style)
    root.insert(0, defs)

    ET.indent(root, space="  ")
    body = thin(name, ET.tostring(root, encoding="unicode"))
    return (
        "<?xml version='1.0' encoding='utf-8'?>\n"
        f"<!-- Samhie — Hugeicons (MIT, xem LICENSE.hugeicons), nguồn theme/icons/hugeicons/{name}.svg.\n"
        "     Sinh bằng tools/build-icons.py, đừng sửa tay. -->\n"
        f"{body}\n"
    )


def dest(group, icon):
    """Đường dẫn file đích. Nhóm có cỡ ("apps/16") là thư mục cỡ cố định."""
    return THEME / group / f"{icon}.svg" if "/" in group else THEME / group / "scalable" / f"{icon}.svg"


def check_tables():
    names = {p.stem for p in SRC.glob("*.svg")}
    stray = names - MAP.keys() - UNUSED - KVANTUM
    missing = MAP.keys() - names
    sized = [n for ns in SIZE.values() for n in ns]
    if stray:
        sys.exit(f"theme/icons/hugeicons/ có hình chưa nằm trong MAP hay UNUSED: {', '.join(sorted(stray))}")
    if missing:
        sys.exit(f"MAP nhắc tới hình không có trong theme/icons/hugeicons/: {', '.join(sorted(missing))}")
    if set(sized) - MAP.keys() or len(sized) != len(set(sized)):
        sys.exit("SIZE nhắc tới hình không có trong MAP, hoặc một hình hai cỡ")


def outputs():
    """{đường dẫn đích: nội dung} cho mọi icon, sau khi kiểm các bảng."""
    check_tables()
    out = {}
    for name, targets in sorted(MAP.items()):
        svg = convert(name, (SRC / f"{name}.svg").read_text())
        for group, icon in targets:
            out[dest(group, icon)] = svg
    return out


def stale_outputs(out):
    """File do script này sinh trước đây mà giờ không còn trong MAP."""
    mark = "Sinh bằng tools/build-icons.py"
    return [p for p in THEME.rglob("*.svg")
            if not p.is_symlink() and p not in out and mark in p.read_text()]


def render(svg_text, size, bg=None):
    from PIL import Image
    with tempfile.TemporaryDirectory() as tmp:
        svg, png = Path(tmp, "a.svg"), Path(tmp, "a.png")
        svg.write_text(svg_text)
        cmd = ["rsvg-convert", "-w", str(size), "-h", str(size), "-o", str(png), str(svg)]
        if bg:
            cmd[1:1] = ["-b", bg]
        subprocess.run(cmd, check=True)
        return Image.open(png).convert("RGBA" if bg is None else "RGB")


def blur(name, size, dx, dy):
    """Phần mực nằm ở điểm ảnh mờ (0 = mọi điểm ảnh hoặc kín hoặc trống)."""
    img = render(convert(name, (SRC / f"{name}.svg").read_text(), (size, dx, dy)), size)
    alphas = [p[3] / 255 for p in img.getdata()]
    return sum(min(a, 1 - a) for a in alphas) / sum(alphas)


def tune():
    steps = [i / 8 for i in range(-4, 5)]
    family = {n: fam for fam in FAMILIES for n in fam}
    groups = sorted({family.get(n, (n,)) for n in MAP})
    print("NUDGE = {")
    for group in groups:
        size = main_size(group[0])
        if any(main_size(n) != size for n in group):
            sys.exit(f"FAMILIES: {', '.join(group)} không cùng cỡ trong SIZE")

        def total(dx, dy):
            return sum(blur(n, size, dx, dy) for n in group) / len(group)
        base = total(0, 0)
        best = min((total(dx, dy), abs(dx) + abs(dy), dx, dy)
                   for dx in steps for dy in steps)
        if base - best[0] >= 0.06:
            for n in group:
                print(f'    "{n}": ({size}, {best[2]:g}, {best[3]:g}),'
                      f'   # mờ {base:.2f} → {best[0]:.2f}')
    print("}")


def compare(name, dest):
    """Vẽ bản Hugeicons và bản đã chuyển cùng một màu, so từng điểm ảnh."""
    from PIL import ImageChops

    src = (SRC / f"{name}.svg").read_text()
    head, tail = src.split(">", 1)
    vb = view_box(name, re.search(r'viewBox="([^"]*)"', head).group(1))
    head = re.sub(r'viewBox="[^"]*"', f'viewBox="{vb}"', head)
    src = thin(name, re.sub(r'(?i)"(currentColor|black|#000000)"', '"#a1a1a1"', head + ">" + tail))
    new = dest.read_text().replace(FALLBACK, "#a1a1a1")
    a, b = render(src, 128, "#0a0a0a"), render(new, 128, "#0a0a0a")
    diff = ImageChops.difference(a, b).convert("L")
    return sum(1 for v in diff.getdata() if v > 24)


def main():
    args = sys.argv[1:]
    if "--tune" in args:
        check_tables()
        tune()
        return
    check = "--check" in args
    out = outputs()
    out[NUDGE_JS] = nudge_js()
    stale = stale_outputs(out)

    if check:
        old = [p for p, s in out.items() if not p.exists() or p.read_text() != s] + stale
        for p in old:
            print(f"CŨ: {p.relative_to(ROOT)}", file=sys.stderr)
        if old:
            sys.exit("Icon sinh ra không khớp theme/icons/hugeicons/ — chạy python3 tools/build-icons.py rồi commit.")
        print(f"icon: {len(out)} file khớp nguồn ({len(MAP)} hình, {len(UNUSED)} có sẵn chưa lắp)")
        return

    for p in stale:
        print(f"  xoá {p.relative_to(THEME)} (không còn trong MAP)")
        p.unlink()
    for p, s in out.items():
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(s)

    # So hình: mỗi hình một lần (các tên phụ cùng nội dung).
    worst = 0
    for name, targets in sorted(MAP.items()):
        bad = compare(name, dest(*targets[0]))
        worst = max(worst, bad)
        mark = "" if bad == 0 else f"  ← lệch {bad} điểm ảnh"
        print(f"  {name:12} → {', '.join(i for _, i in targets)}{mark}")
    print(f"\n{len(out)} file từ {len(MAP)} hình. Có sẵn chưa lắp: {', '.join(sorted(UNUSED))}")
    if worst > 0:
        sys.exit("Có hình vẽ ra khác bản Hugeicons (ở 128px) — xem lại trước khi commit.")


if __name__ == "__main__":
    main()
