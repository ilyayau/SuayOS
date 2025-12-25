
.section .multiboot, "a"
.align 8
multiboot2_header:
    .long 0xE85250D6              # magic
    .long 0                       # architecture (0 = i386+, works for x86_64)
    .long multiboot2_header_end - multiboot2_header # header length
    .long -(0xE85250D6 + 0 + (multiboot2_header_end - multiboot2_header)) # checksum
    .short 0
    .short 0
    .long 8
multiboot2_header_end:

.section .text
.code32
.global _start
.type _start, @function
_start:
    cli
    mov $stack32_top, %esp
    movl %ebx, mb2_info_ptr # Multiboot2 info pointer (32-bit physical)

    # Ultra-early serial init (COM1) + checkpoints (32-bit)
    mov $0x3F8, %dx
    mov $0x00, %al
    out %al, %dx         # Disable interrupts (IER)
    mov $0x3FB, %dx
    mov $0x80, %al
    out %al, %dx         # Enable DLAB (LCR)
    mov $0x3F8, %dx
    mov $0x03, %al
    out %al, %dx         # Divisor low (DLL) = 3
    mov $0x3F9, %dx
    mov $0x00, %al
    out %al, %dx         # Divisor high (DLM)
    mov $0x3FB, %dx
    mov $0x03, %al
    out %al, %dx         # 8N1
    mov $0x3FA, %dx
    mov $0xC7, %al
    out %al, %dx         # FIFO
    mov $0x3FC, %dx
    mov $0x0B, %al
    out %al, %dx         # OUT2/RTS/DTR

    .macro serial32_putc char
        mov $0x3FD, %dx
    1:
        in %dx, %al
        test $0x20, %al
        jz 1b
        mov $0x3F8, %dx
        movb \char, %al
        out %al, %dx
    .endm

    # BOOT0 at entry
    serial32_putc $'B'
    serial32_putc $'O'
    serial32_putc $'O'
    serial32_putc $'T'
    serial32_putc $'0'
    serial32_putc $0x0A

    # Set up temporary GDT
    lgdt gdt32_ptr
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    # BOOT1 after loading GDT / segments
    serial32_putc $'B'
    serial32_putc $'O'
    serial32_putc $'O'
    serial32_putc $'T'
    serial32_putc $'1'
    serial32_putc $0x0A

    # Enable PAE
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4

    # Set up PML4 (identity map low 1GiB + map high-half window at 0xffffffff80000000)
    mov $pml4, %eax
    mov %eax, %cr3

    # Enable long mode (LME)
    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr

    # Enable paging
    mov %cr0, %eax
    or $0x80000001, %eax
    mov %eax, %cr0

    # BOOT2 before long-mode jump
    serial32_putc $'B'
    serial32_putc $'O'
    serial32_putc $'O'
    serial32_putc $'T'
    serial32_putc $'2'
    serial32_putc $0x0A

    # Far jump to 64-bit code
    ljmp $0x08, $_start64

.align 16
gdt32:
    .quad 0x0000000000000000
    .quad 0x00af9a000000ffff # 0x08: 64-bit code
    .quad 0x00cf92000000ffff # 0x10: 32-bit data
gdt32_ptr:
    .word 24-1
    .long gdt32


.align 4096
pml4:
    .quad pdpt + 0x03
    .fill 510,8,0
    .quad pdpt + 0x03

.align 4096
pdpt:
    .quad pd + 0x03
    .fill 509,8,0
    .quad pd + 0x03
    .quad 0

.align 4096
pd:
    .set __pd_i, 0
    .rept 512
        .quad (__pd_i << 21) | 0x83
        .set __pd_i, __pd_i + 1
    .endr

.section .bss
.align 16
stack32:
    .skip 8192
stack32_top:

.section .bss
.align 16
stack64:
    .skip 16384
stack64_top:

.align 8
mb2_info_ptr:
    .quad 0

.section .text
.code64

