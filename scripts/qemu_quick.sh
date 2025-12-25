#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

exec timeout 8s qemu-system-x86_64 \
  -cdrom build/suayos.iso \
  -no-reboot -no-shutdown \
  -chardev stdio,id=char0,mux=on,signal=off \
  -serial chardev:char0 \
  -device isa-debugcon,iobase=0xe9,chardev=char0 \
  -display none \
  -d int,guest_errors \
  -D build/qemu.log \
  </dev/null || true
