
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
    mov $stack64_top, %rsp
    pop %rdi                # Multiboot2 info pointer
    call kmain
    cli
.hang64:
    hlt
    jmp .hang64
