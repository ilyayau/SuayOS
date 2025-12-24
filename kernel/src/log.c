#include "../include/log.h"
#include "../include/io.h"
#include "../include/serial.h"

#define DEBUGCON_PORT 0xE9

static volatile int g_serial_ready = 0;

void dbg_putc(char c) {
    outb(DEBUGCON_PORT, (uint8_t)c);
}

void log_init(void) {
    serial_init();
    g_serial_ready = 1;
}

void logc(char c) {
    dbg_putc(c);
    if (g_serial_ready) {
        serial_putc(c);
    }
}

void logs(const char *s) {
    if (!s) {
        return;
    }
    while (*s) {
        logc(*s++);
    }
}
