#include "../include/vga.h"
#include "../include/serial.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/pit.h"
#include "../include/mb2.h"
#include "../include/pmm.h"
#include "../include/vmm.h"
#include "../include/kmem.h"
#include "../include/kbd.h"
#include "../include/shell.h"

extern uint64_t __kernel_phys_base, __kernel_phys_end;

__attribute__((noreturn)) void kmain(uint64_t mb_info) {
    __asm__ volatile ("cli");

    serial_init();
    serial_write("stage0: kmain\n");

    serial_write("stage1: gdt init\n");
    gdt_init();
    serial_write("stage2: gdt ok\n");

    serial_write("stage3: idt init\n");
    idt_init();
    serial_write("stage4: idt ok\n");
    extern void syscall_init(void);
    syscall_init();
    serial_write("SuayOS serial online\n");
    mb2_parse(mb_info);
    pmm_init(mb_info);
    vmm_init((uint64_t)&__kernel_phys_base, (uint64_t)&__kernel_phys_end);
    kmem_init();

    serial_write("SuayOS\n");
    serial_write("boot ok\n");

#if ENABLE_IRQ
    pic_remap();
    pic_clear_mask(0); // Unmask IRQ0 (timer)
    pic_clear_mask(1); // Unmask IRQ1 (keyboard)
    pit_init(100); // 100Hz
    kbd_init();
    __asm__ volatile ("sti");
    extern void user_run(void);
    user_run();
    extern void shell_run(void);
    shell_run();
#else
    serial_write("IRQs disabled\n");
#endif

    shell_run();

    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}
