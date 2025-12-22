.global isr_syscall_stub
.type isr_syscall_stub, @function
isr_syscall_stub:
    pushq $0                # dummy error code for int 0x80
    pushq $0x80             # vector number
    jmp isr_common_stub
