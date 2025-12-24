
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
    mov %ebx, %eax        # Multiboot2 info pointer
    push %eax             # Save for 64-bit

    # Set up temporary GDT
    lgdt gdt32_ptr
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    # Enable PAE
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4

    # Set up PML4 (identity map 0-4GB)
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

.align 4096
pdpt:
    .quad pd + 0x03
    .quad 0
    .quad 0
    .quad 0

.align 4096
pd:
    .quad 0x0000000000000083 # 2MB page, present, rw
    .fill 511,8,0

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
        mov $0x3F8, %dx
        movb \char, %al
        out %al, %dx
    .endm

    # Print E64\n

    serial_putc $'E'
    serial_putc $'6'
    serial_putc $'4'
    serial_putc $0x0A

    # Early IDT (valid 64-bit gates)
    xor %edi, %edi
.idt_all:
    mov $exc_unknown, %rsi
    call set_idt_gate
    inc %edi
    cmp $256, %edi
    jne .idt_all

    mov $6, %edi
    mov $exc_ud, %rsi
    call set_idt_gate

    mov $8, %edi
    mov $exc_df, %rsi
    call set_idt_gate

    mov $13, %edi
    mov $exc_gp, %rsi
    call set_idt_gate

    mov $14, %edi
    mov $exc_pf, %rsi
    call set_idt_gate

    lea early_idt_ptr, %rdx
    lidt (%rdx)

    # Print IDT\n
    serial_putc $'I'
    serial_putc $'D'
    serial_putc $'T'
    serial_putc $0x0A

    # Print GDT\n
    serial_putc $'G'
    serial_putc $'D'
    serial_putc $'T'
    serial_putc $0x0A

    # Print KM\n before kmain
    serial_putc $'K'
    serial_putc $'M'
    serial_putc $0x0A

    pop %rdi                # Multiboot2 info pointer
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

.type exc_halt, @function
exc_halt:
    cli
.halt_exc:
    hlt
    jmp .halt_exc

.type exc_unknown, @function
exc_unknown:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $0x20
    serial_putc $'?'
    serial_putc $'?'
    serial_putc $0x0A
    jmp exc_halt

.type exc_ud, @function
exc_ud:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $0x20
    serial_putc $'6'
    serial_putc $0x0A
    jmp exc_halt

.type exc_df, @function
exc_df:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $0x20
    serial_putc $'8'
    serial_putc $0x0A
    jmp exc_halt

.type exc_gp, @function
exc_gp:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $0x20
    serial_putc $'1'
    serial_putc $'3'
    serial_putc $0x0A
    jmp exc_halt

.type exc_pf, @function
exc_pf:
    serial_putc $'E'
    serial_putc $'X'
    serial_putc $'C'
    serial_putc $0x20
    serial_putc $'1'
    serial_putc $'4'
    serial_putc $0x0A
    jmp exc_halt
