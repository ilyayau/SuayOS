.intel_syntax noprefix
.global irq0_stub
.type irq0_stub, @function
irq0_stub:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push rbx
    push rbp
    call irq0_handler
    pop rbp
    pop rbx
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    iretq