.global _start64
.type _start64, @function
_start64:
    cli
    mov $stack64_top, %rsp
    # Early serial COM1 init (0x3F8)
    mov $0x3F8, %dx
    mov $0x00, %al
    out %al, %dx         # Disable interrupts (IER)
    mov $0x3FB, %dx
    mov $0x80, %al
    out %al, %dx         # Enable DLAB (LCR)
    mov $0x3F8, %dx
    mov $0x03, %al
    out %al, %dx         # Set divisor low byte (DLL)
    mov $0x3F9, %dx
    mov $0x00, %al
    out %al, %dx         # Set divisor high byte (DLM)
    mov $0x3FB, %dx
    mov $0x03, %al
    out %al, %dx         # 8N1 (LCR)
    mov $0x3FA, %dx
    mov $0xC7, %al
    out %al, %dx         # Enable FIFO (FCR)
    mov $0x3FC, %dx
    mov $0x0B, %al
    out %al, %dx         # Enable OUT2/RTS/DTR (MCR)

    # Early serial putc (macro)
    .macro serial_putc char
        mov $0x3FD, %dx
    1:
        in %dx, %al
        test $0x20, %al
        jz 1b
        mov $0x3F8, %dx
        movb \char, %al
        out %al, %dx
    .endm

    # BOOT3 entered 64-bit
    serial_putc $'B'
    serial_putc $'O'
    serial_putc $'O'
    serial_putc $'T'
    serial_putc $'3'
    serial_putc $0x0A

    # Early IDT: default handler for everything
    xor %edi, %edi
.idt_all:
    mov $isr_default_stub, %rsi
    call set_idt_gate
    inc %edi
    cmp $256, %edi
    jne .idt_all

    # Exceptions 0..31 -> common register-dumping handler
    mov $0, %edi;  mov $isr_stub_0, %rsi;  call set_idt_gate
    mov $1, %edi;  mov $isr_stub_1, %rsi;  call set_idt_gate
    mov $2, %edi;  mov $isr_stub_2, %rsi;  call set_idt_gate
    mov $3, %edi;  mov $isr_stub_3, %rsi;  call set_idt_gate
    mov $4, %edi;  mov $isr_stub_4, %rsi;  call set_idt_gate
    mov $5, %edi;  mov $isr_stub_5, %rsi;  call set_idt_gate
    mov $6, %edi;  mov $isr_stub_6, %rsi;  call set_idt_gate
    mov $7, %edi;  mov $isr_stub_7, %rsi;  call set_idt_gate
    mov $8, %edi;  mov $isr_stub_8, %rsi;  call set_idt_gate
    mov $9, %edi;  mov $isr_stub_9, %rsi;  call set_idt_gate
    mov $10, %edi; mov $isr_stub_10, %rsi; call set_idt_gate
    mov $11, %edi; mov $isr_stub_11, %rsi; call set_idt_gate
    mov $12, %edi; mov $isr_stub_12, %rsi; call set_idt_gate
    mov $13, %edi; mov $isr_stub_13, %rsi; call set_idt_gate
    mov $14, %edi; mov $isr_stub_14, %rsi; call set_idt_gate
    mov $15, %edi; mov $isr_stub_15, %rsi; call set_idt_gate
    mov $16, %edi; mov $isr_stub_16, %rsi; call set_idt_gate
    mov $17, %edi; mov $isr_stub_17, %rsi; call set_idt_gate
    mov $18, %edi; mov $isr_stub_18, %rsi; call set_idt_gate
    mov $19, %edi; mov $isr_stub_19, %rsi; call set_idt_gate
    mov $20, %edi; mov $isr_stub_20, %rsi; call set_idt_gate
    mov $21, %edi; mov $isr_stub_21, %rsi; call set_idt_gate
    mov $22, %edi; mov $isr_stub_22, %rsi; call set_idt_gate
    mov $23, %edi; mov $isr_stub_23, %rsi; call set_idt_gate
    mov $24, %edi; mov $isr_stub_24, %rsi; call set_idt_gate
    mov $25, %edi; mov $isr_stub_25, %rsi; call set_idt_gate
    mov $26, %edi; mov $isr_stub_26, %rsi; call set_idt_gate
    mov $27, %edi; mov $isr_stub_27, %rsi; call set_idt_gate
    mov $28, %edi; mov $isr_stub_28, %rsi; call set_idt_gate
    mov $29, %edi; mov $isr_stub_29, %rsi; call set_idt_gate
    mov $30, %edi; mov $isr_stub_30, %rsi; call set_idt_gate
    mov $31, %edi; mov $isr_stub_31, %rsi; call set_idt_gate

    # Critical early exceptions: print-and-halt handlers (avoid triple fault)
    mov $6, %edi;  mov $early_exc_ud, %rsi; call set_idt_gate
    mov $8, %edi;  mov $early_exc_df, %rsi; call set_idt_gate
    mov $13, %edi; mov $early_exc_gp, %rsi; call set_idt_gate
    mov $14, %edi; mov $early_exc_pf, %rsi; call set_idt_gate

    lea early_idt_ptr, %rdx
    lidt (%rdx)

    # BOOT4 after IDT load
    serial_putc $'B'
    serial_putc $'O'
    serial_putc $'O'
    serial_putc $'T'
    serial_putc $'4'
    serial_putc $0x0A

    # BOOT5 before calling kmain
    serial_putc $'B'
    serial_putc $'O'
    serial_putc $'O'
    serial_putc $'T'
    serial_putc $'5'
    serial_putc $0x0A

    mov mb2_info_ptr(%rip), %rdi # Multiboot2 info pointer
    andq $-16, %rsp
    call kmain
    cli
