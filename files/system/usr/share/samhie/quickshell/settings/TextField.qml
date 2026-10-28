// Ô nhập chữ trong dòng của Group — cùng hình với ô "Set date and time" của
// trang Date & Time (nền lớp màu chữ 8%, bo 6). Chữ gợi ý ẩn theo
// `displayText` (có cả chữ Unikey đang gõ dở — PLAN mục 12).
//
//   TextField { id: f; width: 240; placeholder: "Name"; onAccepted: … }
//   TextField { password: true }    // chấm tròn, không gợi ý của bộ gõ
import QtQuick
import ".."
import "../colors.js" as Colors

Rectangle {
    id: root

    property alias text: edit.text
    property alias input: edit
    property string placeholder: ""
    property bool password: false
    signal accepted()

    width: 240
    height: 32
    radius: 6
    color: Qt.alpha(Theme.c.text, 0.08)

    // id KHÔNG được là `input`: trong Caret { input: … } chữ `input` là
    // thuộc tính của chính Caret → tự trỏ vào nó (null), dấu nháy lệch + không ẩn.
    TextInput {
        id: edit
        x: 12
        width: parent.width - 24
        height: parent.height
        verticalAlignment: TextInput.AlignVCenter
        clip: true
        echoMode: root.password ? TextInput.Password : TextInput.Normal
        passwordCharacter: "•"
        inputMethodHints: root.password ? Qt.ImhHiddenText | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText : Qt.ImhNone
        color: Theme.c.text
        selectionColor: Theme.c.selection
        selectedTextColor: Theme.c.onSelection
        font.family: Colors.fonts.ui
        font.weight: Colors.fonts.uiWeight
        font.pixelSize: 24
        cursorDelegate: Caret { input: edit; color: Theme.c.text; size: 16 }
        onAccepted: root.accepted()

        Text {
            anchors.verticalCenter: parent.verticalCenter
            visible: edit.displayText === ""
            text: root.placeholder
            color: Theme.c.textMuted
            font: edit.font
        }
    }
}
