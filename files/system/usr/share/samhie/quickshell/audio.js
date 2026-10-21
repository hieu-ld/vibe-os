// Dùng chung cho trang Sound Output của quick settings và trang Sound của app
// Settings: nhận loại thiết bị âm thanh (Pipewire) → icon trong bộ Samhie.
.pragma library

// Ngõ ra: HDMI/DisplayPort = màn hình; tai nghe (device.form-factor, hoặc
// thiết bị Bluetooth bluez_output) = tai nghe; còn lại = loa. `properties` chỉ
// có khi node đang được theo dõi (PwObjectTracker) — chưa có thì đoán theo tên.
function outputIcon(node) {
    const p = node.properties ?? {}
    const text = (node.name + " " + (node.description ?? "")).toLowerCase()
    const form = (p["device.form-factor"] ?? "").toLowerCase()
    if (/hdmi|displayport/.test(text))
        return "video-display"
    if (/headphone|headset|hands-free/.test(form + " " + text) || node.name.startsWith("bluez_output"))
        return "audio-headphones"
    return "audio-speakers"
}

// Tên hiển thị của thiết bị (loa, micro).
function deviceName(node) {
    return node.description || node.nickname || node.name
}

