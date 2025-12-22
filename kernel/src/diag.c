#include "../include/diag.h"
#include "../include/serial.h"
#include "../include/vga.h"
#include <stdint.h>

void print_dec(uint64_t n, char *buf) {
    char tmp[21]; int i = 0, j = 0;
    if (n == 0) { buf[0] = '0'; buf[1] = 0; return; }
    while (n) { tmp[i++] = '0' + (n % 10); n /= 10; }
    while (i--) buf[j++] = tmp[i];
    buf[j] = 0;
}
void print_hex(uint64_t n, char *buf, int prefix) {
    char tmp[17]; int i = 0, j = 0;
    if (prefix) { buf[j++] = '0'; buf[j++] = 'x'; }
    if (n == 0) { buf[j++] = '0'; buf[j] = 0; return; }
    while (n) { tmp[i++] = "0123456789ABCDEF"[n & 0xF]; n >>= 4; }
    while (i--) buf[j++] = tmp[i];
    buf[j] = 0;
}
void panic(const char *msg) {
    serial_write("PANIC: ");
    serial_write(msg);
    serial_write("\n");
    vga_clear();
    vga_print("PANIC: ");
    vga_print(msg);
    for (;;) { __asm__ volatile ("cli; hlt"); }
}
void panic_assert(const char *file, int line, const char *expr) {
    char buf[64];
    serial_write("ASSERT FAIL: ");
    serial_write(expr);
    serial_write(" at ");
    serial_write(file);
    serial_write(":");
    print_dec((uint64_t)line, buf);
    serial_write(buf);
    serial_write("\n");
    vga_clear();
    vga_print("ASSERT FAIL: ");
    vga_print(expr);
    vga_print(" at ");
    vga_print(file);
    vga_print(":");
    print_dec((uint64_t)line, buf);
    vga_print(buf);
    for (;;) { __asm__ volatile ("cli; hlt"); }
}
