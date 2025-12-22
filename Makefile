.PHONY: all kernel iso run clean


BUILD_DIR := build
ISO_NAME := suayos.iso
ISO_PATH := $(BUILD_DIR)/$(ISO_NAME)
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
SRC := kernel/src/main.c
LINKER := kernel/arch/x86_64/linker.ld

CC := $(shell command -v clang 2>/dev/null || command -v gcc)
CFLAGS := -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone -O2 -Wall -Wextra
LDFLAGS := -T $(LINKER) -nostdlib -static


all: iso


kernel: $(KERNEL_ELF)

$(KERNEL_ELF): $(SRC) $(LINKER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(SRC) -o $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUILD_DIR)/main.o -o $(KERNEL_ELF)


iso: kernel
	@mkdir -p $(BUILD_DIR)
	@echo "TODO: produce bootable ISO at $(ISO_PATH)"


run: iso
	@echo "TODO: launch QEMU with $(ISO_PATH)"


clean:
	@rm -rf $(BUILD_DIR)
