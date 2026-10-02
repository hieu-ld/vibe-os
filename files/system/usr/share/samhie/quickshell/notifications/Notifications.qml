// Máy chủ thông báo của Samhie (thay mako — giai đoạn 3, user chốt 2026-10-01):
// nhận thông báo của app (D-Bus org.freedesktop.Notifications), hiện popup ở
// góc trên-phải (NotificationPopups.qml), giữ lại một danh sách cho chuông
// trên thanh (bar/BellButton.qml + bar/NotificationCenter.qml) như bản 1.0:
// xem lại, Clear All, Đừng làm phiền.
//
// Popup tự tắt thì BÁO APP là đã hết hạn (expire), danh sách chỉ giữ BẢN CHÉP.
// Không giữ thông báo "sống" mãi được: `notify-send -A` (script chụp màn hình
// dùng) chờ tới khi thông báo đóng — giữ mãi là script treo mãi.
import QtQuick
import Quickshell
import Quickshell.Services.Notifications

Scope {
    id: root

    // Đừng làm phiền: không hiện popup (trừ thông báo khẩn), vẫn vào danh sách.
    // Nhớ qua lần đăng nhập sau: quicksettings/Settings.qml đọc/ghi.
    property bool doNotDisturb: false
    // Popup bị chặn trên màn hình đang mở danh sách của chuông (thông báo mới
    // đã nằm sẵn trong danh sách đó).
    property bool centerOpen: false
    // Số thanh đang mở một popup (lịch, menu nguồn, khay, danh sách chuông):
    // popup thông báo nằm tầng Overlay, sẽ đè lên chúng → tạm ẩn (vẫn đếm giờ).
    property int barPopups: 0

    readonly property int historyLimit: 50
    readonly property int defaultTimeout: 5000   // ms, khi app để server tự chọn
    readonly property int longTimeout: 10000     // ms, thông báo khẩn / app xin không tự tắt

    // Danh sách cho chuông, mới nhất ĐẦU — các Entry bên dưới.
    property var history: []
    // Popup đang hiện, mới nhất ĐẦU — cũng là các Entry.
    property var popups: []

    // Một thông báo. `n` tự về null khi thông báo đóng (app rút, hết hạn, bị
    // bỏ); chữ/ảnh là BẢN CHÉP, cập nhật theo app khi `n` còn sống.
    component Entry: QtObject {
        id: entry
        property Notification n
        property string appName
        // Icon của thẻ (tên icon hoặc url) và ảnh xem trước (url, "" = không có).
        property string icon
        property string preview
        property string summary
        property string body
        property string desktopEntry
        property bool critical
        property date time: new Date()
        property bool released: false
        // Popup tự ẩn sau bao lâu (ms) — popup nào cũng tự ẩn (user 2026-10-02).
        property int timeout
        // Ẩn popup nhưng KHÔNG báo app hết hạn: thông báo khẩn, hoặc app xin
        // "không tự tắt" (thời hạn 0) — nó vẫn sống trong danh sách của chuông.
        property bool keepAlive

        function copy() {
            if (!n)
                return
            appName = n.appName
            // `notify-send -i X` KHÔNG gửi app_icon mà gửi hint image-path
            // (đo 2026-10-01): Quickshell đưa nó ra n.image dạng
            // "image://icon/X". X là tên icon → icon của thẻ; X là đường dẫn
            // file (vd ảnh vừa chụp) → ảnh xem trước. image-data (ảnh đại
            // diện trong app nhắn tin…) → icon.
            const iconPrefix = "image://icon/"
            const img = n.image
            const imgIsFile = img.startsWith(iconPrefix + "/")
            preview = imgIsFile ? "file://" + img.slice(iconPrefix.length) : ""
            icon = n.appIcon !== "" ? n.appIcon
                : img !== "" && !imgIsFile ? img : ""
            summary = root.plain(n.summary)
            body = root.plain(n.body)
            desktopEntry = n.desktopEntry
            critical = n.urgency === NotificationUrgency.Critical
        }
        Component.onCompleted: copy()

        readonly property Connections watch: Connections {
            target: entry.n
            function onSummaryChanged() { entry.copy() }
            function onBodyChanged() { entry.copy() }
            function onAppIconChanged() { entry.copy() }
            function onAppNameChanged() { entry.copy() }
            function onImageChanged() { entry.copy() }
            // App tự rút thông báo (vd tin nhắn đã đọc ở máy khác) → bỏ popup.
            function onClosed() { root.removePopup(entry) }
        }
    }
    Component {
        id: entryComponent
        Entry {}
    }

    // Bỏ thẻ HTML: không quảng cáo hỗ trợ markup (bodyMarkupSupported = false)
    // nhưng app nào cũng có thể gửi. Không render markup — <img>/<a> trong
    // thông báo là đường tải thứ lạ, và chữ thường là đủ.
    function plain(s) {
        return (s || "").replace(/<[^>]*>/g, "")
            .replace(/&lt;/g, "<").replace(/&gt;/g, ">")
            .replace(/&quot;/g, "\"").replace(/&apos;/g, "'").replace(/&amp;/g, "&")
    }

    // Mục không còn ở popup lẫn danh sách thì huỷ (một lần — đóng thông báo
    // còn gọi lại qua onClosed).
    function release(entry) {
        if (entry.released || popups.indexOf(entry) >= 0 || history.indexOf(entry) >= 0)
            return
        entry.released = true
        entry.destroy()
    }
    function removePopup(entry) {
        popups = popups.filter(e => e !== entry)
        release(entry)
    }
    function removeEntry(entry) {
        popups = popups.filter(e => e !== entry)
        history = history.filter(e => e !== entry)
        release(entry)
    }

    // Popup hết giờ: báo app "hết hạn" (trừ keepAlive), giữ bản chép trong
    // danh sách.
    function timeoutPopup(entry) {
        const n = entry.n
        const keep = entry.keepAlive
        removePopup(entry)
        if (n && !keep)
            n.expire()
    }

    // Người dùng bấm X: đóng hẳn — khỏi popup lẫn danh sách.
    function dismiss(entry) {
        const n = entry.n
        removeEntry(entry)
        if (n)
            n.dismiss()
    }

    // Bấm vào thân thông báo: chạy việc "default" của app nếu thông báo còn
    // sống; không thì mở app gửi nó (nếu biết app nào). Rồi bỏ khỏi danh sách.
    function activate(entry) {
        const n = entry.n
        const def = n ? n.actions.find(a => a.identifier === "default") : null
        const app = entry.desktopEntry === "" ? null
            : DesktopEntries.applications.values.find(e => e.id === entry.desktopEntry)
        if (def) {
            removeEntry(entry)
            def.invoke()
            return
        }
        dismiss(entry)
        if (app)
            app.execute()
    }

    function invokeAction(entry, action) {
        removeEntry(entry)
        action.invoke()
    }

    function clearAll() {
        const all = history.concat(popups.filter(e => history.indexOf(e) < 0))
        history = []
        popups = []
        for (const e of all) {
            const n = e.n
            root.release(e)
            if (n)
                n.dismiss()
        }
    }

    NotificationServer {
        id: server
        actionsSupported: true
        imageSupported: true
        bodyMarkupSupported: false
        bodyHyperlinksSupported: false
        bodyImagesSupported: false
        keepOnReload: false

        onNotification: n => {
            n.tracked = true

            // App gửi lại cùng id (cập nhật thông báo cũ): Quickshell sửa
            // thẳng đối tượng cũ, không gửi tín hiệu này lần nữa — nên ở đây
            // luôn là thông báo mới.
            const entry = entryComponent.createObject(root, { n: n })

            if (!n.transient) {
                const kept = [entry].concat(root.history)
                root.history = kept.slice(0, root.historyLimit)
                for (const old of kept.slice(root.historyLimit))
                    root.release(old)
            }

            const critical = entry.critical
            if ((root.doNotDisturb || root.centerOpen) && !critical) {
                root.release(entry)
                return
            }
            // expireTimeout tính bằng MILI GIÂY (y như D-Bus gửi — tài liệu
            // Quickshell ghi "giây" là sai: `-t 6000` ra 6000, đo 2026-10-02):
            // > 0 là app chọn, 0 = không tự tắt, < 0 = server chọn. Popup NÀO cũng tự ẩn (user 2026-10-02: khẩn
            // mà đứng mãi thì phiền); khẩn và "không tự tắt" ẩn sau 10 s và
            // vẫn sống trong danh sách (đặc tả: khẩn không tự hết hạn).
            const t = n.expireTimeout
            entry.keepAlive = critical || t === 0
            entry.timeout = entry.keepAlive ? root.longTimeout
                : t > 0 ? t : root.defaultTimeout
            root.popups = [entry].concat(root.popups)
        }
    }
}
