// Số đo máy cho cụm CPU · nhiệt độ · RAM · Swap trên thanh trên (StatsButton)
// — MỘT tiến trình cho mọi màn hình (shell.qml giữ, các thanh đọc chung).
// Mỗi 2 s một dòng: "bận nghỉ memTotal memAvailable swapTotal swapFree nhiệt"
//   CPU   /proc/stat dòng `cpu`: bận = user+nice+system+irq+softirq+steal,
//         nghỉ = idle+iowait; % = phần bận trong hiệu hai lần đọc
//   RAM   (MemTotal − MemAvailable) / MemTotal — như "used" của free/htop
//   Swap  (SwapTotal − SwapFree) / SwapTotal (Fedora: zram); không có swap → -1
//   Nhiệt hwmon `coretemp` (Intel, temp1 = Package) / `k10temp` / `zenpower`
//         (AMD, temp1 = Tctl), không có thì thermal zone `x86_pkg_temp`;
//         milli-°C, không tìm được → -1 (thanh ẩn số nhiệt)
import QtQuick
import Quickshell.Io

Item {
    id: root

    property int cpu: -1
    property int memory: -1
    property int swap: -1
    property int temp: -1

    property real lastBusy: -1
    property real lastIdle: -1

    Process {
        running: true
        command: ["sh", "-c", `
t=
for h in /sys/class/hwmon/hwmon*; do
    case $(cat "$h/name" 2>/dev/null) in
    coretemp | k10temp | zenpower) t=$h/temp1_input; break ;;
    esac
done
if [ -z "$t" ]; then
    for z in /sys/class/thermal/thermal_zone*; do
        [ "$(cat "$z/type" 2>/dev/null)" = x86_pkg_temp ] && t=$z/temp && break
    done
fi
while :; do
    awk '/^cpu /{ printf "%.0f %.0f ", $2+$3+$4+$7+$8+$9, $5+$6; exit }' /proc/stat
    awk '/^(MemTotal|MemAvailable|SwapTotal|SwapFree):/{ printf "%s ", $2 }' /proc/meminfo
    { [ -n "$t" ] && cat "$t" 2>/dev/null; } || echo -1
    sleep 2
done`]
        stdout: SplitParser {
            onRead: line => {
                const n = line.trim().split(/\s+/).map(Number)
                if (n.length < 7 || n.some(isNaN))
                    return
                const [busy, idle, memTotal, memAvail, swapTotal, swapFree, milli] = n
                if (root.lastBusy >= 0) {
                    const d = (busy - root.lastBusy) + (idle - root.lastIdle)
                    if (d > 0)
                        root.cpu = Math.round(100 * (busy - root.lastBusy) / d)
                }
                root.lastBusy = busy
                root.lastIdle = idle
                root.memory = memTotal > 0 ? Math.round(100 * (memTotal - memAvail) / memTotal) : -1
                root.swap = swapTotal > 0 ? Math.round(100 * (swapTotal - swapFree) / swapTotal) : -1
                root.temp = milli > 0 ? Math.round(milli / 1000) : -1
            }
        }
    }
}
