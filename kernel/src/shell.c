#include "../include/shell.h"
#include "../include/serial.h"
#include "../include/diag.h"
#include "../include/pmm.h"
#include "../include/mb2.h"
#include "../include/vmm.h"
#include "../include/string.h"

#include <stdint.h>

#define SHELL_BUF_SIZE 128

static void sh_putc(char c) { serial_putc(c); }
static void sh_write(const char *s) { serial_write(s); }
static void sh_write_nl(const char *s) { serial_write(s); serial_putc('\n'); }
static void sh_nl(void) { serial_putc('\n'); }

static void sh_prompt(void) { sh_write("suayos> "); }

static void sh_print_dec_u64(uint64_t n) {
    char buf[32];
    print_dec(n, buf);
    sh_write(buf);
}

static void cmd_help(void) {
    sh_write_nl("help clear echo meminfo ticks reboot halt ls cat");
}

static void cmd_clear(void) {
    // ANSI clear screen + cursor home.
    sh_write("\x1b[2J\x1b[H");
}

extern uint64_t get_ticks(void);
static void cmd_ticks(void) {
    sh_print_dec_u64(get_ticks());
    sh_nl();
}

static void cmd_meminfo(void) {
    uint64_t free_pages = pmm_free_pages_count();
    uint64_t total_pages = pmm_total_pages();
    sh_write("meminfo: ");
    sh_print_dec_u64(free_pages);
    sh_write(" free / ");
    sh_print_dec_u64(total_pages);
    sh_write_nl(" total pages");
}

__attribute__((noreturn)) static void cmd_reboot(void) {
    sh_write_nl("Rebooting...");
    for (;;) {
        __asm__ volatile ("cli");
        __asm__ volatile ("mov $0xFE, %al");
        __asm__ volatile ("out %al, $0x64");
    }
}

__attribute__((noreturn)) static void cmd_halt(void) {
    sh_write_nl("Halting...");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

static const uint8_t *initrd_base(uint64_t *out_size) {
    uint64_t start = mb2_get_module_start();
    uint64_t end = mb2_get_module_end();
    if (!start || !end || end <= start) {
        *out_size = 0;
        return 0;
    }
    *out_size = end - start;
    return (const uint8_t *)(KERNEL_VMA_BASE + start);
}

static uint32_t be32(const uint8_t b[4]) {
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | (uint32_t)b[3];
}

static int initrd_name_eq(const uint8_t name56[56], const char *want) {
    char name[57];
    int end = 55;
    while (end >= 0 && (name56[end] == ' ' || name56[end] == 0)) {
        --end;
    }
    int len = end + 1;
    for (int i = 0; i < len; ++i) name[i] = (char)name56[i];
    name[len] = 0;
    return strcmp(name, want) == 0;
}

static void initrd_print_name(const uint8_t name56[56]) {
    char name[57];
    int end = 55;
    while (end >= 0 && (name56[end] == ' ' || name56[end] == 0)) {
        --end;
    }
    int len = end + 1;
    for (int i = 0; i < len; ++i) name[i] = (char)name56[i];
    name[len] = 0;
    sh_write(name);
}

static void cmd_ls(void) {
    uint64_t sz = 0;
    const uint8_t *base = initrd_base(&sz);
    if (!base || sz < 60) {
        sh_write_nl("ls: no initrd");
        return;
    }

    uint64_t off = 0;
    while (off + 60 <= sz) {
        const uint8_t *name56 = base + off;
        const uint8_t *size_be = base + off + 56;
        uint32_t fsz = be32(size_be);
        off += 60;
        if (off + (uint64_t)fsz > sz) break;

        initrd_print_name(name56);
        sh_nl();

        off += fsz;
    }
}

static void cmd_cat(const char *arg) {
    if (!arg || !arg[0]) {
        sh_write_nl("cat: missing filename");
        return;
    }

    uint64_t sz = 0;
    const uint8_t *base = initrd_base(&sz);
    if (!base || sz < 60) {
        sh_write_nl("cat: no initrd");
        return;
    }

    uint64_t off = 0;
    while (off + 60 <= sz) {
        const uint8_t *name56 = base + off;
        const uint8_t *size_be = base + off + 56;
        uint32_t fsz = be32(size_be);
        off += 60;
        if (off + (uint64_t)fsz > sz) break;

        if (initrd_name_eq(name56, arg)) {
            serial_write_bytes(base + off, (unsigned long)fsz);
            if (fsz == 0 || base[off + fsz - 1] != '\n') {
                sh_nl();
            }
            return;
        }

        off += fsz;
    }

    sh_write("cat: not found: ");
    sh_write_nl(arg);
}

static const char *skip_spaces(const char *s) {
    while (s && *s == ' ') ++s;
    return s;
}

static void shell_dispatch(char *line) {
    line = (char *)skip_spaces(line);
    if (!line || !line[0]) return;

    char *args = line;
    while (*args && *args != ' ') ++args;
    if (*args) {
        *args++ = 0;
    }
    args = (char *)skip_spaces(args);

    if (strcmp(line, "help") == 0) cmd_help();
    else if (strcmp(line, "clear") == 0) cmd_clear();
    else if (strcmp(line, "echo") == 0) { if (args) sh_write_nl(args); else sh_nl(); }
    else if (strcmp(line, "meminfo") == 0) cmd_meminfo();
    else if (strcmp(line, "ticks") == 0) cmd_ticks();
    else if (strcmp(line, "reboot") == 0) cmd_reboot();
    else if (strcmp(line, "halt") == 0) cmd_halt();
    else if (strcmp(line, "ls") == 0) cmd_ls();
    else if (strcmp(line, "cat") == 0) cmd_cat(args);
    else {
        sh_write_nl("Unknown command");
    }
}

static void shell_selftest(void) {
    static const char *script[] = {
        "help",
        "meminfo",
        "ls",
        "cat readme.md",
        0,
    };

    char buf[SHELL_BUF_SIZE];
    for (int i = 0; script[i]; ++i) {
        sh_prompt();
        sh_write(script[i]);
        sh_nl();

        // Copy into mutable buffer for in-place tokenization.
        unsigned long n = strlen(script[i]);
        if (n >= SHELL_BUF_SIZE) n = SHELL_BUF_SIZE - 1;
        memcpy(buf, script[i], n);
        buf[n] = 0;
        shell_dispatch(buf);
    }
}

void shell_run(void) {
    shell_selftest();
    sh_prompt();

    char line[SHELL_BUF_SIZE];
    unsigned long len = 0;
    for (;;) {
        int c = serial_getc_nonblock();
        if (c < 0) {
            __asm__ volatile ("pause");
            continue;
        }

        if (c == '\r') c = '\n';

        if (c == '\n') {
            line[len] = 0;
            sh_nl();
            shell_dispatch(line);
            len = 0;
            sh_prompt();
            continue;
        }

        if (c == '\b' || c == 127) {
            if (len > 0) {
                --len;
                // Erase on ANSI terminals.
                sh_write("\b \b");
            }
            continue;
        }

        if (c >= 32 && c < 127) {
            if (len < SHELL_BUF_SIZE - 1) {
                line[len++] = (char)c;
                sh_putc((char)c);
            }
            continue;
        }
    }
}
