.PHONY: all kernel iso run clean


BUILD_DIR := build
ISO_NAME := suayos.iso
ISO_PATH := $(BUILD_DIR)/$(ISO_NAME)
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
OBJS := $(BUILD_DIR)/boot.o $(BUILD_DIR)/kmain.o
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
	@echo "TODO: produce bootable ISO at $(ISO_PATH)"


run: iso
	@echo "TODO: launch QEMU with $(ISO_PATH)"


clean:
	@rm -rf $(BUILD_DIR)
