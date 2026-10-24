# Samhie

A personal Linux build on **Fedora Atomic**, **Hyprland** and **Quickshell**.

<!-- Screenshots: dark + light desktop, overview, Settings -->

## What's inside

- **Apps:** Dolphin, Konsole, Kate, Ark, Microsoft Edge, Gwenview (images), Okular (PDF), and more.
- **Vietnamese input:** fcitx5 + Unikey (Telex, VNI).
- **System stats in the top bar:** CPU, temperature, RAM, swap; plus your Claude 5-hour / weekly limits (when you use Claude Code).
- **Windows-style split view:** `Win + ←/→`.

## Security

- **Read-only base system.** Malware can't modify system files.
- **SELinux** is on, and the **firewall** blocks every incoming connection.
- **Daily automatic updates.** The image is rebuilt every day with Fedora's latest fixes; your computer downloads it at 4 AM and applies it on the next restart.
- **Roll back a bad update** — one button in Settings → Updates, or pick the previous version in the boot menu.
- **Signed images.** Your computer checks the signature before accepting an update.
- **Packages come mainly from Fedora's official repositories;** everyday apps run sandboxed as Flatpaks. The lock screen, password keyring and admin prompt are all the official versions.

## Which image

| Your computer | Image |
|---|---|
| AMD or Intel graphics | `ghcr.io/hieu-ld/samhie` |
| NVIDIA graphics (GTX 16xx / RTX 20xx or newer) | `ghcr.io/hieu-ld/samhie-nvidia` |

## Install

1. Install any **Fedora Atomic** edition (for example [Fedora Kinoite](https://fedoraproject.org/atomic-desktops/kinoite/)) — **remember to tick "Encrypt my data"** to encrypt your disk.
2. Open a terminal and switch to Samhie (use `samhie-nvidia` instead of `samhie` on NVIDIA computers):

   ```bash
   # First time: an unverified pull to get Samhie's signing key and verification policy
   rpm-ostree rebase ostree-unverified-registry:ghcr.io/hieu-ld/samhie:latest
   systemctl reboot

   # After the restart: switch to the SIGNED image — every update is verified from now on
   rpm-ostree rebase ostree-image-signed:docker://ghcr.io/hieu-ld/samhie:latest
   systemctl reboot
   ```

3. **NVIDIA only, with Secure Boot on:** enroll the driver key once, then restart and follow the blue (MOK) screen:

   ```bash
   ujust enroll-secure-boot-key
   ```

## Updating and rolling back

Samhie updates itself — nothing to do. To check right away: **Settings → Updates → Check Now**; when an update is ready, click **Restart to Update**.

A new version broke something? **Settings → Updates → Go Back…**, pick the second entry in the boot menu, or run:

```bash
samhie rollback
```

Short terminal commands (`samhie list` shows them):

| Command | What it does |
|---|---|
| `samhie update` | Update Samhie and your apps (applies on the next restart) |
| `samhie status` | Show the running version and any update waiting for a restart |
| `samhie rollback` | Go back to the previous version and restart (asks first) |

The usual commands (`rpm-ostree`, `flatpak`, `ujust`) still work as before.

## Keyboard shortcuts

| Keys | Action |
|---|---|
| `Win` | Overview — start typing to search apps |
| `Win + E` | Files (Dolphin) |
| `Win + T` or `Ctrl + Alt + T` | Terminal |
| `Win + V` | Clipboard history |
| `Win + L` | Lock |
| `Print` or `Win + Shift + S` / `Shift + Print` | Screenshot of a region / the whole screen |
| `Win + ←` / `Win + →` | Snap window to the left / right half |
| `Win + ↑` / `Win + ↓` | Maximize / restore |
| `Alt + F4` or `Win + Q` | Close window |
| `Win + 1…0` | Switch workspace (add `Shift` to take the window along) |

## Make it yours

Edit `~/.config/hypr/hyprland.lua`. It loads Samhie's defaults first, so you only write what you want to change — Samhie updates never overwrite your part. Changes made in the Settings app also live in `~/.config` and never touch the system.

## Verify the image signature

```bash
cosign verify --key cosign.pub ghcr.io/hieu-ld/samhie
```

## License

[Apache-2.0](LICENSE).

Icons are based on the free edition of [Hugeicons](https://hugeicons.com) (MIT); the [Dongle](https://fonts.google.com/specimen/Dongle) font is under the SIL OFL — each comes with its license in its own folder.