.hang64:
    hlt
    jmp .hang64


.type set_idt_gate, @function
set_idt_gate:
    # rdi=vector (0..255), rsi=handler
    lea early_idt, %rbx
    lea (%rbx,%rdi,8), %rax
    lea (%rax,%rdi,8), %rax
    mov %rsi, %rdx
    movw %dx, 0(%rax)            # offset[15:0]
    movw $0x08, 2(%rax)          # selector
    movb $0x00, 4(%rax)          # IST
    movb $0x8E, 5(%rax)          # present, DPL=0, interrupt gate
    shr $16, %rdx
    movw %dx, 6(%rax)            # offset[31:16]
    shr $16, %rdx
    movl %edx, 8(%rax)           # offset[63:32]
    movl $0, 12(%rax)
    ret

.align 16
early_idt:
    .fill 512,8,0
early_idt_ptr:
    .word (256*16)-1
    .quad early_idt

.type serial_hex64, @function
serial_hex64:
    # value in rbx
    mov $16, %rcx
.hex_loop:
    mov %rbx, %rax
    shr $60, %rax
    and $0x0F, %al
    cmp $10, %al
    jl .hex_digit
    add $('A' - 10), %al
    jmp .hex_emit
.hex_digit:
    add $'0', %al
.hex_emit:
    serial_putc %al
    shl $4, %rbx
    dec %rcx
    jnz .hex_loop
    ret

.type early_exc_ud, @function
early_exc_ud:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $' '
    serial_putc $'6'
    serial_putc $' '
    serial_putc $'R'
    serial_putc $'I'
    serial_putc $'P'
    serial_putc $'='
    mov (%rsp), %rbx
    call serial_hex64
    serial_putc $0x0A
    cli
.halt_ud:
    hlt
    jmp .halt_ud

.type early_exc_df, @function
early_exc_df:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $' '
    serial_putc $'8'
    serial_putc $' '
    serial_putc $'R'
    serial_putc $'I'
    serial_putc $'P'
    serial_putc $'='
    mov 8(%rsp), %rbx
    call serial_hex64
    serial_putc $0x0A
    cli
.halt_df:
    hlt
    jmp .halt_df

.type early_exc_gp, @function
early_exc_gp:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $' '
    serial_putc $'1'
    serial_putc $'3'
    serial_putc $' '
    serial_putc $'R'
    serial_putc $'I'
    serial_putc $'P'
    serial_putc $'='
    mov 8(%rsp), %rbx
    call serial_hex64
    serial_putc $0x0A
    cli
.halt_gp:
    hlt
    jmp .halt_gp

.type early_exc_pf, @function
early_exc_pf:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $' '
    serial_putc $'1'
    serial_putc $'4'
    serial_putc $' '
    serial_putc $'R'
    serial_putc $'I'
    serial_putc $'P'
    serial_putc $'='
    mov 8(%rsp), %rbx
    call serial_hex64
    serial_putc $0x0A
    cli
.halt_pf:
    hlt
    jmp .halt_pf
