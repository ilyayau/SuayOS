
#include "../include/vga.h"
#include "../include/serial.h"
#include "../include/gdt.h"
#include "../include/idt.h"

void kmain(void) {
    gdt_init();
    idt_init();
    serial_init();
    serial_write("SuayOS serial online\n");
    vga_clear();
    vga_print("SuayOS booted");
    for (;;) {}
}
