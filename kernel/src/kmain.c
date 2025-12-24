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
#include "../include/io.h"
#include "../include/log.h"

extern uint64_t __kernel_phys_base, __kernel_phys_end;

static inline void io_wait(void) {
    outb(0x80, 0);
}

static void busy_delay_approx_100ms(void) {
    for (uint32_t i = 0; i < 100000; ++i) {
        io_wait();
        __asm__ volatile ("pause");
    }
}

__attribute__((noreturn)) void kmain(uint64_t mb_info) {
    __asm__ volatile ("cli");

    log_init();
    logs("stage0: entered kmain\n");

    logs("[A] before gdt_init\n");
    gdt_init();
    logs("[B] after gdt_init\n");

    logs("[C] before idt_init\n");
    idt_init();
    logs("[D] after idt_init\n");

    extern void syscall_init(void);
    logs("[E] before syscall_init\n");
    syscall_init();
    logs("[F] after syscall_init\n");
    logs("stage1: early init ok\n");
    logs("SuayOS serial+debugcon online\n");

    logs("[G] before mb2_parse\n");
    mb2_parse(mb_info);
    logs("[H] after mb2_parse\n");

    logs("[I] before pmm_init\n");
    pmm_init(mb_info);
    logs("[J] after pmm_init\n");
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
        logs(msg);
    }

    logs("[K] before vmm_init\n");
    vmm_init((uint64_t)&__kernel_phys_base, (uint64_t)&__kernel_phys_end);
    logs("[L] after vmm_init\n");

    logs("[M] before kmem_init\n");
    kmem_init();
    logs("[N] after kmem_init\n");
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
        logs(msg);
    }
    for (int i = 0; i < 8; ++i) {
        kfree(blocks[i]);
        logs("kfree\n");
    }
    vga_clear();
    vga_print("SuayOS booted\n");
    logs("SuayOS booted\n");

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
    logs("IRQs disabled\n");
#endif

    logs("HB");
    for (;;) {
        logc('.');
        busy_delay_approx_100ms();
    }
}
