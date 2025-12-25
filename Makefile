verify-iso: iso
	bash scripts/verify_iso.sh

qemu-quick: iso
	bash scripts/qemu_quick.sh

qemu-debug: iso
	bash scripts/qemu_debug.sh

qemu-triage:
	bash scripts/triage_log.sh

qemu-log: iso
	@mkdir -p $(BUILD_DIR)
	@echo "[qemu] headless run (6s), log: $(BUILD_DIR)/qemu.log"
	@timeout 6s qemu-system-x86_64 \
			-cdrom $(ISO_PATH) \
			-no-reboot -no-shutdown \
			-display none \
			-monitor none \
			-chardev stdio,id=char0,mux=on,signal=off \
			-serial chardev:char0 \
			-device isa-debugcon,iobase=0xe9,chardev=char0 \
			-d int,guest_errors \
			-D $(BUILD_DIR)/qemu.log \
			</dev/null || true
	@echo "[qemu] tail $(BUILD_DIR)/qemu.log"
	@tail -n 200 $(BUILD_DIR)/qemu.log 2>/dev/null || true

check:
	$(MAKE) -B kernel
.PHONY: all kernel iso run clean verify-iso check debug-iso qemu-log qemu-quick qemu-debug qemu-triage


BUILD_DIR := build
ISO_NAME := suayos.iso
ISO_PATH := $(BUILD_DIR)/$(ISO_NAME)
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_MAP := $(BUILD_DIR)/kernel.map
OBJS := $(BUILD_DIR)/boot.o $(BUILD_DIR)/kmain.o $(BUILD_DIR)/vga.o $(BUILD_DIR)/serial.o $(BUILD_DIR)/io.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/isr_stub.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/pit.o $(BUILD_DIR)/irq.o $(BUILD_DIR)/irq_asm.o $(BUILD_DIR)/mb2.o $(BUILD_DIR)/bump.o $(BUILD_DIR)/diag.o $(BUILD_DIR)/pmm.o $(BUILD_DIR)/vmm.o $(BUILD_DIR)/kmem.o $(BUILD_DIR)/kbd.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/string.o $(BUILD_DIR)/user.o $(BUILD_DIR)/enter_usermode.o $(BUILD_DIR)/syscall.o $(BUILD_DIR)/syscall_stub.o $(BUILD_DIR)/isr_common.o
include isr_make.inc
include syscall_make.inc

include user_make.inc
INITRD_IMG := $(BUILD_DIR)/initrd.img
$(BUILD_DIR)/string.o: kernel/src/string.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/shell.o: kernel/src/shell.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/kbd.o: kernel/src/kbd.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/kmem.o: kernel/src/kmem.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(INITRD_IMG):
	@mkdir -p $(BUILD_DIR)
	@./scripts/mkinitrd.sh || true
	@if [ ! -f "$(INITRD_IMG)" ]; then \
		echo "[mkinitrd] WARNING: $(INITRD_IMG) missing; creating empty initrd"; \
		: > "$(INITRD_IMG)"; \
	fi
$(BUILD_DIR)/vmm.o: kernel/src/vmm.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/pmm.o: kernel/src/pmm.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/diag.o: kernel/src/diag.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/mb2.o: kernel/src/mb2.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/bump.o: kernel/src/bump.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/irq.o: kernel/arch/x86_64/irq.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/irq_asm.o: kernel/arch/x86_64/irq.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/pic.o: kernel/arch/x86_64/pic.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pit.o: kernel/arch/x86_64/pit.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/gdt.o: kernel/arch/x86_64/gdt.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/idt.o: kernel/arch/x86_64/idt.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/isr_stub.o: kernel/arch/x86_64/isr.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/io.o: kernel/arch/x86_64/io.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/serial.o: kernel/src/serial.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/vga.o: kernel/src/vga.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
LINKER := kernel/arch/x86_64/linker.ld

CC := $(shell command -v clang 2>/dev/null || command -v gcc)
CFLAGS := -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone -O2 -Wall -Wextra \
	-fno-builtin -fno-omit-frame-pointer \
	-mno-sse -mno-sse2 -mno-mmx -msoft-float \
	-DENABLE_IRQ=0
LDFLAGS := -T $(LINKER) -nostdlib -static


all: iso


kernel: $(KERNEL_ELF)


$(KERNEL_ELF): $(OBJS) $(LINKER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-Map,$(KERNEL_MAP) $(OBJS) -o $(KERNEL_ELF)

$(BUILD_DIR)/boot.o: kernel/arch/x86_64/boot.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kmain.o: kernel/src/kmain.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@




iso: kernel $(INITRD_IMG)
	@mkdir -p $(BUILD_DIR)
	cp $(BUILD_DIR)/kernel.elf iso_root/boot/kernel.elf
	cp $(INITRD_IMG) iso_root/boot/initrd.img
	mkdir -p iso_root/boot/grub
	cp grub.cfg iso_root/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_PATH) iso_root



run: iso
	qemu-system-x86_64 -cdrom $(ISO_PATH) -serial stdio


debug-iso: CFLAGS := $(filter-out -O2,$(CFLAGS)) -O0 -g -fno-omit-frame-pointer
debug-iso: clean iso
	@echo "[debug-iso] Built $(ISO_PATH)"
	@echo "[debug-iso] ELF: $(KERNEL_ELF)"
	@echo "[debug-iso] MAP: $(KERNEL_MAP)"


clean:
	@rm -rf $(BUILD_DIR)
