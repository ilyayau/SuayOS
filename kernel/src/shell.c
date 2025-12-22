#include "../include/shell.h"
#include "../include/vga.h"
#include "../include/kbd.h"
#include "../include/diag.h"
#include "../include/pmm.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#define SHELL_BUF_SIZE 128
static char line_buf[SHELL_BUF_SIZE];
static int line_len = 0;
static void shell_prompt(void) { vga_print("suayos> "); }
static void shell_newline(void) { vga_putc('\n'); }
static void shell_backspace(void) {
    if (line_len > 0) {
        vga_putc('\b');
        --line_len;
    }
}
static void shell_clear_line(void) {
    while (line_len > 0) shell_backspace();
}
static void shell_help(void) {
    vga_print("help clear ticks meminfo reboot\n");
}
static void shell_clear(void) {
    vga_clear();
}
extern uint64_t get_ticks(void);
static void shell_ticks(void) {
    char buf[32];
    uint64_t t = get_ticks();
    int i = 30; buf[31] = 0;
    if (t == 0) { buf[0] = '0'; buf[1] = 0; vga_print(buf); vga_putc('\n'); return; }
    while (t && i >= 0) { buf[i--] = '0' + (t % 10); t /= 10; }
    vga_print(buf + i + 1); vga_putc('\n');
}
static void shell_meminfo(void) {
    char buf[64];
    uint64_t free = pmm_free_pages_count();
    uint64_t total = pmm_total_pages();
    vga_print("meminfo: ");
    int i = 30; buf[31] = 0;
    uint64_t n = free;
    if (n == 0) { buf[0] = '0'; buf[1] = 0; vga_print(buf); } else { while (n && i >= 0) { buf[i--] = '0' + (n % 10); n /= 10; } vga_print(buf + i + 1); }
    vga_print(" free / ");
    i = 30; n = total;
    if (n == 0) { buf[0] = '0'; buf[1] = 0; vga_print(buf); } else { while (n && i >= 0) { buf[i--] = '0' + (n % 10); n /= 10; } vga_print(buf + i + 1); }
    vga_print(" total pages\n");
}
static void shell_reboot(void) {
    vga_print("Rebooting...\n");
    for (;;) {
        asm volatile ("cli");
        asm volatile ("mov $0xFE, %al");
        asm volatile ("out %al, $0x64");
    }
}
static void shell_dispatch(const char *cmd) {
    if (!cmd[0]) return;
    if (!strcmp(cmd, "help")) shell_help();
    else if (!strcmp(cmd, "clear")) shell_clear();
    else if (!strcmp(cmd, "ticks")) shell_ticks();
    else if (!strcmp(cmd, "meminfo")) shell_meminfo();
    else if (!strcmp(cmd, "reboot")) shell_reboot();
    else vga_print("Unknown command\n");
}

void shell_run(void) {
    shell_prompt();
    line_len = 0;
    for (;;) {
        int c = kbd_read_char();
        if (c < 0) continue;
        if (c == '\n') {
            line_buf[line_len] = 0;
            shell_newline();
            shell_dispatch(line_buf);
            line_len = 0;
            shell_prompt();
        } else if (c == '\b' || c == 127) {
            shell_backspace();
        } else if (c >= 32 && c < 127 && line_len < SHELL_BUF_SIZE - 1) {
            vga_putc((char)c);
            line_buf[line_len++] = (char)c;
        }
    }
}
