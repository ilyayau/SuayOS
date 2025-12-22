#include "../include/syscall.h"
#include "../include/serial.h"
#include <stdint.h>

#define SYS_WRITE 1
#define SYS_EXIT  2

void syscall_init(void) {}

uint64_t syscall_handler(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    switch (num) {
        case SYS_WRITE: {
            const char *s = (const char *)arg1;
            uint64_t len = arg2;
            for (uint64_t i = 0; i < len; ++i) serial_putc(s[i]);
            return len;
        }
        case SYS_EXIT:
            serial_write("[user exited]\n");
            // For now, just halt
            __asm__ volatile ("cli; hlt");
            return 0;
        default:
            serial_write("[bad syscall]\n");
            return (uint64_t)-1;
    }
}
