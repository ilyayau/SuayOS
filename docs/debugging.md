# SuayOS headless debugging

## Headless run (required)
Use the Makefile target:

- `make clean && make iso && make qemu-log`

`qemu-log` runs QEMU for ~6 seconds (via `timeout 6s`) with:
- COM1 serial mapped to the terminal (`-serial stdio`)
- QEMU debugcon mapped to the terminal (`-debugcon stdio -global isa-debugcon.iobase=0xe9`)
- QEMU internal event log written to `build/qemu.log` (`-d int,guest_errors -D build/qemu.log`)

## Output channels

### 1) Debugcon (port 0xE9)
The kernel writes bytes to I/O port `0xE9`. QEMU prints these bytes to the terminal when started with:

- `-debugcon stdio -global isa-debugcon.iobase=0xe9`

This is extremely early and does not depend on IRQs.

### 2) COM1 serial (0x3F8)
The kernel also writes to COM1 at `0x3F8`. QEMU prints it to the terminal with:

- `-serial stdio`

## Markers and heartbeat
During boot, `kmain` prints bracketed markers around risky init stages, e.g.:
- `[A] before gdt_init` / `[B] after gdt_init`
- `[C] before idt_init` / `[D] after idt_init`

After printing `SuayOS booted`, the kernel enters a non-interrupt busy-wait heartbeat and prints:

- `HB....` continuously

If the VM hangs or triple-faults, the last marker printed tells you which stage was last reached.
