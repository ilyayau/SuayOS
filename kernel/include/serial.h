#pragma once
// COM1 serial (0x3F8)
void serial_init(void);
void serial_write(const char *s);
void serial_putc(char c);

// Returns -1 if no byte is available.
int serial_getc_nonblock(void);

// Writes raw bytes (does not stop on NUL).
void serial_write_bytes(const void *data, unsigned long n);
