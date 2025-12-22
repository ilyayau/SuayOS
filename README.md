# SuayOS

Minimal skeleton for a freestanding x86_64 kernel and bootable ISO.

## Prerequisites
- GNU Make
- x86_64-elf binutils and gcc (cross toolchain) available on PATH
- `grub-mkrescue` (from `grub-pc-bin` on Debian/Ubuntu or equivalent) and `xorriso`
- `qemu-system-x86_64` for running the ISO

## Build and run
```sh
# Build the freestanding kernel binary
make kernel

# Package a bootable ISO under build/suayos.iso
make iso

# Build everything
make all

# Run the ISO in QEMU
make run

# Clean build artifacts
make clean
```

## Layout
- `kernel/` — architecture code, headers, sources
- `iso_root/` — GRUB boot tree used to assemble the ISO
- `scripts/` — helper scripts (reserved)
