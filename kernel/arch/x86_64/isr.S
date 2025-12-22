.global isr_default_stub
.type isr_default_stub, @function
isr_default_stub:
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbx
    pushq %rbp
    mov $msg, %rsi
    call serial_write
.hang:
    hlt
    jmp .hang
msg:
    .asciz "[ISR] Interrupt received\n"
