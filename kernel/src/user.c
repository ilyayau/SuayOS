#include "../include/user.h"
#include "../include/gdt.h"
#include <stdint.h>

// User stack (4KB)
static uint8_t user_stack[4096] __attribute__((aligned(16)));

// User program: just prints hello and exits via syscall
static uint8_t user_prog[] = {
    0xB8, 0x01, 0x00, 0x00, 0x00, // mov eax, 1 (sys_write)
    0x48, 0x8D, 0x35, 0x0A, 0x00, 0x00, 0x00, // lea rsi, [rip+0xa] (msg)
    0xBA, 0x0E, 0x00, 0x00, 0x00, // mov edx, 14 (len)
    0x0F, 0x05,                   // syscall
    0xB8, 0x02, 0x00, 0x00, 0x00, // mov eax, 2 (sys_exit)
    0x0F, 0x05,                   // syscall
    // msg:
    'H','e','l','l','o',' ','f','r','o','m',' ','u','s','r','\n'
};

extern void enter_usermode(void *rip, void *rsp, uint64_t arg);

void user_run(void) {
    // Copy user_prog to a known address (just use &user_prog for now)
    void *rip = user_prog;
    void *rsp = user_stack + sizeof(user_stack);
    enter_usermode(rip, rsp, 0);
}
