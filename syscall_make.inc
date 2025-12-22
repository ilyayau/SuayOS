# Syscall stub and handler for int 0x80
$(BUILD_DIR)/syscall_stub.o: kernel/arch/x86_64/syscall_stub.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/syscall.o: kernel/src/syscall.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
