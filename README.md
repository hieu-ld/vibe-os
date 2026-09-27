# Samhie

Distro Linux cá nhân dựng trên **Fedora Atomic**, desktop **Hyprland**, giao
diện tự vẽ bằng **Quickshell**. Làm để dùng hằng ngày cho việc code và làm việc,
chia sẻ cho bạn bè dùng cùng.

> ⚠️ Đang trong giai đoạn dựng khung — chưa dùng được hằng ngày. Tiến độ:
> [docs/PLAN.md](docs/PLAN.md).

## Vì sao an toàn

- **Hệ thống gốc chỉ đọc.** Phần mềm độc hại không sửa được file hệ thống.
- **SELinux** bật ở chế độ enforcing, **tường lửa** chặn mọi kết nối từ ngoài vào.
- **Tự cập nhật mỗi ngày.** Image được build lại hằng ngày bằng bản vá mới nhất
  của Fedora; máy tự tải về và áp dụng ở lần khởi động sau.
- **Cập nhật lỗi thì quay lại được** — chọn bản cũ ngay trong menu khởi động.
- **Image được ký số.** Máy kiểm chữ ký trước khi nhận bản cập nhật.

## Chọn bản nào

| Máy của bạn | Image |
|---|---|
| Card AMD hoặc Intel | `ghcr.io/hieu-ld/samhie` |
| Card NVIDIA (GTX 16xx / RTX 20xx trở lên) | `ghcr.io/hieu-ld/samhie-nvidia` |

## Cài đặt

1. Cài một bản **Fedora Atomic** bất kỳ (ví dụ [Fedora Kinoite](https://fedoraproject.org/atomic-desktops/kinoite/))
   — **nhớ tích "Encrypt my data"** để mã hoá ổ đĩa.
2. Mở terminal, chuyển sang Samhie (thay `samhie` bằng `samhie-nvidia` nếu máy có card NVIDIA):

   ```bash
   # Lần đầu: tải bản chưa kiểm chữ ký để nhận khoá và chính sách kiểm chữ ký của Samhie
   rpm-ostree rebase ostree-unverified-registry:ghcr.io/hieu-ld/samhie:latest
   systemctl reboot

   # Sau khi khởi động lại: chuyển sang bản CÓ kiểm chữ ký — từ giờ mọi cập nhật đều được kiểm
   rpm-ostree rebase ostree-image-signed:docker://ghcr.io/hieu-ld/samhie:latest
   systemctl reboot
   ```

3. **Chỉ máy NVIDIA, nếu bật Secure Boot:** nạp khoá của driver một lần, rồi khởi
   động lại và làm theo màn hình xanh (MOK) hiện ra:

   ```bash
   ujust enroll-secure-boot-key
   ```

## Quay lại bản trước

Bản cập nhật mới bị lỗi? Khởi động lại và chọn mục thứ hai trong menu khởi
động, hoặc:

```bash
rpm-ostree rollback
systemctl reboot
```

## Phím tắt chính

| Phím | Việc |
|---|---|
| `Win + E` | Mở quản lý file (Dolphin) |
| `Win + R` hoặc `Win + Space` | Mở app |
| `Win + T` hoặc `Ctrl + Alt + T` | Mở terminal |
| `Win + L` | Khoá máy |
| `Win + V` | Lịch sử clipboard |
| `Win + Shift + S` | Chụp một vùng màn hình |
| `Alt + F4` hoặc `Win + Q` | Đóng cửa sổ |
| `Win + 1…0` | Chuyển workspace |

## Tuỳ chỉnh cho riêng mình

Sửa `~/.config/hypr/hyprland.lua`. File đó nạp cấu hình mặc định của Samhie
trước, nên bạn chỉ cần viết lại đúng thứ muốn đổi — bản cập nhật của Samhie
không bao giờ ghi đè lên phần của bạn.

## Kiểm chữ ký image

```bash
cosign verify --key cosign.pub ghcr.io/hieu-ld/samhie
```
