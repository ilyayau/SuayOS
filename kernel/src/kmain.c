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

extern uint64_t __kernel_phys_base, __kernel_phys_end;
void kmain(uint64_t mb_info) {
    gdt_init();
    idt_init();
    extern void syscall_init(void);
    syscall_init();
    serial_init();
    serial_write("SuayOS serial online\n");
    mb2_parse(mb_info);
    pmm_init(mb_info);
    // Self-test: allocate 3 pages and log addresses
    for (int i = 0; i < 3; ++i) {
        void *p = pmm_alloc_pages(1);
        char msg[64] = "pmm ";
        uint64_t addr = (uint64_t)p;
        int j = 4;
        if (!addr) { msg[j++] = '0'; }
        else {
            char tmp[20]; int k = 0;
            while (addr) { tmp[k++] = "0123456789ABCDEF"[addr % 16]; addr /= 16; }
            msg[j++] = '0'; msg[j++] = 'x';
            while (k--) msg[j++] = tmp[k];
        }
        msg[j++] = '\n'; msg[j] = 0;
        serial_write(msg);
    }
    vmm_init((uint64_t)&__kernel_phys_base, (uint64_t)&__kernel_phys_end);
    kmem_init();
    // kmalloc/kfree stress test
    void *blocks[8];
    for (int i = 0; i < 8; ++i) {
        blocks[i] = kmalloc(64 + i * 32, 16);
        char msg[64] = "kmalloc ";
        uint64_t addr = (uint64_t)blocks[i];
        int j = 8;
        if (!addr) { msg[j++] = '0'; }
        else {
            char tmp[20]; int k = 0;
            while (addr) { tmp[k++] = "0123456789ABCDEF"[addr % 16]; addr /= 16; }
            msg[j++] = '0'; msg[j++] = 'x';
            while (k--) msg[j++] = tmp[k];
        }
        msg[j++] = '\n'; msg[j] = 0;
        serial_write(msg);
    }
    for (int i = 0; i < 8; ++i) {
        kfree(blocks[i]);
        serial_write("kfree\n");
    }
    pic_remap();
    pic_clear_mask(0); // Unmask IRQ0 (timer)
    pic_clear_mask(1); // Unmask IRQ1 (keyboard)
    pit_init(100); // 100Hz
    vga_clear();
    vga_print("SuayOS booted\n");
    kbd_init();
    __asm__ volatile ("sti");
    // Test user mode: run user program
    extern void user_run(void);
    user_run();

    // Start interactive shell
    extern void shell_run(void);
    shell_run();
}
