#include <stdint.h>
#include "../include/serial.h"
#include "../include/io.h"
#define COM1 0x3F8
void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
    (void)inb(COM1);
}
static int serial_is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

static int serial_is_data_ready(void) {
    return inb(COM1 + 5) & 0x01;
}
void serial_write(const char *s) {
    while (*s) {
        while (!serial_is_transmit_empty()) {}
        outb(COM1, *s++);
    }
}

void serial_putc(char c) {
    while (!serial_is_transmit_empty()) {}
    outb(COM1, c);
}

int serial_getc_nonblock(void) {
    if (!serial_is_data_ready()) {
        return -1;
    }
    return (int)(uint8_t)inb(COM1);
}

void serial_write_bytes(const void *data, unsigned long n) {
    const uint8_t *p = (const uint8_t *)data;
    for (unsigned long i = 0; i < n; ++i) {
        serial_putc((char)p[i]);
    }
}
