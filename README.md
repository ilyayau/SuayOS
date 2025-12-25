# SuayOS

Minimal skeleton for a freestanding x86_64 kernel and bootable ISO.


## Prerequisites (Linux)
- GNU Make
- `grub-mkrescue` (from `grub-pc-bin` on Debian/Ubuntu or equivalent) and `xorriso`
- `qemu-system-x86_64`
- `clang` (preferred) or `gcc`

Check your toolchain:
```sh
./scripts/setup_toolchain.sh
```

Install missing tools (Ubuntu/Debian):
```sh
sudo apt-get install build-essential grub-pc-bin xorriso qemu-system-x86
```


## Build and run
```sh
# Full clean build
./scripts/build.sh

# Fast sanity build
make check

# Run the ISO in QEMU
make run
```

## Crash debugging (serial-first)

Run QEMU with deterministic serial output + a log file:
```sh
make qemu-log
```

Expected early markers (in order):
- `BOOT0` .. `BOOT5` (from early boot)
- `KMAIN` (from kernel entry)

When the kernel prints a faulting `RIP` (from an exception or `panic()`), symbolize it:
```sh
./scripts/symbolize.sh 0xDEADBEEF
```

Artifacts:
- `build/kernel.elf` (symbols/debug info for `addr2line`)
- `build/kernel.map` (linker map file)

For more detailed symbolization (line numbers), use:
```sh
make debug-iso
```

## Layout
- `kernel/` — architecture code, headers, sources
- `iso_root/` — GRUB boot tree used to assemble the ISO
- `scripts/` — helper scripts (reserved)
