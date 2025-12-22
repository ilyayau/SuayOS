$(BUILD_DIR)/user.o: kernel/src/user.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/enter_usermode.o: kernel/arch/x86_64/enter_usermode.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
