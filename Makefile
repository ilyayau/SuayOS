.PHONY: all kernel iso run clean

BUILD_DIR := build
ISO_NAME := suayos.iso
ISO_PATH := $(BUILD_DIR)/$(ISO_NAME)

all: iso

kernel:
	@mkdir -p $(BUILD_DIR)
	@echo "TODO: build freestanding kernel"

iso: kernel
	@mkdir -p $(BUILD_DIR)
	@echo "TODO: produce bootable ISO at $(ISO_PATH)"

run: iso
	@echo "TODO: launch QEMU with $(ISO_PATH)"

clean:
	@rm -rf $(BUILD_DIR)
