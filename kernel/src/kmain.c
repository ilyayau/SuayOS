
#include "../include/vga.h"
#include "../include/serial.h"

void kmain(void) {
    serial_init();
    serial_write("SuayOS serial online\n");
    vga_clear();
    vga_print("SuayOS booted");
    for (;;) {}
}
