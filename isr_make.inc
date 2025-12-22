# isr_common.o rule
$(BUILD_DIR)/isr_common.o: kernel/arch/x86_64/isr_common.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/isr.o: kernel/src/isr.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
