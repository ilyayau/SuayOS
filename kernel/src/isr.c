#include "../include/isr.h"
#include "../include/syscall.h"
#include "../include/serial.h"
#include "../include/diag.h"
#include <stdint.h>

__attribute__((visibility("default")))
#ifdef __cplusplus
extern "C"
#endif
void isr_dispatch(uint64_t vector, uint64_t err, void *frame) {
    // frame points to saved regs (top = r15). Layout from isr_common_stub pushes:
    //  r15,r14,r13,r12,r11,r10,r9,r8,rbp,rbx,rdi,rsi,rdx,rcx,rax
    uint64_t *regs = (uint64_t *)frame;

    if (vector < 32) {
        // Stack beyond regs contains: [vector][err][RIP][CS][RFLAGS](...[RSP][SS] if CPL change)
        uint64_t vec_on_stack = regs[15];
        uint64_t err_on_stack = regs[16];
        uint64_t rip = regs[17];
        uint64_t cs = regs[18];
        uint64_t rflags = regs[19];

        uint64_t rsp = 0;
        uint64_t ss = 0;
        if ((cs & 3) == 3) {
            rsp = regs[20];
            ss = regs[21];
        } else {
            // No stack-switch frame; report current RSP as best-effort.
            __asm__ volatile ("mov %%rsp, %0" : "=r"(rsp));
            uint16_t ss16;
            __asm__ volatile ("mov %%ss, %0" : "=r"(ss16));
            ss = ss16;
        }

        char buf[64];
        serial_write("EXCEPTION vector="); print_dec(vec_on_stack, buf); serial_write(buf);
        serial_write(" err="); print_hex(err_on_stack, buf, 1); serial_write(buf);
        serial_write("\n");
        serial_write("RIP="); print_hex(rip, buf, 1); serial_write(buf);
        serial_write(" RSP="); print_hex(rsp, buf, 1); serial_write(buf);
        serial_write(" RFLAGS="); print_hex(rflags, buf, 1); serial_write(buf);
        serial_write(" CS="); print_hex(cs, buf, 1); serial_write(buf);
        serial_write(" SS="); print_hex(ss, buf, 1); serial_write(buf);
        serial_write("\n");

        (void)vector;
        (void)err;
        for (;;) __asm__ volatile ("cli; hlt");
    }

    if (vector == 0x80) {
        // Syscall: rax=num, rdi=arg1, rsi=arg2, rdx=arg3
        uint64_t num = regs[14]; // rax
        uint64_t arg1 = regs[10]; // rdi
        uint64_t arg2 = regs[11]; // rsi
        uint64_t arg3 = regs[12]; // rdx
        regs[14] = syscall_handler(num, arg1, arg2, arg3); // return in rax
        return;
    }
    // Default: just halt
    __asm__ volatile ("cli; hlt");
}
