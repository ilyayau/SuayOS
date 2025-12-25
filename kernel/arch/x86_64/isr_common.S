.intel_syntax noprefix
.global isr_common_stub
.type isr_common_stub, @function
isr_common_stub:
    // Save registers
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push rbx
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    // Stack at entry (ring0):
    //   [vector][error][RIP][CS][RFLAGS](...[RSP][SS] if CPL change)
    // After pushing 15 regs (120 bytes):
    //   vector at [rsp+0x78], error at [rsp+0x80]
    mov rdi, [rsp + 0x78] // vector number
    mov rsi, [rsp + 0x80] // error code
    mov rdx, rsp           // pointer to saved regs
    call isr_dispatch
    // Restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rbx
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    add rsp, 16           // pop vector, error code
    iretq
