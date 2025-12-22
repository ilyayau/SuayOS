check:
	$(MAKE) -B kernel
.PHONY: all kernel iso run clean


BUILD_DIR := build
ISO_NAME := suayos.iso
ISO_PATH := $(BUILD_DIR)/$(ISO_NAME)
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
OBJS := $(BUILD_DIR)/boot.o $(BUILD_DIR)/kmain.o $(BUILD_DIR)/vga.o $(BUILD_DIR)/serial.o $(BUILD_DIR)/io.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/pit.o $(BUILD_DIR)/irq.o $(BUILD_DIR)/irq_asm.o $(BUILD_DIR)/mb2.o $(BUILD_DIR)/bump.o $(BUILD_DIR)/diag.o $(BUILD_DIR)/pmm.o
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

$(BUILD_DIR)/isr.o: kernel/arch/x86_64/isr.S
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
CFLAGS := -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone -O2 -Wall -Wextra
LDFLAGS := -T $(LINKER) -nostdlib -static


all: iso


kernel: $(KERNEL_ELF)


$(KERNEL_ELF): $(OBJS) $(LINKER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(KERNEL_ELF)

$(BUILD_DIR)/boot.o: kernel/arch/x86_64/boot.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kmain.o: kernel/src/kmain.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@



iso: kernel
	@mkdir -p $(BUILD_DIR)
	cp $(BUILD_DIR)/kernel.elf iso_root/boot/kernel.elf
	grub-mkrescue -o $(ISO_PATH) iso_root > /dev/null 2>&1



run: iso
	qemu-system-x86_64 -cdrom $(ISO_PATH) -serial stdio


clean:
	@rm -rf $(BUILD_DIR)
