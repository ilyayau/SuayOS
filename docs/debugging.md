# SuayOS headless debugging

## Headless run (required)
Use the Makefile target:

- `make clean && make iso && make qemu-log`

`qemu-log` runs QEMU for ~6 seconds (via `timeout 6s`) with:
- COM1 serial mapped to the terminal (via a muxed stdio chardev)
- QEMU debugcon (I/O port `0xE9`) mapped to the same terminal
- QEMU internal event log written to `build/qemu.log` (`-d int,guest_errors -D build/qemu.log`)

## Output channels

### 1) Debugcon (port 0xE9)
The kernel writes bytes to I/O port `0xE9`. QEMU prints these bytes to the terminal when started with:

- `-device isa-debugcon,iobase=0xe9,chardev=char0`

This is extremely early and does not depend on IRQs.

### 2) COM1 serial (0x3F8)
The kernel also writes to COM1 at `0x3F8`. QEMU prints it to the terminal with:

- `-chardev stdio,id=char0,mux=on,signal=off`
- `-serial chardev:char0`

## Markers
During boot, the kernel prints deterministic stage markers (e.g. `BOOT0..BOOT5`, then `stage0..stage4`).

If the VM hangs or triple-faults, the last marker printed tells you which stage was last reached.
