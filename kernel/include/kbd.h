#pragma once
// Keyboard ring buffer size
#define KBD_BUF_SIZE 128
void kbd_init(void);
// Non-blocking: returns -1 if buffer empty, else ASCII char (0-255)
int kbd_read_char(void);
void kbd_handle_irq(void);
