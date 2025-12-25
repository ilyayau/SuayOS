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

void klog(const char *msg) {
    serial_write(msg);
}

static inline uint64_t read_rflags(void) {
    uint64_t rflags;
    __asm__ volatile ("pushfq; pop %0" : "=r"(rflags));
    return rflags;
}

static inline uint16_t read_cs(void) {
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs;
}

static inline uint16_t read_ss(void) {
    uint16_t ss;
    __asm__ volatile ("mov %%ss, %0" : "=r"(ss));
    return ss;
}

static inline uint64_t read_rsp(void) {
    uint64_t rsp;
    __asm__ volatile ("mov %%rsp, %0" : "=r"(rsp));
    return rsp;
}

static void panic_dump_regs(void) {
    char buf[64];
    uint64_t rip = (uint64_t)__builtin_return_address(0);
    uint64_t rsp = read_rsp();
    uint64_t rflags = read_rflags();
    uint64_t cs = read_cs();
    uint64_t ss = read_ss();

    serial_write("RIP="); print_hex(rip, buf, 1); serial_write(buf);
    serial_write(" RSP="); print_hex(rsp, buf, 1); serial_write(buf);
    serial_write(" RFLAGS="); print_hex(rflags, buf, 1); serial_write(buf);
    serial_write(" CS="); print_hex(cs, buf, 1); serial_write(buf);
    serial_write(" SS="); print_hex(ss, buf, 1); serial_write(buf);
    serial_write("\n");
}

__attribute__((noreturn)) void panic(const char *msg) {
    serial_write("PANIC: ");
    serial_write(msg);
    serial_write("\n");
    panic_dump_regs();

    vga_clear();
    vga_print("PANIC: ");
    vga_print(msg);
    vga_print("\n");
    for (;;) { __asm__ volatile ("cli; hlt"); }
}

__attribute__((noreturn)) void panic_at(const char *file, int line, const char *msg) {
    char buf[64];
    serial_write("PANIC: ");
    serial_write(msg);
    serial_write(" at ");
    serial_write(file);
    serial_write(":");
    print_dec((uint64_t)line, buf);
    serial_write(buf);
    serial_write("\n");
    panic_dump_regs();

    vga_clear();
    vga_print("PANIC: ");
    vga_print(msg);
    vga_print(" at ");
    vga_print(file);
    vga_print(":");
    vga_print(buf);
    vga_print("\n");
    for (;;) { __asm__ volatile ("cli; hlt"); }
}

__attribute__((noreturn)) void panic_assert(const char *file, int line, const char *expr) {
    char buf[64];
    serial_write("ASSERT FAIL: ");
    serial_write(expr);
    serial_write(" at ");
    serial_write(file);
    serial_write(":");
    print_dec((uint64_t)line, buf);
    serial_write(buf);
    serial_write("\n");
    panic_dump_regs();
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

__attribute__((noreturn)) void panic_assert_msg(const char *file, int line, const char *expr, const char *msg) {
    char buf[64];
    serial_write("ASSERT FAIL: ");
    serial_write(expr);
    serial_write(" ( ");
    serial_write(msg);
    serial_write(" ) at ");
    serial_write(file);
    serial_write(":");
    print_dec((uint64_t)line, buf);
    serial_write(buf);
    serial_write("\n");
    panic_dump_regs();

    vga_clear();
    vga_print("ASSERT FAIL: ");
    vga_print(expr);
    vga_print("\n");
    vga_print(msg);
    vga_print("\n");
    vga_print(file);
    vga_print(":");
    vga_print(buf);
    for (;;) { __asm__ volatile ("cli; hlt"); }
}
