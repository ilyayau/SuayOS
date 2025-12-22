.section .multiboot2_header, "a"
.align 8
.long 0xE85250D6              # magic
.long 0                     # architecture (0 = i386+, works for x86_64)
.long multiboot2_header_end - multiboot2_header # header length
.long -(0xE85250D6 + 0 + (multiboot2_header_end - multiboot2_header)) # checksum

# Minimal end tag
multiboot2_header:
    .long 0xE85250D6
    .long 0
    .long multiboot2_header_end - multiboot2_header
    .long -(0xE85250D6 + 0 + (multiboot2_header_end - multiboot2_header))
    # End tag
    .short 0
    .short 0
    .long 8
multiboot2_header_end:

.section .text
.global _start
.type _start, @function



_start:
    # Set up stack
    lea stack_top(%rip), %rsp
    mov %rbx, %rdi   # Multiboot2 info pointer (in rbx per spec)
    call kmain
    cli
.hang:
    hlt
    jmp .hang

# 16 KiB stack
.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top:
