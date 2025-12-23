#!/bin/bash
set -e
ISO=build/suayos.iso
CFG=boot/grub/grub.cfg
KERNEL=boot/kernel.elf

if ! [ -f "$ISO" ]; then
  echo "FAIL: $ISO not found"; exit 1
fi

# List files in ISO (requires bsdtar)
if ! command -v bsdtar >/dev/null; then
  echo "FAIL: bsdtar not installed. Install with: sudo pacman -S libarchive"
  exit 1
fi

echo "[INFO] Listing files in ISO:"
bsdtar -tf "$ISO" | grep -E "$CFG|$KERNEL" || { echo "FAIL: $CFG or $KERNEL missing in ISO"; exit 1; }

echo "[INFO] Extracting grub.cfg from ISO:"
bsdtar -xf "$ISO" -O "$CFG" > /tmp/grub.cfg.suayos
cat /tmp/grub.cfg.suayos

grep -q '^\s*multiboot2 /boot/kernel.elf' /tmp/grub.cfg.suayos || { echo "FAIL: grub.cfg does not use multiboot2 /boot/kernel.elf"; exit 1; }

echo "PASS: ISO contains correct kernel and grub.cfg with multiboot2 entry."
