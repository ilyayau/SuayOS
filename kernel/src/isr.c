#include "../include/isr.h"
#include "../include/syscall.h"
#include <stdint.h>

__attribute__((visibility("default")))
#ifdef __cplusplus
extern "C"
#endif
void isr_dispatch(uint64_t vector, uint64_t err, void *frame) {
    if (vector == 0x80) {
        // Syscall: rax=num, rdi=arg1, rsi=arg2, rdx=arg3
        uint64_t *regs = (uint64_t *)frame;
        uint64_t num = regs[15]; // rax
        uint64_t arg1 = regs[10]; // rdi
        uint64_t arg2 = regs[9];  // rsi
        uint64_t arg3 = regs[8];  // rdx
        regs[15] = syscall_handler(num, arg1, arg2, arg3); // return in rax
        return;
    }
    // Default: just halt
    __asm__ volatile ("cli; hlt");
}
