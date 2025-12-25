#!/usr/bin/env bash
set -euo pipefail

ISO_PATH="${ISO_PATH:-build/suayos.iso}"
LOG_PATH="${LOG_PATH:-build/qemu.log}"
QEMU_TIMEOUT="${QEMU_TIMEOUT:-15s}"

mkdir -p "$(dirname "$LOG_PATH")"

exec timeout "$QEMU_TIMEOUT" qemu-system-x86_64 \
  -cdrom "$ISO_PATH" \
  -no-reboot -no-shutdown \
  -chardev stdio,id=char0,mux=on,signal=off \
  -serial chardev:char0 \
  -device isa-debugcon,iobase=0xe9,chardev=char0 \
  -display none \
  -d int,guest_errors \
  -D "$LOG_PATH" \
  </dev/null || true
