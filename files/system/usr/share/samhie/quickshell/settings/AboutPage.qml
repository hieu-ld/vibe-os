// Trang About — như "About This Mac": logo + tên ở giữa, dưới là nhóm thông
// tin máy. Đọc một lần lúc mở trang (một lệnh sh, không cần quyền root):
//   tên máy   /proc/sys/kernel/hostname
//   CPU       "model name" trong /proc/cpuinfo (bỏ (R)/(TM)/"CPU @ …")
//   GPU       lspci -mm, dòng VGA / 3D / Display controller
//   RAM       MemTotal trong /proc/meminfo, làm tròn LÊN GB (MemTotal nhỏ
//             hơn RAM thật một chút — máy 16 GB báo ~15,4)
//   hệ        VERSION_ID trong /etc/os-release, nhân `uname -r`
import QtQuick
import Quickshell.Io
import ".."
import "../colors.js" as Colors

Column {
    id: root

    property string title
    property var info: ({})

    width: parent ? parent.width : 0
    spacing: 24

    Process {
        running: true
        command: ["sh", "-c",
            "echo host=$(cat /proc/sys/kernel/hostname);"
            + "echo cpu=$(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2-);"
            + "echo mem=$(awk '/MemTotal/ {print $2}' /proc/meminfo);"
            + ". /etc/os-release; echo os=$VERSION_ID;"
            + "echo kernel=$(uname -r);"
            + "lspci -mm 2>/dev/null | grep -E '\"(VGA compatible controller|3D controller|Display controller)\"' | sed 's/^/gpu=/'"]
        stdout: StdioCollector {
            onStreamFinished: {
                const out = { gpus: [] }
                for (const line of text.trim().split("\n")) {
                    const i = line.indexOf("=")
                    const key = line.slice(0, i), val = line.slice(i + 1).trim()
                    if (key === "gpu")
                        out.gpus.push(root.gpuName(val))
                    else
                        out[key] = val
                }
                root.info = out
            }
        }
    }

    // `00:02.0 "VGA compatible controller" "Intel Corporation" "AlderLake-S GT1"`
    // → "Intel AlderLake-S GT1"; tên trong [ ] (NVIDIA: "GA106 [GeForce RTX
    // 3060 Lite Hash Rate]") được ưu tiên.
    function gpuName(line) {
        const f = []
        const re = /"([^"]*)"/g
        for (let m = re.exec(line); m; m = re.exec(line))
            f.push(m[1])
        const vendor = (f[1] ?? "").replace(/ Corporation| Inc\.|, Inc\.|\[AMD\/ATI\]/g, "").trim()
        const bracket = (f[2] ?? "").match(/\[(.*)\]/)
        return (vendor + " " + (bracket ? bracket[1] : (f[2] ?? ""))).trim()
    }

    function cpuName(s) {
        return (s ?? "").replace(/\((R|TM)\)/g, "").replace(/ CPU @.*$/, "").replace(/\s+/g, " ").trim()
    }

    // Logo + tên.
    Column {
        width: parent.width
        spacing: 4
        topPadding: 8

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "file:///usr/share/icons/hicolor/scalable/apps/samhie.svg"
            sourceSize: Qt.size(96, 96)
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Samhie"
            color: Theme.c.text
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 40
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.info.os ? "Based on Fedora " + root.info.os : ""
            color: Theme.c.textMuted
            font.family: Colors.fonts.ui
            font.weight: Colors.fonts.uiWeight
            font.pixelSize: 24
        }
    }

    Group {
        InfoRow { label: "Name"; value: root.info.host ?? "" }
        InfoRow { label: "Processor"; value: root.cpuName(root.info.cpu) }
        // Mỗi card một dòng (máy Samhie: Intel bo mạch + NVIDIA).
        Repeater {
            model: root.info.gpus ?? []
            InfoRow {
                required property string modelData
                required property int index
                label: index === 0 ? "Graphics" : ""
                value: modelData
            }
        }
        InfoRow {
            label: "Memory"
            value: root.info.mem ? Math.ceil(root.info.mem / 1024 / 1024) + " GB" : ""
        }
    }

    Group {
        title: "System"
        InfoRow { label: "Fedora"; value: root.info.os ?? "" }
        InfoRow { label: "Kernel"; value: root.info.kernel ?? "" }
    }
}
