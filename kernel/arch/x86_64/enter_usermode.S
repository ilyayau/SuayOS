
# x86_64 enter_usermode: switch to ring3 and iretq
# void enter_usermode(void *rip, void *rsp, uint64_t arg)
# rdi = rip, rsi = rsp, rdx = arg

    .globl enter_usermode
    .type enter_usermode, @function
enter_usermode:
    mov %rsi, %rsp        # set user stack
    pushq $0x20           # SS = user data
    pushq %rsi            # RSP = user stack
    pushfq                # RFLAGS
    pushq $0x18           # CS = user code
    pushq %rdi            # RIP = user entry
    mov $0x23, %ax        # DS/ES/FS/GS = user data (RPL=3)
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    swapgs                # set GS base for user
    iretq                 # pop to ring3
    hlt                   # should never reach here
