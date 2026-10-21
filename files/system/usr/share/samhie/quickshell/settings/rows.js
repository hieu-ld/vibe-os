// Dòng trong Group có vạch ngăn phía trên, trừ dòng HIỆN đầu tiên của nhóm —
// nhóm có thể chứa Repeater (cao 0) và dòng ẩn ("No output devices" khi đã
// có thiết bị), nên không lấy children[0].
.pragma library

function isFirst(item) {
    if (!item.parent)
        return false
    for (const c of item.parent.children)
        if (c.visible && c.height > 0)
            return c === item
    return false
}
