ENTRY(_start)
SECTIONS
{
    . = 0x100000;
    __kernel_phys_base = .;
    /* Place Multiboot2 header first and keep it */
    .multiboot : ALIGN(8) {
        KEEP(*(.multiboot))
    }
    .text : {
        *(.text*)
    }
    .rodata : {
        *(.rodata*)
    }
    .data : {
        *(.data*)
    }
    .bss : {
        *(.bss*)
        *(COMMON)
    }
    __kernel_phys_end = .;
}
