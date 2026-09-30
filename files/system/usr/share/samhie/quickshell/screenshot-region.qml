// Chọn vùng chụp màn hình kiểu Ubuntu (Win+Shift+S) — /usr/libexec/samhie/
// screenshot gọi file này bằng `quickshell -p`, truyền qua biến môi trường:
//   SAMHIE_SHOT_FROZEN  ảnh grim chụp mọi màn hình NGAY lúc bấm phím
//   SAMHIE_SHOT_FILE    chỗ lưu ảnh vùng chọn
//
// - Màn hình ĐỨNG HÌNH: mỗi màn hình hiện phần của nó trong ảnh FROZEN.
//   Không dùng ScreencopyView của Quickshell: nó chỉ chụp khi compositor có
//   dmabuf, không có thì đứng chờ mãi mà không báo lỗi (đo 2026-09-29 với sway
//   không GPU). grim thì chạy ở mọi nơi, và chụp trước khi Quickshell kịp mở.
// - Kéo để chọn; thả tay KHÔNG chụp liền mà hiện khung trắng có tay nắm: kéo
//   góc/cạnh để chỉnh cỡ, kéo bên trong để dời, kéo ra ngoài để chọn lại.
//   Enter = chụp, Esc = huỷ.
// - Chụp: CẮT thẳng từ ảnh FROZEN (Image.sourceClipRect + grabToImage), ở độ
//   phân giải gốc của ảnh. Không chụp lại màn hình lúc Enter: bản đầu làm vậy
//   (grim vùng chọn, cách của hyprshot) thì con trỏ chuột — đang nằm ngay
//   trên vùng chọn — lọt vào ảnh (máy ảo 2026-09-29).
// - Chỉ một vùng, nằm gọn trong một màn hình.
import QtQuick
import Quickshell
import Quickshell.Wayland
import "colors.js" as Colors

ShellRoot {
    id: root

    readonly property string outFile: Quickshell.env("SAMHIE_SHOT_FILE") ?? ""
    readonly property string frozenFile: Quickshell.env("SAMHIE_SHOT_FROZEN") ?? ""

    // grim ghép mọi màn hình thành MỘT ảnh theo bố cục của chúng (toạ độ
    // logic, lấy từ góc trên-trái của khung bao) — như screenshot-preview.qml.
    readonly property var screens: Array.from(Quickshell.screens)
    readonly property int minX: Math.min(...screens.map(s => s.x))
    readonly property int minY: Math.min(...screens.map(s => s.y))
    readonly property int layoutWidth: Math.max(...screens.map(s => s.x + s.width)) - minX
    readonly property int layoutHeight: Math.max(...screens.map(s => s.y + s.height)) - minY

    Component.onCompleted: if (outFile === "" || frozenFile === "") Qt.quit()

    // Vùng chọn, toạ độ logic trong màn hình selScreen. selScreen null = chưa chọn.
    property var selScreen: null
    property rect sel: Qt.rect(0, 0, 0, 0)
    readonly property bool hasSelection: selScreen !== null && sel.width >= 1 && sel.height >= 1

    readonly property int frameWidth: 2     // viền trắng quanh vùng chọn
    readonly property int handleThick: 5    // tay nắm: vạch dày hơn viền
    readonly property int handleLength: 22
    readonly property int grab: 14          // bán kính bắt chuột quanh cạnh/góc

    // Enter: cửa sổ của màn hình đang có vùng chọn tự cắt ảnh rồi thoát.
    // Script kiểm file ảnh có được tạo không.
    signal captureRequested()
    property bool saving: false
    function capture() {
        if (!hasSelection || saving) return;
        saving = true;
        captureRequested();
    }

    Variants {
        model: root.screens

        PanelWindow {
            id: win
            required property ShellScreen modelData
            readonly property bool owns: root.selScreen === modelData

            screen: modelData
            // Hiện khi ảnh đông nạp xong, rồi GIỮ hiện. Đừng gắn thẳng visible với
            // status: ở scale lẻ, cửa sổ hiện ra thì dpr đổi 1 → 1,5, Qt nạp lại
            // ảnh (status về Loading) → cửa sổ ẩn → dpr về 1 → nạp xong → hiện…
            // lặp mãi, không bao giờ thấy (đo 2026-09-30, sway scale 1.5).
            property bool shown: false
            visible: shown
            color: "transparent"
            anchors { top: true; bottom: true; left: true; right: true }
            exclusionMode: ExclusionMode.Ignore
            WlrLayershell.layer: WlrLayer.Overlay
            // samhie.lua tắt hiệu ứng mở/đóng cho namespace này: khung đông
            // mà trượt vào thì lộ ngay.
            WlrLayershell.namespace: "samhie-screenshot"
            WlrLayershell.keyboardFocus: WlrKeyboardFocus.Exclusive

            // Tỉ lệ 1:1 khi mọi màn hình cùng scale: đúng từng pixel.
            Image {
                id: frozen
                source: root.frozenFile === "" ? ""
                    : "file://" + root.frozenFile.split("/").map(encodeURIComponent).join("/")
                x: root.minX - win.modelData.x
                y: root.minY - win.modelData.y
                width: root.layoutWidth
                height: root.layoutHeight
                fillMode: Image.Stretch
                asynchronous: true
                cache: false
                onStatusChanged: if (status === Image.Ready) win.shown = true
            }

            // Ô cắt ảnh: chỉ nạp khi Enter, nằm DƯỚI ảnh đông (z) nên không
            // hiện ra. sourceClipRect tính bằng pixel của ảnh FROZEN: grim ghép
            // ảnh theo scale lớn nhất, nên đổi từ toạ độ logic qua tỉ lệ px.
            // Cỡ Item = cỡ pixel / devicePixelRatio: vẽ ra đúng 1:1, không nội suy.
            // grabToImage KHÔNG truyền cỡ đích: Qt 6 tự nhân cỡ đích với dpr,
            // truyền cỡ pixel vào là ảnh phóng to gấp đôi ở scale 2 (đo 2026-09-30).
            Image {
                id: cropper
                z: -1
                readonly property real px: frozen.sourceSize.width / root.layoutWidth
                readonly property rect clip: Qt.rect(
                    Math.round((win.modelData.x - root.minX + root.sel.x) * px),
                    Math.round((win.modelData.y - root.minY + root.sel.y) * px),
                    Math.round(root.sel.width * px),
                    Math.round(root.sel.height * px))
                source: root.saving && win.owns ? frozen.source : ""
                sourceClipRect: clip
                width: clip.width / win.devicePixelRatio
                height: clip.height / win.devicePixelRatio
                smooth: false
                cache: false

                onStatusChanged: {
                    if (status === Image.Ready) {
                        grabToImage(result => {
                            if (!result.saveToFile(root.outFile))
                                console.warn("Không lưu được", root.outFile);
                            Qt.quit();
                        });
                    } else if (status === Image.Error) {
                        Qt.quit();
                    }
                }
            }

            // Lớp tối: cả màn hình khi chưa chọn, bốn dải quanh vùng chọn khi đã chọn.
            Item {
                id: shade
                anchors.fill: parent
                readonly property rect r: win.owns ? root.sel : Qt.rect(0, 0, 0, 0)
                readonly property bool hole: win.owns && root.hasSelection

                component Band: Rectangle {
                    color: Colors.shared.captureShade
                    opacity: 0.5
                }
                Band { x: 0; y: 0; width: parent.width; height: shade.hole ? shade.r.y : parent.height }
                Band {
                    visible: shade.hole
                    x: 0; y: shade.r.y + shade.r.height
                    width: parent.width; height: parent.height - y
                }
                Band {
                    visible: shade.hole
                    x: 0; y: shade.r.y
                    width: shade.r.x; height: shade.r.height
                }
                Band {
                    visible: shade.hole
                    x: shade.r.x + shade.r.width; y: shade.r.y
                    width: parent.width - x; height: shade.r.height
                }
            }

            // Khung trắng + tay nắm, nằm trọn NGOÀI vùng chọn.
            Item {
                id: frame
                visible: win.owns && root.hasSelection
                x: root.sel.x; y: root.sel.y
                width: root.sel.width; height: root.sel.height

                Rectangle {
                    x: -root.frameWidth; y: -root.frameWidth
                    width: parent.width + 2 * root.frameWidth
                    height: parent.height + 2 * root.frameWidth
                    color: "transparent"
                    border.width: root.frameWidth
                    border.color: Colors.shared.captureFrame
                }

                // Tay nắm góc: chữ L ôm góc từ phía ngoài. Gốc của Item là ô
                // vuông t×t chéo ngoài góc; hai vạch chạy dọc theo hai cạnh.
                // (Tên onRight/onBottom: `right`/`bottom` trùng thuộc tính
                // FINAL của Item, Qt từ chối nạp file — đo trong máy ảo.)
                Repeater {
                    model: [[false, false], [true, false], [false, true], [true, true]]

                    Item {
                        id: corner
                        required property var modelData
                        readonly property bool onRight: modelData[0]
                        readonly property bool onBottom: modelData[1]
                        readonly property int t: root.handleThick
                        readonly property int len: root.handleLength
                        x: onRight ? frame.width : -t
                        y: onBottom ? frame.height : -t
                        Rectangle {
                            color: Colors.shared.captureFrame
                            x: corner.onRight ? corner.t - corner.len : 0
                            width: corner.len; height: corner.t
                        }
                        Rectangle {
                            color: Colors.shared.captureFrame
                            y: corner.onBottom ? corner.t - corner.len : 0
                            width: corner.t; height: corner.len
                        }
                    }
                }

                // Tay nắm giữa cạnh: vạch ngắn ngay ngoài cạnh.
                component EdgeBar: Rectangle { color: Colors.shared.captureFrame }
                readonly property int barLen: Math.min(root.handleLength, Math.max(0, width - 2 * root.handleLength))
                readonly property int barLenV: Math.min(root.handleLength, Math.max(0, height - 2 * root.handleLength))
                EdgeBar { x: (frame.width - width) / 2; y: -root.handleThick; width: frame.barLen; height: root.handleThick }
                EdgeBar { x: (frame.width - width) / 2; y: frame.height; width: frame.barLen; height: root.handleThick }
                EdgeBar { x: -root.handleThick; y: (frame.height - height) / 2; width: root.handleThick; height: frame.barLenV }
                EdgeBar { x: frame.width; y: (frame.height - height) / 2; width: root.handleThick; height: frame.barLenV }
            }

            MouseArea {
                id: mouse
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                enabled: !root.saving
                cursorShape: cursorFor(pressed ? mode : hitTest(mouseX, mouseY))

                property string mode: ""
                property real startX
                property real startY
                property rect startSel

                // Chuột đang chỉ vào đâu: "" (ngoài, chọn mới), "move", hoặc
                // cạnh/góc theo hướng la bàn: "n", "se", "w"…
                function hitTest(x, y) {
                    if (!win.owns || !root.hasSelection) return "";
                    const s = root.sel, g = root.grab;
                    const l = s.x, r = s.x + s.width, t = s.y, b = s.y + s.height;
                    if (x < l - g || x > r + g || y < t - g || y > b + g) return "";
                    // Vùng chọn nhỏ thì hai cạnh gần nhau: lấy cạnh gần chuột hơn.
                    const dl = Math.abs(x - l), dr = Math.abs(x - r);
                    const dt = Math.abs(y - t), db = Math.abs(y - b);
                    let h = "", v = "";
                    if (Math.min(dl, dr) <= g) h = dl < dr ? "w" : "e";
                    if (Math.min(dt, db) <= g) v = dt < db ? "n" : "s";
                    if (h || v) return v + h;
                    return "move";
                }

                function cursorFor(m) {
                    switch (m) {
                    case "move": return Qt.SizeAllCursor;
                    case "n": case "s": return Qt.SizeVerCursor;
                    case "e": case "w": return Qt.SizeHorCursor;
                    case "nw": case "se": return Qt.SizeFDiagCursor;
                    case "ne": case "sw": return Qt.SizeBDiagCursor;
                    default: return Qt.CrossCursor;
                    }
                }

                function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)); }

                onPressed: event => {
                    mode = hitTest(event.x, event.y);
                    startX = clamp(Math.round(event.x), 0, win.width);
                    startY = clamp(Math.round(event.y), 0, win.height);
                    if (mode === "") {
                        mode = "new";
                        root.selScreen = win.modelData;
                        root.sel = Qt.rect(startX, startY, 0, 0);
                    }
                    startSel = root.sel;
                }

                onPositionChanged: event => {
                    if (!pressed) return;
                    const x = clamp(Math.round(event.x), 0, win.width);
                    const y = clamp(Math.round(event.y), 0, win.height);
                    const s = startSel;

                    if (mode === "new") {
                        root.sel = Qt.rect(Math.min(startX, x), Math.min(startY, y),
                                           Math.abs(x - startX), Math.abs(y - startY));
                    } else if (mode === "move") {
                        root.sel = Qt.rect(clamp(s.x + x - startX, 0, win.width - s.width),
                                           clamp(s.y + y - startY, 0, win.height - s.height),
                                           s.width, s.height);
                    } else {
                        // Kéo cạnh/góc: dời đúng các cạnh có trong mode. Kéo lố qua
                        // cạnh đối diện thì vùng lật lại, như Ubuntu.
                        let l = s.x, r = s.x + s.width, t = s.y, b = s.y + s.height;
                        const dx = x - startX, dy = y - startY;
                        if (mode.includes("w")) l = clamp(l + dx, 0, win.width);
                        if (mode.includes("e")) r = clamp(r + dx, 0, win.width);
                        if (mode.includes("n")) t = clamp(t + dy, 0, win.height);
                        if (mode.includes("s")) b = clamp(b + dy, 0, win.height);
                        root.sel = Qt.rect(Math.min(l, r), Math.min(t, b),
                                           Math.abs(r - l), Math.abs(b - t));
                    }
                }

                onReleased: {
                    // Bấm mà không kéo: bỏ vùng chọn, chờ kéo lại.
                    if (mode === "new" && !root.hasSelection) root.selScreen = null;
                    mode = "";
                }
            }

            // Phím: cửa sổ nào đang giữ bàn phím cũng xử lý như nhau.
            Item {
                anchors.fill: parent
                focus: true
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Escape) {
                        Qt.quit();
                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        root.capture();
                    }
                    event.accepted = true;
                }
            }
        }
    }
}
