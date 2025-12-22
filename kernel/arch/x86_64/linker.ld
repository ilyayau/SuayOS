ENTRY(_start)
SECTIONS
{
    . = 0x100000;
    __kernel_phys_base = .;
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
