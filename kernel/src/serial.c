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
void serial_write(const char *s) {
    while (*s) {
        while (!serial_is_transmit_empty()) {}
        outb(COM1, *s++);
    }
}
