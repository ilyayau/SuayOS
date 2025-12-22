

#include "../include/vga.h"
#include "../include/serial.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/pit.h"

void kmain(void) {
    gdt_init();
    idt_init();
    serial_init();
    serial_write("SuayOS serial online\n");
    pic_remap();
    pic_clear_mask(0); // Unmask IRQ0
    pit_init(100); // 100Hz
    vga_clear();
    vga_print("SuayOS booted");
    __asm__ volatile ("sti");
    for (;;) {}
}
