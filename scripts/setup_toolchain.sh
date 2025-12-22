#!/bin/sh
set -e
missing=0
check() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Missing: $1" >&2
        missing=1
    fi
}

check make
check grub-mkrescue
check qemu-system-x86_64
if command -v clang >/dev/null 2>&1; then
    :
else
    check gcc
fi

if [ $missing -ne 0 ]; then
    echo "\nPlease install the missing tools. On Ubuntu/Debian, try:"
    echo "  sudo apt-get install build-essential grub-pc-bin xorriso qemu-system-x86"
    exit 1
fi

echo "Toolchain OK."
